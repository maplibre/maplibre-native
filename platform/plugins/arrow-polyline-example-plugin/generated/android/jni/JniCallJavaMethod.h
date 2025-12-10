/*

 *
 */

#pragma once

#include "JniTemplateMetainfo.h"
#include "JniReference.h"

#include <string>

namespace glue_internal {
namespace jni {

template <class ResultType, class... Args>
ResultType call_java_method_impl(JNIEnv* const jni_env,
                                 const jobject java_object,
                                 const jmethodID method_id,
                                 Args... args) noexcept {
    static_assert((JniMethodArgumentTypeChecker<Args...>::are_all_jni_types),
                  "Detected attempt to pass to Java method non JNI type parameter");
    static_assert((JniMethodReturnTypeChecker<ResultType>::is_jni_type),
                  "Detected attempt to return from Java method non JNI type value");

    auto method_ptr = JniMethodForType<ResultType, IsDerivedFromJObject<ResultType>::value>::method_ptr;
    return static_cast<ResultType>((jni_env->*method_ptr)(java_object, method_id, args...));
}

template <bool IsResultTypeDerivedFromJObject, class ResultType>
struct ProcessJavaMethodCall final {};

template <class ResultType>
struct ProcessJavaMethodCall<false, ResultType> final {
    template <class... Args>
    static ResultType call(JNIEnv* const jni_env,
                           const jobject java_object,
                           const jmethodID method_id,
                           Args... args) noexcept {
        return call_java_method_impl<ResultType>(jni_env, java_object, method_id, args...);
    }
};

template <class ResultType>
struct ProcessJavaMethodCall<true, ResultType> final {
    template <class... Args>
    static JniReference<ResultType> call(JNIEnv* const jni_env,
                                         const jobject java_object,
                                         const jmethodID method_id,
                                         Args... args) noexcept {
        return make_local_ref(jni_env, call_java_method_impl<ResultType>(jni_env, java_object, method_id, args...));
    }
};

template <>
struct ProcessJavaMethodCall<false, void> final {
    template <class... Args>
    static void call(JNIEnv* const jni_env,
                     const jobject java_object,
                     const jmethodID method_id,
                     Args... args) noexcept {
        call_java_method_impl<void>(jni_env, java_object, method_id, args...);
    }
};

template <class ResultType, class JavaObject, class... Args>
typename std::conditional<IsDerivedFromJObject<ResultType>::value, JniReference<ResultType>, ResultType>::type
call_java_method(JNIEnv* const jni_env,
                 const JavaObject& java_object,
                 const jmethodID method_id,
                 const Args&... args) noexcept {
    return ProcessJavaMethodCall<IsDerivedFromJObject<ResultType>::value, ResultType>::call(
        jni_env, jni_reference_unwrap(java_object), method_id, jni_reference_unwrap(args)...);
}

template <class ResultType, class JavaObject, class... Args>
typename std::conditional<IsDerivedFromJObject<ResultType>::value, JniReference<ResultType>, ResultType>::type
call_java_method(JNIEnv* const jni_env,
                 const JniReference<jclass>& java_class,
                 const JavaObject& java_object,
                 const char* method_name,
                 const char* jni_signature,
                 const Args&... args) noexcept {
    const jmethodID method_id = jni_env->GetMethodID(java_class.get(), method_name, jni_signature);
    return call_java_method<ResultType>(jni_env, java_object, method_id, args...);
}

template <class ResultType, class JavaObject, class... Args>
typename std::conditional<IsDerivedFromJObject<ResultType>::value, JniReference<ResultType>, ResultType>::type
call_java_method(JNIEnv* const jni_env,
                 const char* java_class_name,
                 const JavaObject& java_object,
                 const char* method_name,
                 const char* jni_signature,
                 const Args&... args) noexcept {
    return call_java_method<ResultType>(
        jni_env, find_class(jni_env, java_class_name), java_object, method_name, jni_signature, args...);
}

template <class ResultType, class JavaObject, class... Args>
typename std::conditional<IsDerivedFromJObject<ResultType>::value, JniReference<ResultType>, ResultType>::type
call_java_method(JNIEnv* const jni_env,
                 const JavaObject& java_object,
                 const char* method_name,
                 const char* jni_signature,
                 const Args&... args) noexcept {
    return call_java_method<ResultType>(
        jni_env, get_object_class(jni_env, java_object), java_object, method_name, jni_signature, args...);
}

} // namespace jni
} // namespace glue_internal
