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

#include "cbridge/include/ByteArrayHandle.h"
#include "cbridge_internal/include/BaseHandleImpl.h"

#include <memory>
#include <new>
#include <optional>
#include <vector>

_baseRef
byteArray_create_handle( )
{
    return reinterpret_cast< _baseRef >( new ( ::std::nothrow )
                                             ::std::shared_ptr< ::std::vector< uint8_t > >(
                                                 new ( ::std::nothrow ) ::std::vector< uint8_t >{} ) );
}

void
byteArray_release_handle( _baseRef handle )
{
    delete get_pointer< ::std::shared_ptr< ::std::vector< uint8_t > > >( handle );
}

void
byteArray_assign( _baseRef handle, const uint8_t* data, const size_t size )
{
    ( *get_pointer< ::std::shared_ptr< ::std::vector< uint8_t > > >( handle ) )
        ->assign( data, data + size );
}

const uint8_t*
byteArray_data_get( _baseRef handle )
{
    return ( *get_pointer< ::std::shared_ptr< ::std::vector< uint8_t > > >( handle ) )->data( );
}

size_t
byteArray_size_get( _baseRef handle )
{
    return ( *get_pointer< ::std::shared_ptr< ::std::vector< uint8_t > > >( handle ) )->size( );
}

_baseRef
byteArray_create_optional_handle( )
{
    return reinterpret_cast< _baseRef >(
        new ( ::std::nothrow ) std::optional< ::std::shared_ptr< ::std::vector< uint8_t > > >(
            ::std::shared_ptr< ::std::vector< uint8_t > >( new ( ::std::nothrow ) ::std::vector< uint8_t >{} ) ) );
}

void
byteArray_release_optional_handle( _baseRef handle )
{
    delete reinterpret_cast< std::optional <::std::shared_ptr< ::std::vector< uint8_t > > >* >(handle);
}

_baseRef
byteArray_unwrap_optional_handle( _baseRef handle )
{
    return reinterpret_cast< _baseRef >(
        &**reinterpret_cast< std::optional< ::std::shared_ptr< ::std::vector< uint8_t > > >* >( handle ) );
}

void
byteArray_assign_optional( _baseRef handle, const uint8_t* data, const size_t size )
{
    ( **get_pointer< std::optional< ::std::shared_ptr< ::std::vector< uint8_t > > > >( handle ) )
        ->assign( data, data + size );
}
