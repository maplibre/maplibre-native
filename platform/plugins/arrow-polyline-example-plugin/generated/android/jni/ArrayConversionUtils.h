/*

 *
 */

#pragma once

#include "BoxingConversionUtils.h"
#include "JniClassCache.h"
#include "JniCppConversionUtils.h"
#include "JniJavaContainers.h"
#include "JniReference.h"
#include "JniTypeId.h"

#include <memory>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace glue_internal
{
namespace jni
{
// Forward declare map conversion routines

template < typename K, typename V, typename Hash >
std::unordered_map< K, V, Hash >
convert_from_jni(JNIEnv* const env, const JniReference<jobject>& java_map, TypeId<std::unordered_map< K, V, Hash>>);

template<typename K, typename V, typename Hash>
std::optional<std::unordered_map<K, V, Hash>>
convert_from_jni(JNIEnv* const env,
                 const JniReference<jobject>& _jMap,
                 TypeId<std::optional<std::unordered_map<K, V, Hash>>>);

template < typename K, typename V, typename Hash >
JniReference<jobject>
convert_to_jni(JNIEnv* const env, const std::unordered_map< K, V, Hash >& input);

template<typename K, typename V, typename Hash>
JniReference<jobject>
convert_to_jni(JNIEnv* const env, const std::optional<std::unordered_map<K, V, Hash>>& input);

// Forward declare set conversion routines

template <typename T, typename Hash, typename std::enable_if<!std::is_enum<T>::value, int>::type = 0>
JniReference<jobject>
convert_to_jni(JNIEnv* const env, const std::unordered_set<T, Hash>& input);

template <typename T, typename Hash, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
JniReference<jobject>
convert_to_jni(JNIEnv* const env, const std::unordered_set<T, Hash>& input);

template<typename T, typename Hash>
JniReference<jobject>
convert_to_jni(JNIEnv* const env, const std::optional<std::unordered_set<T, Hash>>& input);

template <typename T, typename Hash>
std::unordered_set<T, Hash>
convert_from_jni(JNIEnv* const env, const JniReference<jobject>& java_set, TypeId<std::unordered_set<T, Hash>>);

template<typename T, typename Hash>
std::optional<std::unordered_set<T, Hash>>
convert_from_jni(JNIEnv* const env,
                 const JniReference<jobject>& java_set,
                 TypeId<std::optional<std::unordered_set<T, Hash>>>);

template<typename Container>
std::optional<Container>
convert_optional_container_from_jni(JNIEnv* const env,
                                    const JniReference<jobject>& java_container,
                                    TypeId<std::optional<Container>>);

template<typename Container>
JniReference<jobject>
convert_optional_container_to_jni(JNIEnv* const env, const std::optional<Container>& optional_container);

// Templated functions to create ArrayLists from C++ vectors and vice versa

template < typename T >
JniReference<jobject>
convert_to_jni(JNIEnv* const env, const std::vector< T >& input)
{
    JavaArrayListAdder list_appender{env};

    for (const T& element : input)
    {
        list_appender.add(convert_to_jni(env, element));
    }

    return list_appender.fetch_container();
}

template < typename T >
std::vector< T >
convert_from_jni(JNIEnv* const env, const JniReference<jobject>& array_list, TypeId<std::vector< T >>)
{
    std::vector< T > result;

    if (env->IsSameObject(array_list.get(), nullptr))
    {
        return result;
    }

    const JavaListIterator list_iterator(env, array_list);

    const jint length = list_iterator.length();

    result.reserve(length);

    for (jint i = 0; i < length; i++)
    {
        result.emplace_back(convert_from_jni(env, list_iterator.get(array_list, i), TypeId<T>{}));
    }

    return result;
}

template <typename T>
JniReference<jobject>
convert_to_jni_optimized(JNIEnv* const env, const std::vector<std::shared_ptr<T>>& input, const char* class_name) {
    auto vector_ptr = new (std::nothrow) std::shared_ptr<std::vector<std::shared_ptr<T>>>(
        new (std::nothrow) std::vector<std::shared_ptr<T>>(input)
    );
    return create_instance_object(env, find_class(env, class_name), reinterpret_cast<jlong>(vector_ptr));
}

// Templated functions to create HashMaps from C++ unordered_maps and vice versa

template < typename K, typename V, typename Hash >
JniReference<jobject>
convert_to_jni(JNIEnv* const env, const std::unordered_map< K, V, Hash >& input)
{
    JavaHashMapAdder map_adder{env};

    for (const auto& pair : input)
    {
        map_adder.add(convert_to_jni(env, pair.first), convert_to_jni(env, pair.second));
    }

    return map_adder.fetch_hash_map();
}

template < typename K, typename V, typename Hash >
::std::unordered_map< K, V, Hash >
convert_from_jni(JNIEnv* const env, const JniReference<jobject>& java_map, TypeId<::std::unordered_map< K, V, Hash >>)
{
    ::std::unordered_map< K, V, Hash > result{};

    if (env->IsSameObject(java_map.get(), nullptr))
    {
        return result;
    }

    const JavaMapIterator map_iterator(env, java_map);

    while(map_iterator.has_next())
    {
        const auto& key_value = map_iterator.next();
        result.emplace(convert_from_jni(env, key_value.first, TypeId<K>{}),
                       convert_from_jni(env, key_value.second, TypeId<V>{}));
    }

    return result;
}

// Templated functions to create HashSet from C++ unordered_set and vice versa

template <typename T, typename Hash, typename std::enable_if<!std::is_enum<T>::value, int>::type>
JniReference<jobject>
convert_to_jni(JNIEnv* const env, const std::unordered_set<T, Hash>& input)
{
    JavaHashSetAdder set_adder{env};

    for (const T& element : input)
    {
        set_adder.add(convert_to_jni(env, element));
    }

    return set_adder.fetch_container();
}

template <typename T, typename Hash, typename std::enable_if<std::is_enum<T>::value, int>::type>
JniReference<jobject>
convert_to_jni(JNIEnv* env, const std::unordered_set<T, Hash>& input)
{
    JavaEnumSetAdder set_adder{env, CachedJavaClass<T>::java_class};

    for (const T& element : input)
    {
        set_adder.add(convert_to_jni(env, element));
    }

    return set_adder.fetch_container();
}

template <typename T, typename Hash >
std::unordered_set<T, Hash>
convert_from_jni(JNIEnv* const env, const JniReference<jobject>& java_set, TypeId<std::unordered_set<T, Hash>>)
{
    std::unordered_set<T, Hash> result{};

    if (env->IsSameObject(java_set.get(), nullptr))
    {
        return result;
    }

    const JavaSetIterator set_iterator(env, java_set);

    while (set_iterator.has_next())
    {
        result.emplace(convert_from_jni(env, set_iterator.next(), TypeId<T>{}));
    }

    return result;
}

// Optionals

template<typename K, typename V, typename Hash>
std::optional<std::unordered_map<K, V, Hash>>
convert_from_jni(JNIEnv* const env,
                 const JniReference<jobject>& java_map,
                 TypeId<std::optional<std::unordered_map<K, V, Hash>>>)
{
    return convert_optional_container_from_jni(env, java_map, TypeId<std::optional<std::unordered_map<K, V, Hash>>>{});
}

template<typename T>
std::optional<std::vector<T>>
convert_from_jni(JNIEnv* const env,
                 const JniReference<jobject>& array_list,
                 TypeId<std::optional<std::vector<T>>>)
{
    return convert_optional_container_from_jni(env, array_list, TypeId<std::optional<std::vector<T>>>{});
}

template<typename K, typename V, typename Hash>
JniReference<jobject>
convert_to_jni(JNIEnv* const env, const std::optional<std::unordered_map<K, V, Hash>>& input)
{
    return convert_optional_container_to_jni(env, input);
}

template<typename T>
JniReference<jobject>
convert_to_jni(JNIEnv* const env, const std::optional<std::vector<T>>& input)
{
    return convert_optional_container_to_jni(env, input);
}

template<typename T, typename Hash >
JniReference<jobject>
convert_to_jni(JNIEnv* const env, const std::optional<std::unordered_set<T, Hash>>& input)
{
    return convert_optional_container_to_jni(env, input);
}

template<typename T, typename Hash>
std::optional<std::unordered_set<T, Hash>>
convert_from_jni(JNIEnv* const env,
                 const JniReference<jobject>& java_set,
                 TypeId<std::optional<std::unordered_set<T, Hash>>>)
{
    return convert_optional_container_from_jni(env, java_set, TypeId<std::optional<std::unordered_set<T, Hash>>>{});
}

template<typename Container>
std::optional<Container>
convert_optional_container_from_jni(JNIEnv* const env,
                                    const JniReference<jobject>& java_container,
                                    TypeId<std::optional<Container>>)
{
    return java_container ? convert_from_jni(env, java_container, TypeId<Container>{}) : std::optional<Container>{};
}

template<typename Container>
JniReference<jobject>
convert_optional_container_to_jni(JNIEnv* const env, const std::optional<Container>& optional_container)
{
    return optional_container ? convert_to_jni(env, *optional_container) : JniReference<jobject>{};
}


}
}
