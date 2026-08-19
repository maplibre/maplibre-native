#pragma once

#include <mbgl/annotation/annotation.hpp>

#include "multi_point.hpp"

namespace mln {
namespace android {

class Polygon : private MultiPoint {
public:
    static constexpr auto Name() { return "org/maplibre/android/annotations/Polygon"; };

    static mln::FillAnnotation toAnnotation(jni::JNIEnv&, const jni::Object<Polygon>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
