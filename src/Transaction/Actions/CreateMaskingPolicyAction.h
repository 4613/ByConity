
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

// #pragma once

// #include <Transaction/Actions/Action.h>
// #include <Access/MaskingPolicyDataModel.h>

// namespace DB
// {
// class CreateMaskingPolicyAction : public Action
// {
// public:
//     CreateMaskingPolicyAction(const Context & context_, const TxnTimestamp & txn_id_, MaskingPolicyModel & policy_)
//         : Action(context_, txn_id_), policy(policy_)
//     {
//     }

//     ~CreateMaskingPolicyAction() = default;

//     void executeV1(TxnTimestamp commit_time) override;
//     void abort() override;
// private:
//     MaskingPolicyModel & policy;
// };

// }
