
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

#include <Optimizer/Property/Property.h>
#include <Optimizer/SymbolEquivalencesDeriver.h>

namespace DB
{
class PropertyMatcher
{
public:
    static bool matchNodePartitioning(
        const Context & context, Partitioning & required, const Partitioning & actual, const SymbolEquivalences & equivalences = {});

    static bool matchStreamPartitioning(
        const Context & context,
        const Partitioning & required,
        const Partitioning & actual,
        const SymbolEquivalences & equivalences = {});
    static Property compatibleCommonRequiredProperty(const std::unordered_set<Property, PropertyHash> & properties);
};
}
