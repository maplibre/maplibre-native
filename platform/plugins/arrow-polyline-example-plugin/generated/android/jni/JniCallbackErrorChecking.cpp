/*

 *
 */

#include "JniCallbackErrorChecking.h"

#include "JniCallJavaMethod.h"
#include "JniCppConversionUtils.h"

#include <string>

namespace glue_internal {
namespace jni {

void checkExceptionAndReportIfAny(JNIEnv* env) {
    if (!env->ExceptionCheck()) {
        return;
    }

    jthrowable exception = env->ExceptionOccurred();
    env->ExceptionDescribe();
    env->ExceptionClear();

    std::string errorMessage = "REASON: ";

    auto exceptionToString = call_java_method<jstring>(env, exception, "toString", "()Ljava/lang/String;");
    errorMessage += convert_from_jni(env, exceptionToString, TypeId<std::string>{});

    auto frames = call_java_method<jobjectArray>(env, exception, "getStackTrace", "()[Ljava/lang/StackTraceElement;");
    auto framesLength = static_cast<size_t>(env->GetArrayLength(frames.get()));

    if (0 == frames || 0 == framesLength) {
        env->FatalError(errorMessage.c_str());
    }

    for (auto i = 0; i < framesLength; ++i) {
        auto stackFrame = make_local_ref(env, env->GetObjectArrayElement(frames.get(), i));
        auto stackFrameToString = call_java_method<jstring>(env, stackFrame.get(), "toString", "()Ljava/lang/String;");
        const auto functionCallStackLine = convert_from_jni(env, stackFrameToString, TypeId<std::string>{});

        // This is done so that all stack frame messages will be on same line with actual exception message.
        if (i == 0) {
            errorMessage += " at " + functionCallStackLine;
        } else {
            errorMessage += " <- " + functionCallStackLine;
        }
    }

    env->FatalError(errorMessage.c_str());
}

} // namespace jni
} // namespace glue_internal
