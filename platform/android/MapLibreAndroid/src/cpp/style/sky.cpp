#include "sky.hpp"

#include "android_conversion.hpp"
#include "conversion/property_value.hpp"
#include "conversion/transition_options.hpp"

#include <mln/style/conversion_impl.hpp>
#include <mln/util/logging.hpp>

#include <string>

namespace mln {
namespace android {

Sky::Sky(jni::JNIEnv&)
    : sky(std::make_unique<mln::style::Sky>()) {}

Sky::Sky(const mln::style::Sky& coreSky)
    : sky(std::make_unique<mln::style::Sky>(coreSky.impl)) {}

const mln::style::Sky& Sky::getSky() const {
    return *sky;
}

jni::Local<jni::Object<Sky>> Sky::createJavaSkyPeer(jni::JNIEnv& env, const mln::style::Sky& coreSky) {
    auto peer = std::make_unique<Sky>(coreSky);
    auto result = peer->createJavaPeer(env);
    peer.release();
    return result;
}

const Sky* Sky::getNativePeer(jni::JNIEnv& env, const jni::Object<Sky>& javaSky) {
    static auto& javaClass = jni::Class<Sky>::Singleton(env);
    static auto nativePtrField = javaClass.GetField<jlong>(env, "nativePtr");
    return reinterpret_cast<const Sky*>(javaSky.Get(env, nativePtrField));
}

jni::Local<jni::Object<Sky>> Sky::createJavaPeer(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<Sky>::Singleton(env);
    static auto constructor = javaClass.GetConstructor<jni::jlong>(env);
    return javaClass.New(env, constructor, reinterpret_cast<jni::jlong>(this));
}

void Sky::setProperty(jni::JNIEnv& env, const jni::String& jname, const jni::Object<>& jvalue) {
    const std::string name = jni::Make<std::string>(env, jname);
    const auto error = sky->setProperty(name, Value(env, jvalue));
    if (error) {
        mln::Log::Error(mln::Event::JNI, "Error setting sky property " + name + ": " + error->message);
    }
}

namespace {

jni::Local<jni::Object<TransitionOptions>> toJavaTransition(jni::JNIEnv& env,
                                                            const mln::style::TransitionOptions& options) {
    using namespace mln::android::conversion;
    return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
}

mln::style::TransitionOptions makeTransition(jlong duration, jlong delay) {
    mln::style::TransitionOptions options;
    options.duration.emplace(mln::Milliseconds(duration));
    options.delay.emplace(mln::Milliseconds(delay));
    return options;
}

} // namespace

#define SKY_PROPERTY(Name)                                                                    \
    jni::Local<jni::Object<>> Sky::get##Name(jni::JNIEnv& env) {                              \
        using namespace mln::android::conversion;                                             \
        return std::move(*convert<jni::Local<jni::Object<>>>(env, sky->get##Name()));         \
    }                                                                                         \
    void Sky::set##Name##Transition(jni::JNIEnv&, jlong duration, jlong delay) {              \
        sky->set##Name##Transition(makeTransition(duration, delay));                          \
    }                                                                                         \
    jni::Local<jni::Object<TransitionOptions>> Sky::get##Name##Transition(jni::JNIEnv& env) { \
        return toJavaTransition(env, sky->get##Name##Transition());                           \
    }

SKY_PROPERTY(AtmosphereBlend)
SKY_PROPERTY(FogColor)
SKY_PROPERTY(FogGroundBlend)
SKY_PROPERTY(HorizonColor)
SKY_PROPERTY(HorizonFogBlend)
SKY_PROPERTY(SkyColor)
SKY_PROPERTY(SkyHorizonBlend)

#undef SKY_PROPERTY

void Sky::registerNative(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<Sky>::Singleton(env);

#define METHOD(MethodPtr, name) jni::MakeNativePeerMethod<decltype(MethodPtr), (MethodPtr)>(name)
    jni::RegisterNativePeer<Sky>(env,
                                 javaClass,
                                 "nativePtr",
                                 jni::MakePeer<Sky>,
                                 "initialize",
                                 "finalize",
                                 METHOD(&Sky::setProperty, "nativeSetProperty"),
                                 METHOD(&Sky::getAtmosphereBlend, "nativeGetAtmosphereBlend"),
                                 METHOD(&Sky::getAtmosphereBlendTransition, "nativeGetAtmosphereBlendTransition"),
                                 METHOD(&Sky::setAtmosphereBlendTransition, "nativeSetAtmosphereBlendTransition"),
                                 METHOD(&Sky::getFogColor, "nativeGetFogColor"),
                                 METHOD(&Sky::getFogColorTransition, "nativeGetFogColorTransition"),
                                 METHOD(&Sky::setFogColorTransition, "nativeSetFogColorTransition"),
                                 METHOD(&Sky::getFogGroundBlend, "nativeGetFogGroundBlend"),
                                 METHOD(&Sky::getFogGroundBlendTransition, "nativeGetFogGroundBlendTransition"),
                                 METHOD(&Sky::setFogGroundBlendTransition, "nativeSetFogGroundBlendTransition"),
                                 METHOD(&Sky::getHorizonColor, "nativeGetHorizonColor"),
                                 METHOD(&Sky::getHorizonColorTransition, "nativeGetHorizonColorTransition"),
                                 METHOD(&Sky::setHorizonColorTransition, "nativeSetHorizonColorTransition"),
                                 METHOD(&Sky::getHorizonFogBlend, "nativeGetHorizonFogBlend"),
                                 METHOD(&Sky::getHorizonFogBlendTransition, "nativeGetHorizonFogBlendTransition"),
                                 METHOD(&Sky::setHorizonFogBlendTransition, "nativeSetHorizonFogBlendTransition"),
                                 METHOD(&Sky::getSkyColor, "nativeGetSkyColor"),
                                 METHOD(&Sky::getSkyColorTransition, "nativeGetSkyColorTransition"),
                                 METHOD(&Sky::setSkyColorTransition, "nativeSetSkyColorTransition"),
                                 METHOD(&Sky::getSkyHorizonBlend, "nativeGetSkyHorizonBlend"),
                                 METHOD(&Sky::getSkyHorizonBlendTransition, "nativeGetSkyHorizonBlendTransition"),
                                 METHOD(&Sky::setSkyHorizonBlendTransition, "nativeSetSkyHorizonBlendTransition"));
}

} // namespace android
} // namespace mln
