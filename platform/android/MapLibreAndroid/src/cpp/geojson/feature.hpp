#pragma once

#include <mln/util/feature.hpp>

#include <jni/jni.hpp>

namespace mln {
namespace android {
namespace geojson {

class Feature {
public:
    static constexpr auto Name() { return "org/maplibre/geojson/Feature"; };

    static mln::GeoJSONFeature convert(jni::JNIEnv&, const jni::Object<Feature>&);
    static jni::Local<jni::Array<jni::Object<Feature>>> convert(jni::JNIEnv&, const std::vector<mln::Feature>&);
    static jni::Local<jni::Array<jni::Object<Feature>>> convert(jni::JNIEnv&, const std::vector<mln::GeoJSONFeature>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace geojson
} // namespace android
} // namespace mln
