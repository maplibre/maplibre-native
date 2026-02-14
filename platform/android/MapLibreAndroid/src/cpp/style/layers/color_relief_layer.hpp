// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#pragma once

#include "layer.hpp"
#include "../transition_options.hpp"
#include <mbgl/layermanager/color_relief_layer_factory.hpp>
#include <mbgl/style/layers/color_relief_layer.hpp>
#include <jni/jni.hpp>

namespace mbgl {
namespace android {

class ColorReliefLayer : public Layer {
public:
    using SuperTag = Layer;
    static constexpr auto Name() { return "org/maplibre/android/style/layers/ColorReliefLayer"; };

    ColorReliefLayer(jni::JNIEnv&, jni::String&, jni::String&);

    ColorReliefLayer(mbgl::style::ColorReliefLayer&);

    ColorReliefLayer(std::unique_ptr<mbgl::style::ColorReliefLayer>);

    ~ColorReliefLayer();

    // Properties

    jni::Local<jni::Object<jni::ObjectTag>> getColorReliefOpacity(jni::JNIEnv&);
    void setColorReliefOpacityTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getColorReliefOpacityTransition(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getColorReliefColor(jni::JNIEnv&);

}; // class ColorReliefLayer

class ColorReliefJavaLayerPeerFactory final : public JavaLayerPeerFactory, public mbgl::ColorReliefLayerFactory {
public:
    ~ColorReliefJavaLayerPeerFactory() override;

    // JavaLayerPeerFactory overrides.
    jni::Local<jni::Object<Layer>> createJavaLayerPeer(jni::JNIEnv&, mbgl::style::Layer&) final;
    jni::Local<jni::Object<Layer>> createJavaLayerPeer(jni::JNIEnv& env, std::unique_ptr<mbgl::style::Layer>) final;

    void registerNative(jni::JNIEnv&) final;

    LayerFactory* getLayerFactory() final { return this; }

}; // class ColorReliefJavaLayerPeerFactory

} // namespace android
} // namespace mbgl
