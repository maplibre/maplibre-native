// This file is generated. Edit android/platform/scripts/generate-style-code.js, then run `make android-style-code`.

#include "location_indicator_layer.hpp"

#include <string>

#include "../conversion/property_value.hpp"
#include "../conversion/transition_options.hpp"

#include <mbgl/style/layer_impl.hpp>

namespace mbgl {
namespace android {

    inline mbgl::style::LocationIndicatorLayer& toLocationIndicatorLayer(mbgl::style::Layer& layer) {
        return static_cast<mbgl::style::LocationIndicatorLayer&>(layer);
    }

    /**
     * Creates an owning peer object (for layers not attached to the map) from the JVM side
     */
    LocationIndicatorLayer::LocationIndicatorLayer(jni::JNIEnv& env, jni::String& layerId)
        : Layer(std::make_unique<mbgl::style::LocationIndicatorLayer>(jni::Make<std::string>(env, layerId))) {
    }

    /**
     * Creates a non-owning peer object (for layers currently attached to the map)
     */
    LocationIndicatorLayer::LocationIndicatorLayer(mbgl::style::LocationIndicatorLayer& coreLayer)
        : Layer(coreLayer) {
    }

    /**
     * Creates an owning peer object (for layers not attached to the map)
     */
    LocationIndicatorLayer::LocationIndicatorLayer(std::unique_ptr<mbgl::style::LocationIndicatorLayer> coreLayer)
        : Layer(std::move(coreLayer)) {
    }

    LocationIndicatorLayer::~LocationIndicatorLayer() = default;

    // Property getters

    jni::Local<jni::Object<>> LocationIndicatorLayer::getTopImage(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        return std::move(*convert<jni::Local<jni::Object<>>>(env, toLocationIndicatorLayer(layer).getTopImage()));
    }

    jni::Local<jni::Object<>> LocationIndicatorLayer::getBearingImage(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        return std::move(*convert<jni::Local<jni::Object<>>>(env, toLocationIndicatorLayer(layer).getBearingImage()));
    }

    jni::Local<jni::Object<>> LocationIndicatorLayer::getShadowImage(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        return std::move(*convert<jni::Local<jni::Object<>>>(env, toLocationIndicatorLayer(layer).getShadowImage()));
    }

    jni::Local<jni::Object<>> LocationIndicatorLayer::getPerspectiveCompensation(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        return std::move(*convert<jni::Local<jni::Object<>>>(env, toLocationIndicatorLayer(layer).getPerspectiveCompensation()));
    }

    jni::Local<jni::Object<>> LocationIndicatorLayer::getImageTiltDisplacement(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        return std::move(*convert<jni::Local<jni::Object<>>>(env, toLocationIndicatorLayer(layer).getImageTiltDisplacement()));
    }

    jni::Local<jni::Object<>> LocationIndicatorLayer::getBearing(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        return std::move(*convert<jni::Local<jni::Object<>>>(env, toLocationIndicatorLayer(layer).getBearing()));
    }

    jni::Local<jni::Object<>> LocationIndicatorLayer::getLocation(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        return std::move(*convert<jni::Local<jni::Object<>>>(env, toLocationIndicatorLayer(layer).getLocation()));
    }

    jni::Local<jni::Object<TransitionOptions>> LocationIndicatorLayer::getLocationTransition(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        mbgl::style::TransitionOptions options = toLocationIndicatorLayer(layer).getLocationTransition();
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
    }

    void LocationIndicatorLayer::setLocationTransition(jni::JNIEnv&, jlong duration, jlong delay) {
        mbgl::style::TransitionOptions options;
        options.duration.emplace(mbgl::Milliseconds(duration));
        options.delay.emplace(mbgl::Milliseconds(delay));
        toLocationIndicatorLayer(layer).setLocationTransition(options);
    }

    jni::Local<jni::Object<>> LocationIndicatorLayer::getAccuracyRadius(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        return std::move(*convert<jni::Local<jni::Object<>>>(env, toLocationIndicatorLayer(layer).getAccuracyRadius()));
    }

    jni::Local<jni::Object<TransitionOptions>> LocationIndicatorLayer::getAccuracyRadiusTransition(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        mbgl::style::TransitionOptions options = toLocationIndicatorLayer(layer).getAccuracyRadiusTransition();
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
    }

    void LocationIndicatorLayer::setAccuracyRadiusTransition(jni::JNIEnv&, jlong duration, jlong delay) {
        mbgl::style::TransitionOptions options;
        options.duration.emplace(mbgl::Milliseconds(duration));
        options.delay.emplace(mbgl::Milliseconds(delay));
        toLocationIndicatorLayer(layer).setAccuracyRadiusTransition(options);
    }

    jni::Local<jni::Object<>> LocationIndicatorLayer::getTopImageSize(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        return std::move(*convert<jni::Local<jni::Object<>>>(env, toLocationIndicatorLayer(layer).getTopImageSize()));
    }

