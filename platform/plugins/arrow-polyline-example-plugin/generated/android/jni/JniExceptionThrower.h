/*

 *
 */

#pragma once

#include "JniReference.h"

namespace glue_internal {
namespace jni {
class JNIEXPORT JniExceptionThrower final {
public:
    explicit JniExceptionThrower(JNIEnv* jni_env) noexcept;

    ~JniExceptionThrower() noexcept;

    void register_exception(JniReference<jobject> exception) noexcept;

private:
    JNIEnv* const m_jni_env;
    JniReference<jobject> m_exception;
};
} // namespace jni
} // namespace glue_internal
