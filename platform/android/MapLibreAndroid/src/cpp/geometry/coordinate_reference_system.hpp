#pragma once

#include <mln/util/crs.hpp>

#include <jni/jni.hpp>

namespace mln {
namespace android {

class CoordinateReferenceSystem {
public:
    static constexpr auto Name() { return "org/maplibre/android/geometry/CoordinateReferenceSystem"; };

    /// Its converters call the Kotlin projection on whichever thread the map converts on, and give NaN for a call that
    /// throws.
    static mln::CRS getCRS(jni::JNIEnv&, const jni::Object<CoordinateReferenceSystem>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
