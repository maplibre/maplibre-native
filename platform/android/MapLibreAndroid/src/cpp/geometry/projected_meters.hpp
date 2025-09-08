#pragma once

#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/geometry.hpp>

#include <jni/jni.hpp>

namespace mbgl {
namespace android {

class ProjectedMeters : private mbgl::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/geometry/ProjectedMeters"; };

    static jni::Local<jni::Object<ProjectedMeters>> New(jni::JNIEnv&, double, double);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mbgl
