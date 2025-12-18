/*

 *
 */

#pragma once

#include "JniReference.h"

#include <jni.h>

namespace glue_internal {
namespace jni {

JNIEXPORT std::int64_t get_class_native_handle(JNIEnv* env, const JniReference<jobject>& jobj) noexcept;
JNIEXPORT std::int64_t get_class_native_handle(JNIEnv* env, jobject jobj) noexcept;

} // namespace jni
} // namespace glue_internal
