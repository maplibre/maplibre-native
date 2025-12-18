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
#include <unordered_set>

namespace glue_internal {

template <class T, class H>
struct hash<std::unordered_set<T, H>> {
    size_t operator()(const std::unordered_set<T, H>& set) const {
        size_t hash_value = 67;
        for (const auto& value : set) {
            hash_value = hash_value ^ ::glue_internal::hash<T>()(value);
        }
        return hash_value;
    }
};
} // namespace glue_internal
