#pragma once

#include <mln/util/noncopyable.hpp>
#include <mln/map/camera.hpp>

#include <jni/jni.hpp>

namespace mln {
namespace android {

class CameraPosition : private mln::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/camera/CameraPosition"; };

    static jni::Local<jni::Object<CameraPosition>> New(jni::JNIEnv&, mln::CameraOptions, float pixelRatio);

    static mln::CameraOptions getCameraOptions(jni::JNIEnv&, const jni::Object<CameraPosition>&, float pixelRatio);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