    jni::Local<jni::Object<TransitionOptions>> LocationIndicatorLayer::getTopImageSizeTransition(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        mbgl::style::TransitionOptions options = toLocationIndicatorLayer(layer).getTopImageSizeTransition();
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
    }

    void LocationIndicatorLayer::setTopImageSizeTransition(jni::JNIEnv&, jlong duration, jlong delay) {
        mbgl::style::TransitionOptions options;
        options.duration.emplace(mbgl::Milliseconds(duration));
        options.delay.emplace(mbgl::Milliseconds(delay));
        toLocationIndicatorLayer(layer).setTopImageSizeTransition(options);
    }

    jni::Local<jni::Object<>> LocationIndicatorLayer::getBearingImageSize(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        return std::move(*convert<jni::Local<jni::Object<>>>(env, toLocationIndicatorLayer(layer).getBearingImageSize()));
    }

    jni::Local<jni::Object<TransitionOptions>> LocationIndicatorLayer::getBearingImageSizeTransition(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        mbgl::style::TransitionOptions options = toLocationIndicatorLayer(layer).getBearingImageSizeTransition();
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
    }

    void LocationIndicatorLayer::setBearingImageSizeTransition(jni::JNIEnv&, jlong duration, jlong delay) {
        mbgl::style::TransitionOptions options;
        options.duration.emplace(mbgl::Milliseconds(duration));
        options.delay.emplace(mbgl::Milliseconds(delay));
        toLocationIndicatorLayer(layer).setBearingImageSizeTransition(options);
    }

    jni::Local<jni::Object<>> LocationIndicatorLayer::getShadowImageSize(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        return std::move(*convert<jni::Local<jni::Object<>>>(env, toLocationIndicatorLayer(layer).getShadowImageSize()));
    }

    jni::Local<jni::Object<TransitionOptions>> LocationIndicatorLayer::getShadowImageSizeTransition(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        mbgl::style::TransitionOptions options = toLocationIndicatorLayer(layer).getShadowImageSizeTransition();
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
    }

    void LocationIndicatorLayer::setShadowImageSizeTransition(jni::JNIEnv&, jlong duration, jlong delay) {
        mbgl::style::TransitionOptions options;
        options.duration.emplace(mbgl::Milliseconds(duration));
        options.delay.emplace(mbgl::Milliseconds(delay));
        toLocationIndicatorLayer(layer).setShadowImageSizeTransition(options);
    }

    jni::Local<jni::Object<>> LocationIndicatorLayer::getAccuracyRadiusColor(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        return std::move(*convert<jni::Local<jni::Object<>>>(env, toLocationIndicatorLayer(layer).getAccuracyRadiusColor()));
    }

    jni::Local<jni::Object<TransitionOptions>> LocationIndicatorLayer::getAccuracyRadiusColorTransition(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        mbgl::style::TransitionOptions options = toLocationIndicatorLayer(layer).getAccuracyRadiusColorTransition();
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
    }

    void LocationIndicatorLayer::setAccuracyRadiusColorTransition(jni::JNIEnv&, jlong duration, jlong delay) {
        mbgl::style::TransitionOptions options;
        options.duration.emplace(mbgl::Milliseconds(duration));
        options.delay.emplace(mbgl::Milliseconds(delay));
        toLocationIndicatorLayer(layer).setAccuracyRadiusColorTransition(options);
    }

    jni::Local<jni::Object<>> LocationIndicatorLayer::getAccuracyRadiusBorderColor(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        return std::move(*convert<jni::Local<jni::Object<>>>(env, toLocationIndicatorLayer(layer).getAccuracyRadiusBorderColor()));
    }

    jni::Local<jni::Object<TransitionOptions>> LocationIndicatorLayer::getAccuracyRadiusBorderColorTransition(jni::JNIEnv& env) {
        using namespace mbgl::android::conversion;
        mbgl::style::TransitionOptions options = toLocationIndicatorLayer(layer).getAccuracyRadiusBorderColorTransition();
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
    }

    void LocationIndicatorLayer::setAccuracyRadiusBorderColorTransition(jni::JNIEnv&, jlong duration, jlong delay) {
        mbgl::style::TransitionOptions options;
        options.duration.emplace(mbgl::Milliseconds(duration));
        options.delay.emplace(mbgl::Milliseconds(delay));
        toLocationIndicatorLayer(layer).setAccuracyRadiusBorderColorTransition(options);
    }


    // LocationIndicatorJavaLayerPeerFactory

    LocationIndicatorJavaLayerPeerFactory::~LocationIndicatorJavaLayerPeerFactory() = default;

    namespace {
        jni::Local<jni::Object<Layer>> createJavaPeer(jni::JNIEnv& env, Layer* layer) {
            static auto& javaClass = jni::Class<LocationIndicatorLayer>::Singleton(env);
            static auto constructor = javaClass.GetConstructor<jni::jlong>(env);
            return javaClass.New(env, constructor, reinterpret_cast<jni::jlong>(layer));
        }
    }  // namespace

