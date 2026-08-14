#pragma once

#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/geo.hpp>
#include <mbgl/util/geometry.hpp>

#include <jni/jni.hpp>

namespace mln {
namespace android {

class LatLngBounds : private mln::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/geometry/LatLngBounds"; };

    static jni::Local<jni::Object<LatLngBounds>> New(jni::JNIEnv&, mln::LatLngBounds);

    static mln::LatLngBounds getLatLngBounds(jni::JNIEnv&, const jni::Object<LatLngBounds>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
