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

#include "cbridge/include/StringHandle.h"
#include "cbridge_internal/include/BaseHandleImpl.h"
#include <new>
#include <optional>
#include <string>
#include <memory>

namespace {
template<class T>
_baseRef create_handle( T t ) {
    return reinterpret_cast< _baseRef >(
        new ( ::std::nothrow ) ::std::shared_ptr< T >( new ( ::std::nothrow ) T( t ) ) );
}
template<class T>
void release_handle( _baseRef handle )
{
    delete reinterpret_cast< ::std::shared_ptr< T >*>( handle );
}
template<class T>
T value_get( _baseRef handle )
{
    return **reinterpret_cast< ::std::shared_ptr< T >*>( handle );
}
}

_baseRef
std_string_create_handle( const char* c_str )
{
    return reinterpret_cast< _baseRef >( new ( ::std::nothrow ) ::std::string( c_str ) );
}

void
std_string_release_handle( _baseRef handle )
{
    delete get_pointer< ::std::string >( handle );
}

const char*
std_string_data_get( _baseRef handle )
{
    return get_pointer< ::std::string >( handle )->data( );
}

int64_t
std_string_size_get( _baseRef handle )
{
    return get_pointer< ::std::string >( handle )->size( );
}

_baseRef
std_string_create_optional_handle( const char* c_str )
{
    return reinterpret_cast< _baseRef >(
        new ( ::std::nothrow ) std::optional<std::string>( ::std::string( c_str ) ) );
}

void
std_string_release_optional_handle( _baseRef handle )
{
    delete reinterpret_cast<std::optional<std::string>*>(handle);
}

_baseRef
std_string_unwrap_optional_handle( _baseRef handle )
{
    return reinterpret_cast< _baseRef >( &**reinterpret_cast<std::optional<std::string>*>( handle ) );
}
