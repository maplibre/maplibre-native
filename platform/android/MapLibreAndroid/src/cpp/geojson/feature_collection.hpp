#pragma once

#include "../java/util.hpp"

#include <mln/util/geojson.hpp>

#include <jni/jni.hpp>

namespace mln {
namespace android {
namespace geojson {

class FeatureCollection {
public:
    static constexpr auto Name() { return "org/maplibre/geojson/FeatureCollection"; };

    static mln::FeatureCollection convert(jni::JNIEnv&, const jni::Object<FeatureCollection>&);

    static jni::Local<jni::Object<java::util::List>> features(jni::JNIEnv&, const jni::Object<FeatureCollection>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace geojson
} // namespace android
} // namespace mln
