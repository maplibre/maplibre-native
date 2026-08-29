#pragma once

#include <jni/jni.hpp>

namespace mln {
namespace android {

class PluginRegistry {
public:
    static constexpr auto Name() { return "org/maplibre/android/plugins/MapLibrePluginRegistry"; }

    static jni::jboolean isRegistered(jni::JNIEnv&, const jni::Class<PluginRegistry>&, const jni::String&);
    static jni::jint count(jni::JNIEnv&, const jni::Class<PluginRegistry>&);
    static jni::Local<jni::String> idAt(jni::JNIEnv&, const jni::Class<PluginRegistry>&, jni::jint);
    static jni::jlong registrationFunctionAddress(jni::JNIEnv&, const jni::Class<PluginRegistry>&);
    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
