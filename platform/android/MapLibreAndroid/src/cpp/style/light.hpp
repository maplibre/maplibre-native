#pragma once

#include <mln/util/noncopyable.hpp>

#include <jni/jni.hpp>
#include <mln/style/light.hpp>
#include "transition_options.hpp"
#include "position.hpp"
#include <mln/style/types.hpp>
#include <mln/style/property_value.hpp>

namespace mln {
namespace android {

using namespace style;

class Light : private mln::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/style/light/Light"; };

    static void registerNative(jni::JNIEnv&);

    static jni::Local<jni::Object<Light>> createJavaLightPeer(jni::JNIEnv&, mln::Map&, mln::style::Light&);

    Light(mln::Map&, mln::style::Light&);

    void setAnchor(jni::JNIEnv&, const jni::String&);
    jni::Local<jni::String> getAnchor(jni::JNIEnv&);
    void setPosition(jni::JNIEnv&, const jni::Object<Position>&);
    jni::Local<jni::Object<Position>> getPosition(jni::JNIEnv&);
    void setPositionTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getPositionTransition(jni::JNIEnv&);
    void setColor(jni::JNIEnv&, const jni::String&);
    jni::Local<jni::String> getColor(jni::JNIEnv&);
    void setColorTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getColorTransition(jni::JNIEnv&);
    void setIntensity(jni::JNIEnv&, jni::jfloat);
    jni::jfloat getIntensity(jni::JNIEnv&);
    void setIntensityTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getIntensityTransition(jni::JNIEnv&);
    jni::Local<jni::Object<Light>> createJavaPeer(jni::JNIEnv&);

protected:
    // Raw reference to the light
    mln::style::Light& light;

    // Map is set when the light is retrieved
    mln::Map* map;
};
} // namespace android
} // namespace mln
