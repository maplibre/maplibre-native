/*

 *
 */

#include "JniReference.h"

namespace glue_internal {
namespace jni {

void throw_new_exception(JNIEnv* const jni_env,
                         const char* const exception_class,
                         const char* const exception_text) noexcept {
    const auto exceptionClass = find_class(jni_env, exception_class);
    jni_env->ThrowNew(exceptionClass.get(), exception_text);
}

void throw_new_out_of_memory_exception(JNIEnv* const jni_env) noexcept {
    throw_new_exception(jni_env, "java/lang/OutOfMemoryError", "Cannot allocate native memory.");
}

void throw_new_null_pointer_exception(JNIEnv* const jni_env) noexcept {
    throw_new_exception(jni_env, "java/lang/NullPointerException", "");
}

void throw_new_index_out_of_bounds_exception(JNIEnv* const jni_env) noexcept {
    throw_new_exception(jni_env, "java/lang/IndexOutOfBoundsException", "List index out of bounds.");
}

void throw_new_runtime_exception(JNIEnv* const jni_env, const char* const message) noexcept {
    throw_new_exception(jni_env, "java/lang/RuntimeException", message);
}

} // namespace jni
} // namespace glue_internal
