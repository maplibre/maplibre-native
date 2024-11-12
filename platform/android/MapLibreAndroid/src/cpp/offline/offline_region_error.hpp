#pragma once

#include <mbgl/storage/offline.hpp>
#include <jni/jni.hpp>

namespace mbgl {
namespace android {

class OfflineRegionError {
public:
    static constexpr auto Name() { return "org/maplibre/android/offline/OfflineRegionError"; };

    static jni::Local<jni::Object<OfflineRegionError>> New(jni::JNIEnv&, mbgl::Response::Error);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mbgl
