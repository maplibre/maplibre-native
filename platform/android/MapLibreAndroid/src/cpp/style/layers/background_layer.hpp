// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#pragma once

#include "layer.hpp"
#include "../transition_options.hpp"
#include <mln/layermanager/background_layer_factory.hpp>
#include <mln/style/layers/background_layer.hpp>
#include <jni/jni.hpp>

namespace mln {
namespace android {

class BackgroundLayer : public Layer {
public:
    using SuperTag = Layer;
    static constexpr auto Name() { return "org/maplibre/android/style/layers/BackgroundLayer"; };

    BackgroundLayer(jni::JNIEnv&, jni::String&);

    BackgroundLayer(mln::style::BackgroundLayer&);

    BackgroundLayer(std::unique_ptr<mln::style::BackgroundLayer>);

    ~BackgroundLayer();

    // Properties

    jni::Local<jni::Object<jni::ObjectTag>> getBackgroundColor(jni::JNIEnv&);
    void setBackgroundColorTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getBackgroundColorTransition(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getBackgroundPattern(jni::JNIEnv&);
    void setBackgroundPatternTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getBackgroundPatternTransition(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getBackgroundOpacity(jni::JNIEnv&);
    void setBackgroundOpacityTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getBackgroundOpacityTransition(jni::JNIEnv&);

}; // class BackgroundLayer

class BackgroundJavaLayerPeerFactory final : public JavaLayerPeerFactory, public mln::BackgroundLayerFactory {
public:
    ~BackgroundJavaLayerPeerFactory() override;

    // JavaLayerPeerFactory overrides.
    jni::Local<jni::Object<Layer>> createJavaLayerPeer(jni::JNIEnv&, mln::style::Layer&) final;
    jni::Local<jni::Object<Layer>> createJavaLayerPeer(jni::JNIEnv& env, std::unique_ptr<mln::style::Layer>) final;

    void registerNative(jni::JNIEnv&) final;

    LayerFactory* getLayerFactory() final { return this; }

}; // class BackgroundJavaLayerPeerFactory

} // namespace android
} // namespace mln
