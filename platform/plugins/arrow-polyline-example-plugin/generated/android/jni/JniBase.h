/*

 *
 */

#pragma once

#include <jni.h>

// Depending on how the conversion code is built, it might be necessary to rename
// JNI_OnLoad to allow for a custom implementation or similar. In that case the user
// needs to ensure it is called manually. If it has a custom name, declare the function.
#ifdef GLUECODIUM_JNI_ONLOAD
JNIEXPORT jint GLUECODIUM_JNI_ONLOAD(JavaVM* vm, void*);
#else
#define GLUECODIUM_JNI_ONLOAD JNI_OnLoad
#endif

namespace glue_internal {
namespace jni {

JNIEXPORT JavaVM* get_java_vm() noexcept;

JNIEXPORT JNIEnv* getJniEnvironmentForCurrentThread() noexcept;

} // namespace jni
} // namespace glue_internal
