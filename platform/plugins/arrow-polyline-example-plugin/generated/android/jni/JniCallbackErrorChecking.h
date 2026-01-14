/*

 *
 */

#pragma once

#include <jni.h>

namespace glue_internal {
namespace jni {

JNIEXPORT void checkExceptionAndReportIfAny(JNIEnv* env);

}
} // namespace glue_internal
