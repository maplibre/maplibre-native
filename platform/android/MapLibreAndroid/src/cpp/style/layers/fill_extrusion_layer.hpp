// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#pragma once

#include "layer.hpp"
#include "../transition_options.hpp"
#include <mln/layermanager/fill_extrusion_layer_factory.hpp>
#include <mln/style/layers/fill_extrusion_layer.hpp>
#include <jni/jni.hpp>

namespace mln {
namespace android {

class FillExtrusionLayer : public Layer {
public:
    using SuperTag = Layer;
    static constexpr auto Name() { return "org/maplibre/android/style/layers/FillExtrusionLayer"; };

    FillExtrusionLayer(jni::JNIEnv&, jni::String&, jni::String&);

    FillExtrusionLayer(mln::style::FillExtrusionLayer&);

    FillExtrusionLayer(std::unique_ptr<mln::style::FillExtrusionLayer>);

    ~FillExtrusionLayer();

    // Properties

    jni::Local<jni::Object<jni::ObjectTag>> getFillExtrusionRoundedCornerDistance(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getFillExtrusionOpacity(jni::JNIEnv&);
    void setFillExtrusionOpacityTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getFillExtrusionOpacityTransition(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getFillExtrusionColor(jni::JNIEnv&);
    void setFillExtrusionColorTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getFillExtrusionColorTransition(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getFillExtrusionTranslate(jni::JNIEnv&);
    void setFillExtrusionTranslateTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getFillExtrusionTranslateTransition(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getFillExtrusionTranslateAnchor(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getFillExtrusionPattern(jni::JNIEnv&);
    void setFillExtrusionPatternTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getFillExtrusionPatternTransition(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getFillExtrusionHeight(jni::JNIEnv&);
    void setFillExtrusionHeightTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getFillExtrusionHeightTransition(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getFillExtrusionBase(jni::JNIEnv&);
    void setFillExtrusionBaseTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getFillExtrusionBaseTransition(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getFillExtrusionVerticalGradient(jni::JNIEnv&);

}; // class FillExtrusionLayer

class FillExtrusionJavaLayerPeerFactory final : public JavaLayerPeerFactory, public mln::FillExtrusionLayerFactory {
public:
    ~FillExtrusionJavaLayerPeerFactory() override;

    // JavaLayerPeerFactory overrides.
    jni::Local<jni::Object<Layer>> createJavaLayerPeer(jni::JNIEnv&, mln::style::Layer&) final;
    jni::Local<jni::Object<Layer>> createJavaLayerPeer(jni::JNIEnv& env, std::unique_ptr<mln::style::Layer>) final;

    void registerNative(jni::JNIEnv&) final;

    LayerFactory* getLayerFactory() final { return this; }

}; // class FillExtrusionJavaLayerPeerFactory

} // namespace android
} // namespace mln
