#include "custom_drawable_layer.hpp"

#include <string>

#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace android {

CustomDrawableLayer::CustomDrawableLayer(jni::JNIEnv& env, const jni::String& layerId, jni::jlong host)
    : Layer(std::make_unique<mbgl::style::CustomDrawableLayer>(
          jni::Make<std::string>(env, layerId),
          std::unique_ptr<mbgl::style::CustomDrawableLayerHost>(
              reinterpret_cast<mbgl::style::CustomDrawableLayerHost*>(host)))) {}

CustomDrawableLayer::CustomDrawableLayer(mbgl::style::CustomDrawableLayer& coreLayer)
    : Layer(coreLayer) {}

CustomDrawableLayer::CustomDrawableLayer(std::unique_ptr<mbgl::style::CustomDrawableLayer> coreLayer)
    : Layer(std::move(coreLayer)) {}

CustomDrawableLayer::~CustomDrawableLayer() = default;

namespace {
jni::Local<jni::Object<Layer>> createJavaPeer(jni::JNIEnv& env, Layer* layer) {
    static auto& javaClass = jni::Class<CustomDrawableLayer>::Singleton(env);
    static auto constructor = javaClass.GetConstructor<jni::jlong>(env);
    return javaClass.New(env, constructor, reinterpret_cast<jni::jlong>(layer));
}
} // namespace

CustomDrawableJavaLayerPeerFactory::~CustomDrawableJavaLayerPeerFactory() = default;

jni::Local<jni::Object<Layer>> CustomDrawableJavaLayerPeerFactory::createJavaLayerPeer(jni::JNIEnv& env,
                                                                                       mbgl::style::Layer& layer) {
    return createJavaPeer(env, new CustomDrawableLayer(static_cast<mbgl::style::CustomDrawableLayer&>(layer)));
}

jni::Local<jni::Object<Layer>> CustomDrawableJavaLayerPeerFactory::createJavaLayerPeer(
    jni::JNIEnv& env, std::unique_ptr<mbgl::style::Layer> layer) {
    return createJavaPeer(env,
                          new CustomDrawableLayer(std::unique_ptr<mbgl::style::CustomDrawableLayer>(
                              static_cast<mbgl::style::CustomDrawableLayer*>(layer.release()))));
}

void CustomDrawableJavaLayerPeerFactory::registerNative(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<CustomDrawableLayer>::Singleton(env);

#define METHOD(MethodPtr, name) jni::MakeNativePeerMethod<decltype(MethodPtr), (MethodPtr)>(name)

    jni::RegisterNativePeer<CustomDrawableLayer>(env,
                                                 javaClass,
                                                 "nativePtr",
                                                 jni::MakePeer<CustomDrawableLayer, const jni::String&, jni::jlong>,
                                                 "initialize",
                                                 "finalize");
}

} // namespace android
} // namespace mbgl
