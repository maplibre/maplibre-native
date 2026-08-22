#pragma once

#include <mln/util/geometry.hpp>

#include "geometry.hpp"

#include <jni/jni.hpp>

namespace mln {
namespace android {
namespace geojson {

class Point {
public:
    using SuperTag = Geometry;
    static constexpr auto Name() { return "org/maplibre/geojson/Point"; };
    static constexpr auto Type() { return "Point"; };

    static jni::Local<jni::Object<Point>> New(jni::JNIEnv&, const mln::Point<double>&);
    static mln::Point<double> convert(jni::JNIEnv&, const jni::Object<Point>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace geojson
} // namespace android
} // namespace mln
