#pragma once

#include <mln/util/geojson.hpp>
#include <mln/util/geometry.hpp>
#include <mln/util/noncopyable.hpp>

#include "geometry.hpp"
#include "../java/util.hpp"

#include <jni/jni.hpp>

namespace mln {
namespace android {
namespace geojson {

class LineString {
public:
    using SuperTag = Geometry;
    static constexpr auto Name() { return "org/maplibre/geojson/LineString"; };
    static constexpr auto Type() { return "LineString"; };

    static jni::Local<jni::Object<LineString>> New(jni::JNIEnv&, const mln::LineString<double>&);

    static mapbox::geojson::line_string convert(jni::JNIEnv&, const jni::Object<LineString>&);

    static mapbox::geojson::line_string convert(jni::JNIEnv&, const jni::Object<java::util::List /*<Point>*/>&);

    static jni::Local<jni::Object<java::util::List>> coordinates(jni::JNIEnv&, const jni::Object<LineString>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace geojson
} // namespace android
} // namespace mln
