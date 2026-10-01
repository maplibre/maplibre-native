#pragma once

#include <mln/storage/offline.hpp>
#include <jni/jni.hpp>

namespace mln {
namespace android {

class OfflineRegionError {
public:
    static constexpr auto Name() { return "org/maplibre/android/offline/OfflineRegionError"; };

    static jni::Local<jni::Object<OfflineRegionError>> New(jni::JNIEnv&, mln::Response::Error);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
