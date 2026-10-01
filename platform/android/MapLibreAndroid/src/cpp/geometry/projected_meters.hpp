#pragma once

#include <mln/util/noncopyable.hpp>
#include <mln/util/geometry.hpp>

#include <jni/jni.hpp>

namespace mln {
namespace android {

class ProjectedMeters : private mln::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/geometry/ProjectedMeters"; };

    static jni::Local<jni::Object<ProjectedMeters>> New(jni::JNIEnv&, double, double);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
