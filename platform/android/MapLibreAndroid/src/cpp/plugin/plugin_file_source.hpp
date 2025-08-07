#pragma once

#include <jni/jni.hpp>
#include <mbgl/plugin/plugin_file_source.hpp>

namespace mbgl {
namespace android {

class PluginProtocolHandlerResource {
public:
    static constexpr auto Name() { return "org/maplibre/android/plugin/PluginProtocolHandlerResource"; };
    static void registerNative(jni::JNIEnv &);

    static jni::Local<jni::Object<PluginProtocolHandlerResource>> Create(jni::JNIEnv &);
    static void Update(jni::JNIEnv &, jni::Object<PluginProtocolHandlerResource> &, const mbgl::Resource &);
};

class PluginProtocolHandlerResponse {
public:
    static constexpr auto Name() { return "org/maplibre/android/plugin/PluginProtocolHandlerResponse"; };
    static void registerNative(jni::JNIEnv &);

    static jni::Local<jni::Object<PluginProtocolHandlerResponse>> Create(jni::JNIEnv &);
    static void Update(jni::JNIEnv &, jni::Object<PluginProtocolHandlerResponse> &, mbgl::Response &);
};

class PluginFileSource {
public:
    static constexpr auto Name() { return "org/maplibre/android/plugin/PluginFileSource"; };

    // static mbgl::PluginFileSource getFileSource(jni::JNIEnv&, const jni::Object<PluginFileSource>&);

    static jni::Global<jni::Object<PluginProtocolHandlerResource>> createJavaResource(jni::JNIEnv &env,
                                                                                      const mbgl::Resource &resource);

    static void registerNative(jni::JNIEnv &);
};

class PluginFileSourceContainer {
public:
    jni::Global<jni::Object<PluginFileSource>> _fileSource;
};

} // namespace android
} // namespace mbgl
