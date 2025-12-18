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

#include "cbridge_internal/include/WrapperCache.h"

namespace glue_internal {

std::atomic<bool> WrapperCache::is_alive{};

WrapperCache::WrapperCache() noexcept {
    is_alive = true;
}
WrapperCache::~WrapperCache() {
    is_alive = false;
}

void WrapperCache::cache_wrapper(WrapperCache::CppPtr cpp_ptr, WrapperCache::SwiftPtr swift_ptr) {
    ::std::lock_guard<std::mutex> lock(mutex);
    cache[cpp_ptr] = swift_ptr;
}

WrapperCache::SwiftPtr WrapperCache::get_cached_wrapper(WrapperCache::CppPtr cpp_ptr) {
    ::std::lock_guard<std::mutex> lock(mutex);
    auto iter = cache.find(cpp_ptr);
    return (iter != cache.end()) ? iter->second : nullptr;
}

void WrapperCache::remove_cached_wrapper(WrapperCache::CppPtr cpp_ptr) {
    ::std::lock_guard<std::mutex> lock(mutex);
    cache.erase(cpp_ptr);
}

// Handle cases of non-deterministic ordering of construction and destruction of global
// variables. This is on two ends:
// 1. Use function static variable to ensure it's constructed on first use.
// 2. Use an "alive" flag to ensure it won't crash if destructed early. (e.g. storing a
//    shared_ptr to a proxy object as a static variable)
WrapperCache& get_wrapper_cache() {
    static WrapperCache cache;
    return cache;
}

} // namespace glue_internal
