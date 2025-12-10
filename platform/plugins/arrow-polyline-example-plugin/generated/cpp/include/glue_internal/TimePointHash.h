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
#include <chrono>

namespace glue_internal {

template<class Clock, class Duration>
struct hash<std::chrono::time_point<Clock, Duration>> {
    size_t operator()(const std::chrono::time_point<Clock, Duration>& t) const noexcept {
        return hash<typename Clock::rep>{}(t.time_since_epoch().count());
    }
};
}
