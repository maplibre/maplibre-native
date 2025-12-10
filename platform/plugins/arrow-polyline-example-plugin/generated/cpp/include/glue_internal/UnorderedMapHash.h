// -------------------------------------------------------------------------------------------------
// Copyright (C) 2016-2019 HERE Europe B.V.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0
// License-Filename: LICENSE
//
// -------------------------------------------------------------------------------------------------

#pragma once
#include "Hash.h"
#include <unordered_map>

namespace glue_internal {

template <class Key, class T, class Hash>
struct hash<std::unordered_map<Key, T, Hash>> {
    size_t operator()(const std::unordered_map<Key, T, Hash>& vec) const {
        size_t hash_value = 29;
        auto key_hasher = Hash();
        auto value_hasher = hash<T>();
        for (const auto& entry : vec) {
            hash_value ^= (809 ^ key_hasher(entry.first));
            hash_value ^= (811 ^ value_hasher(entry.second));
        }
        return hash_value;
    }
};
} // namespace glue_internal
