/*

 *
 */

#pragma once

#include "JniTemplateMetainfo.h"

#include <jni.h>
#include <cstddef>
#include <memory>

namespace glue_internal
{
namespace jni
{

class JNIEXPORT JniReferenceDeleter final
{
public:
    JniReferenceDeleter() noexcept;

    static JniReferenceDeleter create_local(JNIEnv* jni_env) noexcept;

    static JniReferenceDeleter create_global() noexcept;

    static JniReferenceDeleter create_non_releasing() noexcept;

    void operator () (jobject obj) const noexcept;

private:
    enum class DeleteReferenceMethod
    {
        LOCAL,
        GLOBAL,
        NON_RELEASING
    };

    JniReferenceDeleter(JNIEnv* jni_env, DeleteReferenceMethod delete_reference_method) noexcept;

private:
    JNIEnv* m_jni_env = nullptr;
    DeleteReferenceMethod m_delete_reference_method = DeleteReferenceMethod::NON_RELEASING;
};

template<class JniType>
using JniReference = std::unique_ptr<typename std::remove_pointer<JniType>::type, JniReferenceDeleter>;

template<class T, class Deleter, template<class, class> class SmartPointer>
T* jni_reference_unwrap(const SmartPointer<T, Deleter>& argument) noexcept
{
    return argument.get();
}

template<class T>
T jni_reference_unwrap(const T& argument) noexcept
{
    return argument;
}

template<class JniType>
JniReference<JniType> make_local_ref(JNIEnv* jni_env, JniType jobj) noexcept
{
    static_assert((IsDerivedFromJObject<JniType>::value),
                  "Detected attempt to create local reference to type which is not derived from jobject");
    return JniReference<JniType>(jobj, JniReferenceDeleter::create_local(jni_env));
}

template<class JniType>
JniReference<JniType> make_global_ref(JniType jobj) noexcept
{
    static_assert((IsDerivedFromJObject<JniType>::value),
                  "Detected attempt to create global reference to type which is not derived from jobject");
    return JniReference<JniType>(jobj, JniReferenceDeleter::create_global());
}

template<class JniType>
JniReference<JniType> make_non_releasing_ref(JniType jobj) noexcept
{
    static_assert((IsDerivedFromJObject<JniType>::value),
                  "Detected attempt to create local reference to type which is not derived from jobject");
    return JniReference<JniType>(jobj, JniReferenceDeleter::create_non_releasing());
}

template<class JniType>
JniReference<JniType> new_global_ref(JNIEnv* jni_env, JniType jobj) noexcept
{
    return make_global_ref(static_cast<JniType>(jni_env->NewGlobalRef( jobj )));
}

JNIEXPORT JniReference<jclass> find_class(JNIEnv* jni_env, const char* name) noexcept;

template<class JniType>
JniReference<jclass> get_object_class(JNIEnv* jni_env, const JniType& java_instance) noexcept
{
    return make_local_ref(jni_env, jni_env->GetObjectClass(jni_reference_unwrap(java_instance)));
}

template<class ... Args>
JniReference<jobject> new_object_impl(JNIEnv* env, jclass java_class, jmethodID constructor_id, Args ... args) noexcept
{
    static_assert((JniMethodArgumentTypeChecker<Args...>::are_all_jni_types),
                   "Detected attempt to pass to Java constructor non JNI type parameter");
    return make_local_ref(env, env->NewObject(java_class, constructor_id, args...));
}

template<class JavaClass, class ... Args>
JniReference<jobject> new_object(JNIEnv* env,
                                 const JavaClass& java_class,
                                 jmethodID constructor_id,
                                 const Args& ... args) noexcept
{
    return new_object_impl(env, jni_reference_unwrap(java_class), constructor_id, jni_reference_unwrap(args)...);
}

JNIEXPORT JniReference<jobject>
create_object( JNIEnv* env, const JniReference<jclass>& javaClass ) noexcept;

JNIEXPORT JniReference<jobject>
create_instance_object( JNIEnv* env, const JniReference<jclass>& javaClass, jlong instancePointer ) noexcept;

JNIEXPORT JniReference<jobject>
alloc_object( JNIEnv* env, const JniReference<jclass>& javaClass ) noexcept;

} // namespace jni
} // namespace glue_internal
