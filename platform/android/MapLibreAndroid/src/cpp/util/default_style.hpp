#pragma once

#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/default_style.hpp>

#include <jni/jni.hpp>

namespace mbgl {
namespace android {

class DefaultStyle : private mbgl::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/util/DefaultStyle"; };

    static jni::Local<jni::Object<DefaultStyle>> New(jni::JNIEnv&, const mbgl::util::DefaultStyle&);
    static mbgl::util::DefaultStyle getDefaultStyle(jni::JNIEnv&, const jni::Object<DefaultStyle>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mbgl
