#pragma once

#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/geo.hpp>
#include <mbgl/util/geometry.hpp>

#include <jni/jni.hpp>

namespace mln {
namespace android {

class LatLng : private mln::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/geometry/LatLng"; };

    static jni::Local<jni::Object<LatLng>> New(jni::JNIEnv&, const mln::LatLng&);

    static mln::Point<double> getGeometry(jni::JNIEnv&, const jni::Object<LatLng>&);

    static mln::LatLng getLatLng(jni::JNIEnv&, const jni::Object<LatLng>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
