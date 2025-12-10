/*

 *
 */

#include "JniReference.h"

#include "JniBase.h"

namespace glue_internal
{
namespace jni
{

JniReferenceDeleter::JniReferenceDeleter() noexcept = default;

JniReferenceDeleter JniReferenceDeleter::create_local(JNIEnv* jni_env) noexcept
{
    return JniReferenceDeleter(jni_env, DeleteReferenceMethod::LOCAL);
}

JniReferenceDeleter JniReferenceDeleter::create_global() noexcept
{
    return JniReferenceDeleter(nullptr, DeleteReferenceMethod::GLOBAL);
}

JniReferenceDeleter JniReferenceDeleter::create_non_releasing() noexcept
{
    return JniReferenceDeleter(nullptr, DeleteReferenceMethod::NON_RELEASING);
}

JniReferenceDeleter::JniReferenceDeleter(JNIEnv* jni_env, DeleteReferenceMethod delete_reference_method) noexcept
    : m_jni_env(jni_env)
    , m_delete_reference_method(delete_reference_method)
{
}

void JniReferenceDeleter::operator () (jobject obj) const noexcept
{
    if (obj)
    {
        switch (m_delete_reference_method)
        {
        case DeleteReferenceMethod::LOCAL:
            m_jni_env->DeleteLocalRef( obj );
            break;
        case DeleteReferenceMethod::GLOBAL:
            if (JNIEnv* jni_env = ::glue_internal::jni::getJniEnvironmentForCurrentThread( ))
            {
                jni_env->DeleteGlobalRef( obj );
            }
            break;
        case DeleteReferenceMethod::NON_RELEASING:
            // noop
            break;
        }
    }
}


JniReference<jclass> find_class(JNIEnv* jni_env, const char* name) noexcept
{
    return make_local_ref(jni_env, jni_env->FindClass(name));
}

JniReference<jobject>
create_object( JNIEnv* env, const JniReference<jclass>& javaClass ) noexcept
{
    const char* name = "<init>";
    const char* signature = "()V";
    auto theConstructor = env->GetMethodID( javaClass.get(), name, signature );

    return new_object(env, javaClass, theConstructor);
}

JniReference<jobject>
create_instance_object( JNIEnv* env, const JniReference<jclass>& javaClass, jlong instancePointer ) noexcept
{
    const char* name = "<init>";
    const char* signature = "(JLjava/lang/Object;)V";
    auto theConstructor = env->GetMethodID( javaClass.get(), name, signature );

    // On Mac platform `jlong` and `jobject` are `long long`, but `NULL` is `long`, so need to cast here.
    return new_object(env, javaClass, theConstructor, instancePointer, static_cast<jobject>(NULL));
}

JniReference<jobject>
alloc_object( JNIEnv* env, const JniReference<jclass>& javaClass ) noexcept
{
    return make_local_ref( env, env->AllocObject( javaClass.get( ) ) );
}

} // namespace jni
} // namespace glue_internal
