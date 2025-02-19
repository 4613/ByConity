/*
 * Copyright 2016-2023 ClickHouse, Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/*
 * This file may have been modified by Bytedance Ltd. and/or its affiliates (“ Bytedance's Modifications”).
 * All Bytedance's Modifications are Copyright (2023) Bytedance Ltd. and/or its affiliates.
 */

#include "ParquetBlockInputFormat.h"
#include <boost/algorithm/string/case_conv.hpp>
#include "Common/setThreadName.h"
#include "Formats/FormatSettings.h"
#if USE_PARQUET

#    include <Formats/FormatFactory.h>
#    include <IO/ReadBufferFromMemory.h>
#    include <IO/copyData.h>
#    include <arrow/api.h>
#    include <arrow/io/api.h>
#    include <arrow/status.h>
#    include <parquet/arrow/reader.h>
#    include <parquet/arrow/schema.h>
#    include <parquet/file_reader.h>
#    include <common/logger_useful.h>
#    include "ArrowBufferedStreams.h"
#    include "ArrowColumnToCHColumn.h"
#    include "ArrowFieldIndexUtil.h"


namespace CurrentMetrics
{
    extern const Metric ParquetDecoderThreads;
    extern const Metric ParquetDecoderThreadsActive;
}


namespace DB
{

namespace ErrorCodes
{
    extern const int BAD_ARGUMENTS;
    extern const int CANNOT_READ_ALL_DATA;
}

#define THROW_ARROW_NOT_OK(status)                                     \
    do                                                                 \
    {                                                                  \
        if (::arrow::Status _s = (status); !_s.ok())                   \
            throw Exception(_s.ToString(), ErrorCodes::BAD_ARGUMENTS); \
    } while (false)

ParquetBlockInputFormat::ParquetBlockInputFormat(
    ReadBuffer & buf,
    const Block & header_,
    const FormatSettings & format_settings_,
    size_t max_decoding_threads_,
    size_t min_bytes_for_seek_)
    : IInputFormat(header_, buf)
    , format_settings(format_settings_)
    , skip_row_groups(format_settings.parquet.skip_row_groups)
    , max_decoding_threads(max_decoding_threads_)
    , min_bytes_for_seek(min_bytes_for_seek_)
    , pending_chunks(PendingChunk::Compare { .row_group_first = format_settings_.parquet.preserve_order })
{
    if (max_decoding_threads > 1 && format_settings.parquet.file_size > 0)
        pool = std::make_unique<ThreadPool>(max_decoding_threads);
}

ParquetBlockInputFormat::~ParquetBlockInputFormat()
{
    is_stopped = true;
    if (pool)
        pool->wait();
}

void ParquetBlockInputFormat::initializeIfNeeded()
{
    if (std::exchange(is_initialized, true))
        return;

    // Create arrow file adapter.
    // TODO: Make the adapter do prefetching on IO threads, based on the full set of ranges that
    //       we'll need to read (which we know in advance). Use max_download_threads for that.
    auto * seekable_buffer_ptr = dynamic_cast<SeekableReadBuffer *>(&in);
    if (seekable_buffer_ptr != nullptr && format_settings.parquet.file_size != 0)
    {
        arrow_file = asArrowFile(*seekable_buffer_ptr, format_settings.parquet.file_size);
    }
    else
    {
        arrow_file = asArrowFile(in);
    }

    if (is_stopped)
        return;

    metadata = parquet::ReadMetaData(arrow_file);

    std::shared_ptr<arrow::Schema> schema;
    THROW_ARROW_NOT_OK(parquet::arrow::FromParquetSchema(metadata->schema(), &schema));

    row_groups.resize(metadata->num_row_groups());

    ArrowFieldIndexUtil field_util(
        format_settings.parquet.case_insensitive_column_matching,
        format_settings.parquet.allow_missing_columns);
    column_indices = field_util.findRequiredIndices(getPort().getHeader(), *schema);
}

std::vector<int> ParquetBlockInputFormat::getColumnIndices(
    const std::shared_ptr<arrow::Schema> & schema, const Block & header, const FormatSettings & format_settings)
{
    ArrowFieldIndexUtil field_util(format_settings.parquet.case_insensitive_column_matching, format_settings.parquet.allow_missing_columns);
    return field_util.findRequiredIndices(header, *schema);
}

void ParquetBlockInputFormat::initializeRowGroupReader(size_t row_group_idx)
{
    auto & row_group = row_groups[row_group_idx];

    parquet::ArrowReaderProperties properties;
    properties.set_use_threads(false);
    if (format_settings.parquet.max_block_size > 0)
        properties.set_batch_size(format_settings.parquet.max_block_size);
    else 
    {
        const auto batch_size = metadata->RowGroup(row_group_idx)->num_rows();
        properties.set_batch_size(batch_size);
    }

    // When reading a row group, arrow will:
    //  1. Look at `metadata` to get all byte ranges it'll need to read from the file (typically one
    //     per requested column in the row group).
    //  2. Coalesce ranges that are close together, trading off seeks vs read amplification.
    //     This is controlled by CacheOptions.
    //  3. Process the columns one by one, issuing the corresponding (coalesced) range reads as
    //     needed. Each range gets its own memory buffer allocated. These buffers stay in memory
    //     (in arrow::io::internal::ReadRangeCache) until the whole row group reading is done.
    //     So the memory usage of a "SELECT *" will be at least the compressed size of a row group
    //     (typically hundreds of MB).
    //
    // With this coalescing, we don't need any readahead on our side, hence avoid_buffering in
    // asArrowFile().
    //
    // This adds one unnecessary copy. We should probably do coalescing and prefetch scheduling on
    // our side instead.
    properties.set_pre_buffer(true);
    auto cache_options = arrow::io::CacheOptions::LazyDefaults();
    cache_options.hole_size_limit = min_bytes_for_seek;
    cache_options.range_size_limit = 1l << 40; // reading the whole row group at once is fine
    properties.set_cache_options(cache_options);

    // Workaround for a workaround in the parquet library.
    //
    // From ComputeColumnChunkRange() in contrib/arrow/cpp/src/parquet/file_reader.cc:
    //  > The Parquet MR writer had a bug in 1.2.8 and below where it didn't include the
    //  > dictionary page header size in total_compressed_size and total_uncompressed_size
    //  > (see IMPALA-694). We add padding to compensate.
    //
    // That padding breaks the pre-buffered mode because the padded read ranges may overlap each
    // other, failing an assert. So we disable pre-buffering in this case.
    // That version is >10 years old, so this is not very important.
    if (metadata->writer_version().VersionLt(parquet::ApplicationVersion::PARQUET_816_FIXED_VERSION()))
        properties.set_pre_buffer(false);

    parquet::arrow::FileReaderBuilder builder;
    THROW_ARROW_NOT_OK(
        builder.Open(arrow_file, /* not to be confused with ArrowReaderProperties */ parquet::default_reader_properties(), metadata));
    builder.properties(properties);
    // TODO: Pass custom memory_pool() to enable memory accounting with non-jemalloc allocators.
    THROW_ARROW_NOT_OK(builder.Build(&row_group.file_reader));

    THROW_ARROW_NOT_OK(
        row_group.file_reader->GetRecordBatchReader({static_cast<int>(row_group_idx)}, column_indices, &row_group.record_batch_reader));
    std::shared_ptr<arrow::Schema> schema;
    THROW_ARROW_NOT_OK(row_group.file_reader->GetSchema(&schema));

    row_group.arrow_column_to_ch_column = std::make_unique<ArrowColumnToCHColumn>(
        getPort().getHeader(), schema, "Parquet", format_settings.parquet.allow_missing_columns, format_settings.null_as_default);
    row_group.row_group_bytes_uncompressed = metadata->RowGroup(static_cast<int>(row_group_idx))->total_compressed_size();
    row_group.row_group_rows = metadata->RowGroup(static_cast<int>(row_group_idx))->num_rows();
}

void ParquetBlockInputFormat::scheduleRowGroup(size_t row_group_idx)
{
    chassert(!mutex.try_lock());

    auto & status = row_groups[row_group_idx].status;
    chassert(status == RowGroupState::Status::NotStarted || status == RowGroupState::Status::Paused);

    status = RowGroupState::Status::Running;

    pool->scheduleOrThrowOnError(
        [this, row_group_idx, thread_group = CurrentThread::getGroup()]()
        {
            if (thread_group)
                CurrentThread::attachToIfDetached(thread_group);
            SCOPE_EXIT({if (thread_group) CurrentThread::detachQueryIfNotDetached();});

            try
            {
                setThreadName("ParquetDecoder");

                threadFunction(row_group_idx);
            }
            catch (...)
            {
                std::lock_guard lock(mutex);
                background_exception = std::current_exception();
                condvar.notify_all();
            }
        });
}

void ParquetBlockInputFormat::threadFunction(size_t row_group_idx)
{
    std::unique_lock lock(mutex);

    auto & row_group = row_groups[row_group_idx];
    chassert(row_group.status == RowGroupState::Status::Running);

    while (true)
    {
        if (is_stopped || row_group.num_pending_chunks >= max_pending_chunks_per_row_group)
        {
            row_group.status = RowGroupState::Status::Paused;
            return;
        }

        decodeOneChunk(row_group_idx, lock);

        if (row_group.status == RowGroupState::Status::Done)
            return;
    }
}

void ParquetBlockInputFormat::decodeOneChunk(size_t row_group_idx, std::unique_lock<std::mutex> & lock)
{
    auto & row_group = row_groups[row_group_idx];
    chassert(row_group.status != RowGroupState::Status::Done);
    chassert(lock.owns_lock());
    SCOPE_EXIT({ chassert(lock.owns_lock() || std::uncaught_exceptions()); });

    lock.unlock();

    auto end_of_row_group = [&] {
        row_group.arrow_column_to_ch_column.reset();
        row_group.record_batch_reader.reset();
        row_group.file_reader.reset();

        lock.lock();
        row_group.status = RowGroupState::Status::Done;

        // We may be able to schedule more work now, but can't call scheduleMoreWorkIfNeeded() right
        // here because we're running on the same thread pool, so it'll deadlock if thread limit is
        // reached. Wake up generate() instead.
        condvar.notify_all();
    };

    if (!row_group.record_batch_reader)
    {
        if (skip_row_groups.contains(static_cast<int>(row_group_idx)))
        {
            // Pretend that the row group is empty.
            // (We could avoid scheduling the row group on a thread in the first place. But the
            // skip_row_groups feature is mostly unused, so it's better to be a little inefficient
            // than to add a bunch of extra mostly-dead code for this.)
            end_of_row_group();
            return;
        }

        initializeRowGroupReader(row_group_idx);
    }


    auto batch = row_group.record_batch_reader->Next();
    if (!batch.ok())
        throw ParsingException(ErrorCodes::CANNOT_READ_ALL_DATA, "Error while reading Parquet data: {}", batch.status().ToString());

    if (!*batch)
    {
        end_of_row_group();
        return;
    }

    auto tmp_table = arrow::Table::FromRecordBatches({*batch});

    size_t approx_chunk_original_size = static_cast<size_t>(std::ceil(static_cast<double>(row_group.row_group_bytes_uncompressed) / row_group.row_group_rows * (*tmp_table)->num_rows()));
    PendingChunk res = {.chunk_idx = row_group.next_chunk_idx, .row_group_idx = row_group_idx, .approx_original_chunk_size = approx_chunk_original_size};

    /// If defaults_for_omitted_fields is true, calculate the default values from default expression for omitted fields.
    /// Otherwise fill the missing columns with zero values of its type.
    row_group.arrow_column_to_ch_column->arrowTableToCHChunk(res.chunk, *tmp_table);

    lock.lock();

    ++row_group.next_chunk_idx;
    ++row_group.num_pending_chunks;
    pending_chunks.push(std::move(res));
    condvar.notify_all();
}

void ParquetBlockInputFormat::scheduleMoreWorkIfNeeded(std::optional<size_t> row_group_touched)
{
    while (row_groups_completed < row_groups.size())
    {
        auto & row_group = row_groups[row_groups_completed];
        if (row_group.status != RowGroupState::Status::Done || row_group.num_pending_chunks != 0)
            break;
        ++row_groups_completed;
    }

    if (pool)
    {
        while (row_groups_started - row_groups_completed < max_decoding_threads &&
               row_groups_started < row_groups.size())
            scheduleRowGroup(row_groups_started++);

        if (row_group_touched)
        {
            auto & row_group = row_groups[*row_group_touched];
            if (row_group.status == RowGroupState::Status::Paused &&
                row_group.num_pending_chunks < max_pending_chunks_per_row_group)
                scheduleRowGroup(*row_group_touched);
        }
    }
}

Chunk ParquetBlockInputFormat::generate()
{
    initializeIfNeeded();

    std::unique_lock lock(mutex);

    while (true)
    {
        if (background_exception)
        {
            is_stopped = true;
            std::rethrow_exception(background_exception);
        }
        if (is_stopped)
            return {};

        scheduleMoreWorkIfNeeded();

        if (!pending_chunks.empty() &&
            (!format_settings.parquet.preserve_order ||
             pending_chunks.top().row_group_idx == row_groups_completed))
        {
            PendingChunk chunk = std::move(const_cast<PendingChunk&>(pending_chunks.top()));
            pending_chunks.pop();

            auto & row_group = row_groups[chunk.row_group_idx];
            chassert(row_group.num_pending_chunks != 0);
            chassert(chunk.chunk_idx == row_group.next_chunk_idx - row_group.num_pending_chunks);
            --row_group.num_pending_chunks;

            scheduleMoreWorkIfNeeded(chunk.row_group_idx);

            previous_block_missing_values = std::move(chunk.block_missing_values);
            previous_approx_bytes_read_for_chunk = chunk.approx_original_chunk_size;
            return std::move(chunk.chunk);
        }

        if (row_groups_completed == row_groups.size())
            return {};

        if (pool)
            condvar.wait(lock);
        else
            decodeOneChunk(row_groups_completed, lock);
    }
}

void ParquetBlockInputFormat::resetParser()
{
    is_stopped = true;
    if (pool)
        pool->wait();

    arrow_file.reset();
    metadata.reset();
    column_indices.clear();
    row_groups.clear();
    while (!pending_chunks.empty())
        pending_chunks.pop();
    row_groups_completed = 0;
    previous_block_missing_values.clear();
    row_groups_started = 0;
    background_exception = nullptr;

    is_stopped = false;
    is_initialized = false;

    IInputFormat::resetParser();
}

const BlockMissingValues & ParquetBlockInputFormat::getMissingValues() const
{
    return previous_block_missing_values;
}


// TODO(RENMING):: fix this.
void registerInputFormatProcessorParquet(FormatFactory &factory)
{
    factory.registerInputFormatProcessor(
            "Parquet",
            [](ReadBuffer &buf,
                const Block &sample,
                const RowInputFormatParams &,
                const FormatSettings & settings)
            {
                return std::make_shared<ParquetBlockInputFormat>(
                    buf,
                    sample,
                    settings, 1, 1024 * 1024);
            });
    factory.markFormatAsColumnOriented("Parquet");
}


}

#else

namespace DB
{
class FormatFactory;
void registerInputFormatProcessorParquet(FormatFactory &)
{
}
}

#endif
