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
#include <memory>
#include <mutex>
#include <new>
#include <unordered_map>

template < class ProxyType >
class CachedProxyBase
{
private:
    using CacheEntryType = std::pair< std::weak_ptr< ProxyType >, void* >;
    struct Cache
    {
        std::unordered_map< const void*, CacheEntryType > cache;
        std::unordered_map< const void*, const void* > reverse_cache;
        std::mutex mutex;
        bool alive;

        Cache( )
            : alive(true)
        {
        }

        ~Cache( )
        {
            alive = false;
        }
    };

protected:
    CachedProxyBase( ) = default;

public:
    template < class FunctionTable >
    static std::shared_ptr< ProxyType >
    get_proxy( FunctionTable&& function_table )
    {
        const void* key = function_table.swift_pointer;
        Cache& cache = get_cache( );
        std::lock_guard< std::mutex > cache_lock( cache.mutex );

        auto cached_weak = cache.cache.find( key );
        if ( cached_weak != cache.cache.end( ) )
        {
            auto cached = cached_weak->second.first.lock( );
            if ( cached )
            {
                // swift pointer was retained when function table was filled, release it when no
                // proxy is created
                function_table.release( function_table.swift_pointer );
                return cached;
            }
        }
        auto proxy = std::shared_ptr< ProxyType >( new ( std::nothrow )
                                                       ProxyType( std::move( function_table ) ) );
        if ( proxy )
        {
            proxy->m_key = key;
            cache.cache[ key ] = std::make_pair( proxy, proxy.get( ) );
            cache.reverse_cache[ proxy.get( ) ] = key;
        }
        return proxy;
    }

    template <class FunctionTable>
    static std::shared_ptr<ProxyType>
    get_proxy_no_cache(FunctionTable&& function_table) {
        auto proxy =
            std::shared_ptr<ProxyType>(new (std::nothrow) ProxyType(std::move(function_table)));
        if (proxy) {
            proxy->m_key = function_table.swift_pointer;
        }
        return proxy;
    }

    static const void*
    get_swift_object( const void* proxy )
    {
        Cache& cache = get_cache( );
        std::lock_guard< std::mutex > cache_lock( cache.mutex );

        auto it = cache.reverse_cache.find( proxy );
        if ( it != cache.reverse_cache.end( ) )
        {
            return it->second;
        }
        return nullptr;
    }

    virtual ~CachedProxyBase( )
    {
        Cache& cache = get_cache( );
        if ( !cache.alive )
        {
            // The cache may have been destroyed if this was stored in a global varaible and
            // destructed after the static variable for the cache.
            return;
        }

        std::lock_guard< std::mutex > cache_lock( cache.mutex );

        auto it = cache.cache.find( m_key );
        if ( it != cache.cache.end( ) )
        {
            cache.reverse_cache.erase( it->second.second );
            cache.cache.erase( it );
        }
    }

private:
    // Handle cases of non-deterministic ordering of construction and destruction of global
    // variables. This is on two ends:
    // 1. Use function static variable to ensure it's constructed on first use.
    // 2. Use an "alive" flag to ensure it won't crash if destructed early. (e.g. storing a
    //    shared_ptr to a proxy object as a static variable)
    static Cache& get_cache( )
    {
        static Cache cache;
        return cache;
    }

private:
    const void* m_key;
};
