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
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DECLARE_HANDLE_METHODS(T) \
_GLUECODIUM_C_EXPORT _baseRef T##_create_handle( T t ); \
_GLUECODIUM_C_EXPORT void T##_release_handle( _baseRef handle );\
_GLUECODIUM_C_EXPORT T T##_value_get( _baseRef handle );

DECLARE_HANDLE_METHODS(bool);
DECLARE_HANDLE_METHODS(float);
DECLARE_HANDLE_METHODS(double);
DECLARE_HANDLE_METHODS(int8_t);
DECLARE_HANDLE_METHODS(uint8_t);
DECLARE_HANDLE_METHODS(int16_t);
DECLARE_HANDLE_METHODS(uint16_t);
DECLARE_HANDLE_METHODS(int32_t);
DECLARE_HANDLE_METHODS(uint32_t);
DECLARE_HANDLE_METHODS(int64_t);
DECLARE_HANDLE_METHODS(uint64_t);

#undef DECLARE_HANDLE_METHODS

#ifdef __cplusplus
}
#endif
