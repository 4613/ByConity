
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

#include <common/shared_ptr_helper.h>
#include <Storages/System/IStorageSystemOneBlock.h>

namespace DB
{
class StorageSystemWorkerGroups : public shared_ptr_helper<StorageSystemWorkerGroups>, public IStorageSystemOneBlock<StorageSystemWorkerGroups>
{
public:
    std::string getName() const override
    {
        return "SystemWorkerGroups";
    }

    static NamesAndTypesList getNamesAndTypes();

protected:
    using IStorageSystemOneBlock::IStorageSystemOneBlock;

    void fillData(MutableColumns & res_columns, const ContextPtr context, const SelectQueryInfo &) const override;
};

}
