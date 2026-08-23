#pragma once

#include <mln/storage/offline.hpp>
#include <jni/jni.hpp>

namespace mln {
namespace android {

class OfflineRegionStatus {
public:
    static constexpr auto Name() { return "org/maplibre/android/offline/OfflineRegionStatus"; };

    static jni::Local<jni::Object<OfflineRegionStatus>> New(jni::JNIEnv&, mln::OfflineRegionStatus status);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
