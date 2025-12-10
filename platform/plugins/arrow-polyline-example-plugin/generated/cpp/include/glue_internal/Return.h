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

#include <initializer_list>
#include <system_error>
#include <type_traits>
#include <utility>

namespace glue_internal {

/**
 * Adapter to allow functions to return values or error conditions at the same time.
 *
 * @tparam Value Is the value to be returned.
 *
 * This adapter can be see as:
 * - an optional value, it implements most of the protocol of std::optional<Value>.
 * - a variant between a Value and an Error, we can have either a Value either an Error.
 *
 */
template < class Value, class Error = std::error_code >
class Return
{
public:
    using value_type = Value;
    using error_type = Error;

private:
    // Internal representation:
    bool m_has_value;
    union {
        Value m_value;
        Error m_error;
    };

public:
    constexpr Return( ) noexcept;
    Return( const Return& );
    Return( Return&& );
    Return( const Value& value );
    Return( Value&& value );
    Return( const Error& error );
    Return( Error&& error );
    ~Return( );

    explicit operator bool( ) const noexcept;

    bool has_value( ) const noexcept;
    Error error( ) const noexcept;

    constexpr Value unsafe_value( ) const &;
    Value unsafe_value( ) &&;

    template < class Dummy = Value >
    constexpr typename std::enable_if< std::is_same<Dummy, Value>::value && std::is_constructible< Dummy >::value, Value >::type
    safe_value( ) const &
    {
        return m_has_value ? m_value : Value( );
    }

    template < class Dummy = Value >
    typename std::enable_if< std::is_same<Dummy, Value>::value && std::is_constructible< Dummy >::value, Value >::type
    safe_value( ) &&
    {
        return m_has_value ? std::move(m_value) : Value( );
    }

private:
    void reset( ) noexcept;
};

template < class Value, class Error >
constexpr Return< Value, Error >::Return( ) noexcept
    : m_has_value( false )
    , m_error( Error{} )
{
}

template < class Value, class Error >
Return< Value, Error >::Return( const Return& other )
    : m_has_value( other.m_has_value )
{
    if ( m_has_value )
    {
        new ( &m_value )( Value )( other.m_value );
    }
    else
    {
        new ( &m_error )( Error )( other.m_error );
    }
}

template < class Value, class Error >
Return< Value, Error >::Return( Return&& other )
    : m_has_value( other.m_has_value )
{
    if ( m_has_value )
    {
        new ( &m_value )( Value )( std::move( other.m_value ) );
    }
    else
    {
        new ( &m_error )( Error )( std::move( other.m_error ) );
    }

    other.reset( );
}

template < class Value, class Error >
Return< Value, Error >::Return( const Value& value )
    : m_has_value( true )
    , m_value( value )
{
}

template < class Value, class Error >
Return< Value, Error >::Return( Value&& value )
    : m_has_value( true )
    , m_value( std::move( value ) )
{
}

template < class Value, class Error >
Return< Value, Error >::Return( const Error& error )
    : m_has_value( false )
    , m_error( error )
{
}

template < class Value, class Error >
Return< Value, Error >::Return( Error&& error )
    : m_has_value( false )
    , m_error( std::move( error ) )
{
}

template < class Value, class Error >
Return< Value, Error >::~Return( )
{
    reset( );
}

template < class Value, class Error >
Return< Value, Error >::operator bool( ) const noexcept
{
    return m_has_value;
}

template < class Value, class Error >
bool
Return< Value, Error >::has_value( ) const noexcept
{
    return m_has_value;
}

template < class Value, class Error >
auto
Return< Value, Error >::error( ) const noexcept -> Error
{
    if ( m_has_value )
    {
        return Error( );
    }
    else
    {
        return m_error;
    }
}

template < class Value, class Error >
constexpr Value
Return< Value, Error >::unsafe_value( ) const &
{
    return m_value;
}

template < class Value, class Error >
Value
Return< Value, Error >::unsafe_value( ) &&
{
    return std::move( m_value );
}

template < class Value, class Error >
void
Return< Value, Error >::reset( ) noexcept
{
    if ( m_has_value )
    {
        m_value.~Value( );
        new ( &m_error )( Error ){};
        m_has_value = false;
    }
    else
    {
        m_error = {};
    }
}

// Partial specialization for "void" Value type.

template < class Error >
class Return< void, Error >
{
public:
    using value_type = void;
    using error_type = Error;

private:
    // Internal representation:
    bool m_has_value;
    Error m_error;

public:
    constexpr Return( ) noexcept;
    Return( const Return& );
    Return( Return&& );
    Return( bool has_value );
    Return( const Error& error );
    Return( Error&& error );
    ~Return( );

    explicit operator bool( ) const noexcept;

    bool has_value( ) const noexcept;
    Error error( ) const noexcept;

    void unsafe_value( ) const & {}
    void unsafe_value( ) && {}
    void safe_value( ) const & {}
    void safe_value( ) && {}

private:
    void reset( ) noexcept;
};

template < class Error >
constexpr Return< void, Error >::Return( ) noexcept
    : m_has_value( false )
    , m_error( Error{} )
{
}

template < class Error >
Return< void, Error >::Return( const Return& other )
    : m_has_value( other.m_has_value )
{
    if ( !m_has_value )
    {
        new ( &m_error )( Error )( other.m_error );
    }
}

template < class Error >
Return< void, Error >::Return( Return&& other )
    : m_has_value( other.m_has_value )
{
    if ( !m_has_value )
    {
        new ( &m_error )( Error )( std::move( other.m_error ) );
    }

    other.reset( );
}

template < class Error >
Return< void, Error >::Return( bool has_value )
    : m_has_value( has_value )
    , m_error( Error{} )
{
}

template < class Error >
Return< void, Error >::Return( const Error& error )
    : m_has_value( false )
    , m_error( error )
{
}

template < class Error >
Return< void, Error >::Return( Error&& error )
    : m_has_value( false )
    , m_error( std::move( error ) )
{
}

template < class Error >
Return< void, Error >::~Return( )
{
    reset( );
}

template < class Error >
Return< void, Error >::operator bool( ) const noexcept
{
    return m_has_value;
}

template < class Error >
bool
Return< void, Error >::has_value( ) const noexcept
{
    return m_has_value;
}

template < class Error >
auto
Return< void, Error >::error( ) const noexcept -> Error
{
    if ( m_has_value )
    {
        return Error( );
    }
    else
    {
        return m_error;
    }
}

template < class Error >
void
Return< void, Error >::reset( ) noexcept
{
    if ( m_has_value )
    {
        new ( &m_error )( Error ){};
        m_has_value = false;
    }
    else
    {
        m_error = {};
    }
}

}
