/*

 *
 */

#pragma once

#include <jni.h>

namespace glue_internal {
namespace jni {

JNIEXPORT void throw_new_out_of_memory_exception(JNIEnv* jni_env) noexcept;
JNIEXPORT void throw_new_null_pointer_exception(JNIEnv* jni_env) noexcept;
JNIEXPORT void throw_new_index_out_of_bounds_exception(JNIEnv* jni_env) noexcept;
JNIEXPORT void throw_new_runtime_exception(JNIEnv* jni_env, const char* message) noexcept;

} // namespace jni
} // namespace glue_internal
