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

#include "BaseHandle.h"
#include "Export.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

_GLUECODIUM_C_EXPORT _baseRef std_string_create_handle(const char* c_str);
_GLUECODIUM_C_EXPORT const char* std_string_data_get(_baseRef handle);
_GLUECODIUM_C_EXPORT int64_t std_string_size_get(_baseRef handle);
_GLUECODIUM_C_EXPORT void std_string_release_handle(_baseRef handle);

_GLUECODIUM_C_EXPORT _baseRef std_string_create_optional_handle(const char* c_str);
_GLUECODIUM_C_EXPORT void std_string_release_optional_handle(_baseRef handle);
_GLUECODIUM_C_EXPORT _baseRef std_string_unwrap_optional_handle(_baseRef handle);

#ifdef __cplusplus
}
#endif
