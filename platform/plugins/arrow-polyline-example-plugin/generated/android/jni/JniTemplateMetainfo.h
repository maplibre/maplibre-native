/*

 *
 */

#pragma once

#include <jni.h>

#include <cstdint>
#include <type_traits>

namespace glue_internal {
namespace jni {

template <class Type>
struct IsDerivedFromJObject final
    : std::conditional<
          std::is_base_of<typename std::remove_pointer<jobject>::type, typename std::remove_pointer<Type>::type>::value,
          std::true_type,
          std::false_type>::type {};

template <class... T>
struct JniMethodArgumentTypeChecker final {
    static constexpr bool are_all_jni_types = true;
};

template <class T, class... Args>
struct JniMethodArgumentTypeChecker<T, Args...> final {
    static constexpr bool are_all_jni_types =
        (std::is_same<T, jboolean>::value || std::is_same<T, jbyte>::value || std::is_same<T, jchar>::value ||
         std::is_same<T, jshort>::value || std::is_same<T, jint>::value || std::is_same<T, jlong>::value ||
         std::is_same<T, std::int64_t>::value || // Sometimes is_same_v<std::int64_t, jlong> == 0
         std::is_same<T, jfloat>::value || std::is_same<T, jdouble>::value || IsDerivedFromJObject<T>::value) &&
        JniMethodArgumentTypeChecker<Args...>::are_all_jni_types;
};

template <typename ResultType, bool IsDerivedFromJObject>
struct JniMethodForType final {
    // If result type is wrong you should have compilation error here
};

template <>
struct JniMethodForType<void, false> final {
    static constexpr auto method_ptr = &JNIEnv::CallVoidMethod;
};

template <>
struct JniMethodForType<jboolean, false> final {
    static constexpr auto method_ptr = &JNIEnv::CallBooleanMethod;
};

template <>
struct JniMethodForType<jbyte, false> final {
    static constexpr auto method_ptr = &JNIEnv::CallByteMethod;
};

template <>
struct JniMethodForType<jchar, false> final {
    static constexpr auto method_ptr = &JNIEnv::CallCharMethod;
};

template <>
struct JniMethodForType<jshort, false> final {
    static constexpr auto method_ptr = &JNIEnv::CallShortMethod;
};

template <>
struct JniMethodForType<jint, false> final {
    static constexpr auto method_ptr = &JNIEnv::CallIntMethod;
};

template <>
struct JniMethodForType<jlong, false> final {
    static constexpr auto method_ptr = &JNIEnv::CallLongMethod;
};

// std::is_same<std::int64_t, jlong> == 0 in some compilers even when jlong == long long.
// This specialisation doesn't make sense on such systems (jlong doesn't derive from jobject),
// otherwise prevents from compilation error.
template <>
struct JniMethodForType<std::int64_t, std::is_same<std::int64_t, jlong>::value> final {
    static constexpr auto method_ptr = JniMethodForType<jlong, false>::method_ptr;
};

template <>
struct JniMethodForType<jfloat, false> final {
    static constexpr auto method_ptr = &JNIEnv::CallFloatMethod;
};

template <>
struct JniMethodForType<jdouble, false> final {
    static constexpr auto method_ptr = &JNIEnv::CallDoubleMethod;
};

template <>
struct JniMethodForType<jobject, true> final {
    static constexpr auto method_ptr = &JNIEnv::CallObjectMethod;
};

template <typename ResultType>
struct JniMethodForType<ResultType, true> final {
    static constexpr auto method_ptr = JniMethodForType<jobject, true>::method_ptr;
};

template <class T>
struct JniMethodReturnTypeChecker final {
    static constexpr bool is_jni_type = JniMethodArgumentTypeChecker<T>::are_all_jni_types;
};

template <>
struct JniMethodReturnTypeChecker<void> final {
    static constexpr bool is_jni_type = true;
};

} // namespace jni
} // namespace glue_internal
