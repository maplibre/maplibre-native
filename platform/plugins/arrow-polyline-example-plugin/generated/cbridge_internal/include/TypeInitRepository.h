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

#include "cbridge/include/BaseHandle.h"
#include "cbridge/include/Export.h"
#include <string_view>

using TypeId = std::string_view;

/**
 * Stores links to Swift init functions.
 */
class _GLUECODIUM_C_EXPORT TypeInitRepository {
public:
    TypeInitRepository();
    ~TypeInitRepository();

    using InitFunction = void*(_baseRef);
    void add_init(const TypeId& id, InitFunction* init);
    InitFunction* get_init(const TypeId& id) const;

private:
    struct Impl;
    Impl* pimpl;
};

_GLUECODIUM_C_EXPORT TypeInitRepository& get_init_repository();
