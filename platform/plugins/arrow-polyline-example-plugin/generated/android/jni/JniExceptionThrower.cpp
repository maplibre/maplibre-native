/*

 *
 */

#include "JniExceptionThrower.h"

namespace glue_internal
{
namespace jni
{

JniExceptionThrower::JniExceptionThrower(JNIEnv* jni_env) noexcept : m_jni_env(jni_env)
{
}

JniExceptionThrower::~JniExceptionThrower() noexcept
{
    if (m_exception)
    {
        m_jni_env->Throw(static_cast<jthrowable>(m_exception.release()));
    }
}

void JniExceptionThrower::register_exception(JniReference<jobject> exception) noexcept
{
    m_exception = std::move(exception);
}

} // namespace jni

} // namespace glue_internal
