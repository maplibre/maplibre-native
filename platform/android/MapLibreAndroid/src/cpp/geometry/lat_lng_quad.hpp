#pragma once

#include <mln/util/noncopyable.hpp>
#include <mln/util/geo.hpp>
#include <mln/util/geometry.hpp>

#include <jni/jni.hpp>
#include <array>

namespace mln {
namespace android {

class LatLngQuad : private mln::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/geometry/LatLngQuad"; };

    static jni::Local<jni::Object<LatLngQuad>> New(jni::JNIEnv&, std::array<mln::LatLng, 4>);

    static std::array<mln::LatLng, 4> getLatLngArray(jni::JNIEnv&, const jni::Object<LatLngQuad>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
