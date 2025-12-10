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
#include "ExportCommonGluecodiumCpp.h"
#include <string_view>

namespace glue_internal {

using TypeId = std::string_view;

class _GLUECODIUM_CPP_EXPORT TypeRepository final {
public:
    TypeRepository();
    ~TypeRepository();

    void add_type(const void* instance, const TypeId& id);
    TypeId get_id(const void* instance) const;
    void remove_type(const void* instance);

private:
    struct Impl;
    Impl* pimpl;
};

_GLUECODIUM_CPP_EXPORT TypeRepository& get_type_repository();

} // namespace glue_internal
