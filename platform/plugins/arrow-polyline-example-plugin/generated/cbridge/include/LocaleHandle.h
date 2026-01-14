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

#include "BaseHandle.h"
#include "Export.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

_GLUECODIUM_C_EXPORT _baseRef locale_create_handle(_baseRef language_code_handle,
                                                   _baseRef country_code_handle,
                                                   _baseRef script_code_handle,
                                                   _baseRef language_tag_handle);
_GLUECODIUM_C_EXPORT void locale_release_handle(_baseRef handle);
_GLUECODIUM_C_EXPORT _baseRef locale_get_language_code(_baseRef handle);
_GLUECODIUM_C_EXPORT _baseRef locale_get_country_code(_baseRef handle);
_GLUECODIUM_C_EXPORT _baseRef locale_get_script_code(_baseRef handle);
_GLUECODIUM_C_EXPORT _baseRef locale_get_language_tag(_baseRef handle);

_GLUECODIUM_C_EXPORT _baseRef locale_create_optional_handle(_baseRef locale_handle);
_GLUECODIUM_C_EXPORT void locale_release_optional_handle(_baseRef handle);
_GLUECODIUM_C_EXPORT _baseRef locale_unwrap_optional_handle(_baseRef handle);

#ifdef __cplusplus
}
#endif
