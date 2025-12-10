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

#include "cbridge/include/BuiltinHandle.h"
#include "cbridge_internal/include/BaseHandleImpl.h"

#include <memory>
#include <new>
#include <optional>

#define DEFINE_HANDLE_METHODS( T )                                                                                      \
    _baseRef T##_create_handle( T t )                                                                                   \
    {                                                                                                                   \
        return reinterpret_cast< _baseRef >( new ( ::std::nothrow ) std::optional< T >( t ) );    \
    }                                                                                                                   \
    void T##_release_handle( _baseRef handle )                                                                          \
    {                                                                                                                   \
        delete reinterpret_cast< std::optional< T >* >( handle );                               \
    }                                                                                                                   \
    T T##_value_get( _baseRef handle )                                                                                  \
    {                                                                                                                   \
        return **reinterpret_cast< std::optional< T >* >( handle );                             \
    }

DEFINE_HANDLE_METHODS( bool );
DEFINE_HANDLE_METHODS( float );
DEFINE_HANDLE_METHODS( double );
DEFINE_HANDLE_METHODS( int8_t );
DEFINE_HANDLE_METHODS( uint8_t );
DEFINE_HANDLE_METHODS( int16_t );
DEFINE_HANDLE_METHODS( uint16_t );
DEFINE_HANDLE_METHODS( int32_t );
DEFINE_HANDLE_METHODS( uint32_t );
DEFINE_HANDLE_METHODS( int64_t );
DEFINE_HANDLE_METHODS( uint64_t );
