#pragma once

#include <mln/util/geojson.hpp>
#include <mln/util/noncopyable.hpp>

#include "../java/util.hpp"
#include "geometry.hpp"

#include <jni/jni.hpp>

namespace mln {
namespace android {
namespace geojson {

class MultiPolygon {
public:
    using SuperTag = Geometry;
    static constexpr auto Name() { return "org/maplibre/geojson/MultiPolygon"; };
    static constexpr auto Type() { return "MultiPolygon"; };

    static jni::Local<jni::Object<MultiPolygon>> New(jni::JNIEnv&, const mln::MultiPolygon<double>&);

    static mapbox::geojson::multi_polygon convert(jni::JNIEnv&, const jni::Object<MultiPolygon>&);

    static jni::Local<jni::Object<java::util::List>> coordinates(jni::JNIEnv&, const jni::Object<MultiPolygon>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace geojson
} // namespace android
} // namespace mln
