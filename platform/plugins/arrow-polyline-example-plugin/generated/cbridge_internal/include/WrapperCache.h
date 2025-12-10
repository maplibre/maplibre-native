// -------------------------------------------------------------------------------------------------
// Copyright (C) 2016-2020 HERE Europe B.V.
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

#include "cbridge/include/Export.h"
#include <atomic>
#include <mutex>
#include <unordered_map>

namespace glue_internal {
class _GLUECODIUM_C_EXPORT WrapperCache {
public:
    static ::std::atomic<bool> is_alive;

    using CppPtr = const void*;
    using SwiftPtr = const void*;

    explicit WrapperCache() noexcept;
    ~WrapperCache();

    void cache_wrapper(CppPtr cpp_ptr, SwiftPtr swift_ptr);
    SwiftPtr get_cached_wrapper(CppPtr cpp_ptr);
    void remove_cached_wrapper(CppPtr cpp_ptr);

private:
    ::std::mutex mutex;
    ::std::unordered_map<CppPtr, SwiftPtr> cache;
};

_GLUECODIUM_C_EXPORT WrapperCache& get_wrapper_cache();
} // namespace glue_internal