    jni::Local<jni::Object<Layer>> LocationIndicatorJavaLayerPeerFactory::createJavaLayerPeer(jni::JNIEnv& env, mbgl::style::Layer& layer) {
        assert(layer.baseImpl->getTypeInfo() == getTypeInfo());
        return createJavaPeer(env, new LocationIndicatorLayer(toLocationIndicatorLayer(layer)));
    }

    jni::Local<jni::Object<Layer>> LocationIndicatorJavaLayerPeerFactory::createJavaLayerPeer(jni::JNIEnv& env, std::unique_ptr<mbgl::style::Layer> layer) {
        assert(layer->baseImpl->getTypeInfo() == getTypeInfo());
        return createJavaPeer(env, new LocationIndicatorLayer(std::unique_ptr<mbgl::style::LocationIndicatorLayer>(static_cast<mbgl::style::LocationIndicatorLayer*>(layer.release()))));
    }

    void LocationIndicatorJavaLayerPeerFactory::registerNative(jni::JNIEnv& env) {
        // Lookup the class
        static auto& javaClass = jni::Class<LocationIndicatorLayer>::Singleton(env);

        #define METHOD(MethodPtr, name) jni::MakeNativePeerMethod<decltype(MethodPtr), (MethodPtr)>(name)

        // Register the peer
        jni::RegisterNativePeer<LocationIndicatorLayer>(
            env,
            javaClass,
            "nativePtr",
            jni::MakePeer<LocationIndicatorLayer, jni::String&>,
            "initialize",
            "finalize",
            METHOD(&LocationIndicatorLayer::getTopImage, "nativeGetTopImage"),
            METHOD(&LocationIndicatorLayer::getBearingImage, "nativeGetBearingImage"),
            METHOD(&LocationIndicatorLayer::getShadowImage, "nativeGetShadowImage"),
            METHOD(&LocationIndicatorLayer::getPerspectiveCompensation, "nativeGetPerspectiveCompensation"),
            METHOD(&LocationIndicatorLayer::getImageTiltDisplacement, "nativeGetImageTiltDisplacement"),
            METHOD(&LocationIndicatorLayer::getBearing, "nativeGetBearing"),
            METHOD(&LocationIndicatorLayer::getLocationTransition, "nativeGetLocationTransition"),
            METHOD(&LocationIndicatorLayer::setLocationTransition, "nativeSetLocationTransition"),
            METHOD(&LocationIndicatorLayer::getLocation, "nativeGetLocation"),
            METHOD(&LocationIndicatorLayer::getAccuracyRadiusTransition, "nativeGetAccuracyRadiusTransition"),
            METHOD(&LocationIndicatorLayer::setAccuracyRadiusTransition, "nativeSetAccuracyRadiusTransition"),
            METHOD(&LocationIndicatorLayer::getAccuracyRadius, "nativeGetAccuracyRadius"),
            METHOD(&LocationIndicatorLayer::getTopImageSizeTransition, "nativeGetTopImageSizeTransition"),
            METHOD(&LocationIndicatorLayer::setTopImageSizeTransition, "nativeSetTopImageSizeTransition"),
            METHOD(&LocationIndicatorLayer::getTopImageSize, "nativeGetTopImageSize"),
            METHOD(&LocationIndicatorLayer::getBearingImageSizeTransition, "nativeGetBearingImageSizeTransition"),
            METHOD(&LocationIndicatorLayer::setBearingImageSizeTransition, "nativeSetBearingImageSizeTransition"),
            METHOD(&LocationIndicatorLayer::getBearingImageSize, "nativeGetBearingImageSize"),
            METHOD(&LocationIndicatorLayer::getShadowImageSizeTransition, "nativeGetShadowImageSizeTransition"),
            METHOD(&LocationIndicatorLayer::setShadowImageSizeTransition, "nativeSetShadowImageSizeTransition"),
            METHOD(&LocationIndicatorLayer::getShadowImageSize, "nativeGetShadowImageSize"),
            METHOD(&LocationIndicatorLayer::getAccuracyRadiusColorTransition, "nativeGetAccuracyRadiusColorTransition"),
            METHOD(&LocationIndicatorLayer::setAccuracyRadiusColorTransition, "nativeSetAccuracyRadiusColorTransition"),
            METHOD(&LocationIndicatorLayer::getAccuracyRadiusColor, "nativeGetAccuracyRadiusColor"),
            METHOD(&LocationIndicatorLayer::getAccuracyRadiusBorderColorTransition, "nativeGetAccuracyRadiusBorderColorTransition"),
            METHOD(&LocationIndicatorLayer::setAccuracyRadiusBorderColorTransition, "nativeSetAccuracyRadiusBorderColorTransition"),
            METHOD(&LocationIndicatorLayer::getAccuracyRadiusBorderColor, "nativeGetAccuracyRadiusBorderColor"));
    }

} // namespace android
} // namespace mbgl
