// This file is generated. Edit android/platform/scripts/generate-style-code.js, then run `make android-style-code`.

#pragma once

#include "layer.hpp"
#include "../transition_options.hpp"
#include <mbgl/layermanager/location_indicator_layer_factory.hpp>
#include <mbgl/style/layers/location_indicator_layer.hpp>
#include <jni/jni.hpp>

namespace mbgl {
namespace android {

class LocationIndicatorLayer : public Layer {
public:
    using SuperTag = Layer;
    static constexpr auto Name() { return "com/mapbox/mapboxsdk/style/layers/LocationIndicatorLayer"; };

    LocationIndicatorLayer(jni::JNIEnv&, jni::String&);

    LocationIndicatorLayer(mbgl::style::LocationIndicatorLayer&);

    LocationIndicatorLayer(std::unique_ptr<mbgl::style::LocationIndicatorLayer>);

    ~LocationIndicatorLayer();

    // Properties

    jni::Local<jni::Object<jni::ObjectTag>> getTopImage(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getBearingImage(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getShadowImage(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getPerspectiveCompensation(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getImageTiltDisplacement(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getBearing(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getLocation(jni::JNIEnv&);
    void setLocationTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getLocationTransition(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getAccuracyRadius(jni::JNIEnv&);
    void setAccuracyRadiusTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getAccuracyRadiusTransition(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getTopImageSize(jni::JNIEnv&);
    void setTopImageSizeTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getTopImageSizeTransition(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getBearingImageSize(jni::JNIEnv&);
    void setBearingImageSizeTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getBearingImageSizeTransition(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getShadowImageSize(jni::JNIEnv&);
    void setShadowImageSizeTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getShadowImageSizeTransition(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getAccuracyRadiusColor(jni::JNIEnv&);
    void setAccuracyRadiusColorTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getAccuracyRadiusColorTransition(jni::JNIEnv&);

    jni::Local<jni::Object<jni::ObjectTag>> getAccuracyRadiusBorderColor(jni::JNIEnv&);
    void setAccuracyRadiusBorderColorTransition(jni::JNIEnv&, jlong duration, jlong delay);
    jni::Local<jni::Object<TransitionOptions>> getAccuracyRadiusBorderColorTransition(jni::JNIEnv&);

}; // class LocationIndicatorLayer

class LocationIndicatorJavaLayerPeerFactory final : public JavaLayerPeerFactory,  public mbgl::LocationIndicatorLayerFactory {
public:
    ~LocationIndicatorJavaLayerPeerFactory() override;

    // JavaLayerPeerFactory overrides.
    jni::Local<jni::Object<Layer>> createJavaLayerPeer(jni::JNIEnv&, mbgl::style::Layer&) final;
    jni::Local<jni::Object<Layer>> createJavaLayerPeer(jni::JNIEnv& env, std::unique_ptr<mbgl::style::Layer>) final;

    void registerNative(jni::JNIEnv&) final;

    LayerFactory* getLayerFactory() final { return this; }

};  // class LocationIndicatorJavaLayerPeerFactory

} // namespace android
} // namespace mbgl
