#pragma once

#include <mln/util/util.hpp>

#include <string>
#include <jni/jni.hpp>

typedef struct _jmethodID* jmethodID;
typedef struct _JavaVM JavaVM;
typedef struct _JNIEnv JNIEnv;

namespace mln {
namespace android {

extern JavaVM* theJVM;

extern std::string cachePath;
extern std::string dataPath;

bool attach_jni_thread(JavaVM* vm, JNIEnv** env, std::string threadName);
void detach_jni_thread(JavaVM* vm, JNIEnv** env, bool detach);

} // namespace android
} // namespace mln
