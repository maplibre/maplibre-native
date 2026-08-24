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

class MultiPoint {
public:
    using SuperTag = Geometry;
    static constexpr auto Name() { return "org/maplibre/geojson/MultiPoint"; };
    static constexpr auto Type() { return "MultiPoint"; };

    static jni::Local<jni::Object<MultiPoint>> New(jni::JNIEnv&, const mln::MultiPoint<double>&);

    static mapbox::geojson::multi_point convert(jni::JNIEnv&, const jni::Object<MultiPoint>&);

    static jni::Local<jni::Object<java::util::List>> coordinates(jni::JNIEnv&, const jni::Object<MultiPoint>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace geojson
} // namespace android
} // namespace mln
