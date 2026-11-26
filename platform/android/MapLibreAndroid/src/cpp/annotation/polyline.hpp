#pragma once

#include <mln/annotation/annotation.hpp>

#include "multi_point.hpp"

namespace mln {
namespace android {

class Polyline : private MultiPoint {
public:
    static constexpr auto Name() { return "org/maplibre/android/annotations/Polyline"; };

    static mln::LineAnnotation toAnnotation(jni::JNIEnv&, const jni::Object<Polyline>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
