#pragma once

#include <mln/util/noncopyable.hpp>
#include <jni/jni.hpp>

#include <string>

#include "../geometry/lat_lng.hpp"

namespace mln {
namespace android {

class Marker : private mln::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/annotations/Marker"; };

    static mln::Point<double> getPosition(jni::JNIEnv&, const jni::Object<Marker>&);

    static std::string getIconId(jni::JNIEnv&, const jni::Object<Marker>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
