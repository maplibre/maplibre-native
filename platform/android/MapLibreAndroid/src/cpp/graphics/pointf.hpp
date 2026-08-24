#pragma once

#include <mln/util/noncopyable.hpp>

#include <jni/jni.hpp>

namespace mln {
namespace android {

class PointF : private mln::util::noncopyable {
public:
    static constexpr auto Name() { return "android/graphics/PointF"; };

    static jni::Local<jni::Object<PointF>> New(jni::JNIEnv&, float, float);

    static mln::ScreenCoordinate getScreenCoordinate(jni::JNIEnv&, const jni::Object<PointF>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
