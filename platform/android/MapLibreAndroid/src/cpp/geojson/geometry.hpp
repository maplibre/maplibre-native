#pragma once

#include <mln/util/geometry.hpp>

#include "../java/util.hpp"

#include <jni/jni.hpp>

namespace mln {
namespace android {
namespace geojson {

class Geometry {
public:
    static constexpr auto Name() { return "org/maplibre/geojson/Geometry"; };

    static jni::Local<jni::Object<Geometry>> New(jni::JNIEnv&, mln::Geometry<double>);

    static mln::Geometry<double> convert(jni::JNIEnv&, const jni::Object<Geometry>&);

    static std::string getType(jni::JNIEnv&, const jni::Object<Geometry>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace geojson
} // namespace android
} // namespace mln
