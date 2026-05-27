#pragma once

#include <jni/jni.hpp>
#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/layermanager/custom_drawable_layer_factory.hpp>
#include "layer.hpp"

namespace mbgl {
namespace android {

class CustomDrawableLayer : public Layer {
public:
    using SuperTag = Layer;
    static constexpr auto Name() { return "org/maplibre/android/style/layers/CustomDrawableLayer"; };

    static void registerNative(jni::JNIEnv&);

    CustomDrawableLayer(jni::JNIEnv&, const jni::String&, jni::jlong);
    CustomDrawableLayer(mbgl::style::CustomDrawableLayer&);
    CustomDrawableLayer(std::unique_ptr<mbgl::style::CustomDrawableLayer>);
    ~CustomDrawableLayer();

    jni::Local<jni::Object<Layer>> createJavaPeer(jni::JNIEnv&);
};

class CustomDrawableJavaLayerPeerFactory final : public JavaLayerPeerFactory, public mbgl::CustomDrawableLayerFactory {
public:
    ~CustomDrawableJavaLayerPeerFactory() override;

    jni::Local<jni::Object<Layer>> createJavaLayerPeer(jni::JNIEnv&, mbgl::style::Layer&) final;
    jni::Local<jni::Object<Layer>> createJavaLayerPeer(jni::JNIEnv& env, std::unique_ptr<mbgl::style::Layer>) final;

    void registerNative(jni::JNIEnv&) final;

    LayerFactory* getLayerFactory() final { return this; }
};

} // namespace android
} // namespace mbgl
