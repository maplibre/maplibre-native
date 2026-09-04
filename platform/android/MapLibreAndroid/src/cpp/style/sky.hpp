#pragma once

#include <jni/jni.hpp>
#include <mln/style/sky.hpp>
#include <mln/util/noncopyable.hpp>

#include "transition_options.hpp"

#include <memory>

namespace mln {
namespace android {

/**
 * Android peer for a self-contained sky snapshot.
 *
 * Unlike Light, this peer intentionally does not retain a raw reference into
 * Style. Sky is optional and can be replaced or removed, so a raw reference
 * would become dangling. NativeMapView copies this snapshot into Style.
 */
class Sky : private mln::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/style/sky/Sky"; }

    static void registerNative(jni::JNIEnv&);
    static jni::Local<jni::Object<Sky>> createJavaSkyPeer(jni::JNIEnv&, const mln::style::Sky&);
    static const Sky* getNativePeer(jni::JNIEnv&, const jni::Object<Sky>&);

    explicit Sky(jni::JNIEnv&);
    explicit Sky(const mln::style::Sky&);

    const mln::style::Sky& getSky() const;

    void setProperty(jni::JNIEnv&, const jni::String&, const jni::Object<>&);

    jni::Local<jni::Object<>> getAtmosphereBlend(jni::JNIEnv&);
    void setAtmosphereBlendTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getAtmosphereBlendTransition(jni::JNIEnv&);

    jni::Local<jni::Object<>> getFogColor(jni::JNIEnv&);
    void setFogColorTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getFogColorTransition(jni::JNIEnv&);

    jni::Local<jni::Object<>> getFogGroundBlend(jni::JNIEnv&);
    void setFogGroundBlendTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getFogGroundBlendTransition(jni::JNIEnv&);

    jni::Local<jni::Object<>> getHorizonColor(jni::JNIEnv&);
    void setHorizonColorTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getHorizonColorTransition(jni::JNIEnv&);

    jni::Local<jni::Object<>> getHorizonFogBlend(jni::JNIEnv&);
    void setHorizonFogBlendTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getHorizonFogBlendTransition(jni::JNIEnv&);

    jni::Local<jni::Object<>> getSkyColor(jni::JNIEnv&);
    void setSkyColorTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getSkyColorTransition(jni::JNIEnv&);

    jni::Local<jni::Object<>> getSkyHorizonBlend(jni::JNIEnv&);
    void setSkyHorizonBlendTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getSkyHorizonBlendTransition(jni::JNIEnv&);

private:
    jni::Local<jni::Object<Sky>> createJavaPeer(jni::JNIEnv&);

    std::unique_ptr<mln::style::Sky> sky;
};

} // namespace android
} // namespace mln
