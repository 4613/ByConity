
/*
 * Copyright (2022) ByteDance Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cstddef>
#include <Columns/ColumnsNumber.h>
#include <Columns/IColumn.h>
#include <Core/Block.h>
#include <Processors/Chunk.h>
#include <common/types.h>
#include <Core/ColumnsWithTypeAndName.h>
#include <Functions/IFunction.h>
#include <Interpreters/Context_fwd.h>

namespace UnitTest
{
extern DB::Chunk createUInt8Chunk(size_t row_num, size_t column_num, UInt8 value);

extern DB::Block createUInt64Block(size_t row_num, size_t column_num, UInt8 value);

extern DB::ExecutableFunctionPtr createRepartitionFunction(DB::ContextPtr context, const DB::ColumnsWithTypeAndName & arguments);

}
