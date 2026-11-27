#pragma once

#include "../geometry/coordinate_reference_system.hpp"

#include <jni/jni.hpp>

namespace mln {
namespace android {

class Style {
public:
    static constexpr auto Name() { return "org/maplibre/android/maps/Style"; };

    static void addProjection(jni::JNIEnv&, const jni::Class<Style>&, const jni::Object<CoordinateReferenceSystem>&);

    static void removeProjection(jni::JNIEnv&, const jni::Class<Style>&, const jni::String&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
