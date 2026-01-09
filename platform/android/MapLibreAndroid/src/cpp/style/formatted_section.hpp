#pragma once

#include <mbgl/util/noncopyable.hpp>

#include <jni/jni.hpp>

namespace mbgl {
namespace android {

class FormattedSection : private mbgl::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/style/types/FormattedSection"; };

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mbgl
