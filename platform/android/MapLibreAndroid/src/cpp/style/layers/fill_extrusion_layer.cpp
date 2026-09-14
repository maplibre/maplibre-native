// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include "fill_extrusion_layer.hpp"

#include <string>

#include "../conversion/property_value.hpp"
#include "../conversion/transition_options.hpp"

#include <mln/style/layer_impl.hpp>

namespace mln {
namespace android {

inline mln::style::FillExtrusionLayer& toFillExtrusionLayer(mln::style::Layer& layer) {
    return static_cast<mln::style::FillExtrusionLayer&>(layer);
}

/**
 * Creates an owning peer object (for layers not attached to the map) from the JVM side
 */
FillExtrusionLayer::FillExtrusionLayer(jni::JNIEnv& env, jni::String& layerId, jni::String& sourceId)
    : Layer(std::make_unique<mln::style::FillExtrusionLayer>(jni::Make<std::string>(env, layerId),
                                                             jni::Make<std::string>(env, sourceId))) {}

/**
 * Creates a non-owning peer object (for layers currently attached to the map)
 */
FillExtrusionLayer::FillExtrusionLayer(mln::style::FillExtrusionLayer& coreLayer)
    : Layer(coreLayer) {}

/**
 * Creates an owning peer object (for layers not attached to the map)
 */
FillExtrusionLayer::FillExtrusionLayer(std::unique_ptr<mln::style::FillExtrusionLayer> coreLayer)
    : Layer(std::move(coreLayer)) {}

FillExtrusionLayer::~FillExtrusionLayer() = default;

// Property getters

jni::Local<jni::Object<>> FillExtrusionLayer::getFillExtrusionRoundedCornerDistance(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<>>>(
            env, style::FillExtrusionLayer::getDefaultFillExtrusionRoundedCornerDistance()));
    }
    return std::move(
        *convert<jni::Local<jni::Object<>>>(env, toFillExtrusionLayer(*layer).getFillExtrusionRoundedCornerDistance()));
}

jni::Local<jni::Object<>> FillExtrusionLayer::getFillExtrusionOpacity(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(
            *convert<jni::Local<jni::Object<>>>(env, style::FillExtrusionLayer::getDefaultFillExtrusionOpacity()));
    }
    return std::move(*convert<jni::Local<jni::Object<>>>(env, toFillExtrusionLayer(*layer).getFillExtrusionOpacity()));
}

jni::Local<jni::Object<TransitionOptions>> FillExtrusionLayer::getFillExtrusionOpacityTransition(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, mln::style::TransitionOptions()));
    }
    mln::style::TransitionOptions options = toFillExtrusionLayer(*layer).getFillExtrusionOpacityTransition();
    return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
}

void FillExtrusionLayer::setFillExtrusionOpacityTransition(jni::JNIEnv&, jlong duration, jlong delay) {
    auto layer = layerPtr.get();
    if (!layer) {
        return;
    }
    mln::style::TransitionOptions options;
    options.duration.emplace(mln::Milliseconds(duration));
    options.delay.emplace(mln::Milliseconds(delay));
    toFillExtrusionLayer(*layer).setFillExtrusionOpacityTransition(options);
}

jni::Local<jni::Object<>> FillExtrusionLayer::getFillExtrusionColor(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(
            *convert<jni::Local<jni::Object<>>>(env, style::FillExtrusionLayer::getDefaultFillExtrusionColor()));
    }
    return std::move(*convert<jni::Local<jni::Object<>>>(env, toFillExtrusionLayer(*layer).getFillExtrusionColor()));
}

jni::Local<jni::Object<TransitionOptions>> FillExtrusionLayer::getFillExtrusionColorTransition(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, mln::style::TransitionOptions()));
    }
    mln::style::TransitionOptions options = toFillExtrusionLayer(*layer).getFillExtrusionColorTransition();
    return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
}

void FillExtrusionLayer::setFillExtrusionColorTransition(jni::JNIEnv&, jlong duration, jlong delay) {
    auto layer = layerPtr.get();
    if (!layer) {
        return;
    }
    mln::style::TransitionOptions options;
    options.duration.emplace(mln::Milliseconds(duration));
    options.delay.emplace(mln::Milliseconds(delay));
    toFillExtrusionLayer(*layer).setFillExtrusionColorTransition(options);
}

jni::Local<jni::Object<>> FillExtrusionLayer::getFillExtrusionTranslate(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(
            *convert<jni::Local<jni::Object<>>>(env, style::FillExtrusionLayer::getDefaultFillExtrusionTranslate()));
    }
    return std::move(
        *convert<jni::Local<jni::Object<>>>(env, toFillExtrusionLayer(*layer).getFillExtrusionTranslate()));
}

jni::Local<jni::Object<TransitionOptions>> FillExtrusionLayer::getFillExtrusionTranslateTransition(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, mln::style::TransitionOptions()));
    }
    mln::style::TransitionOptions options = toFillExtrusionLayer(*layer).getFillExtrusionTranslateTransition();
    return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
}

void FillExtrusionLayer::setFillExtrusionTranslateTransition(jni::JNIEnv&, jlong duration, jlong delay) {
    auto layer = layerPtr.get();
    if (!layer) {
        return;
    }
    mln::style::TransitionOptions options;
    options.duration.emplace(mln::Milliseconds(duration));
    options.delay.emplace(mln::Milliseconds(delay));
    toFillExtrusionLayer(*layer).setFillExtrusionTranslateTransition(options);
}

jni::Local<jni::Object<>> FillExtrusionLayer::getFillExtrusionTranslateAnchor(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<>>>(
            env, style::FillExtrusionLayer::getDefaultFillExtrusionTranslateAnchor()));
    }
    return std::move(
        *convert<jni::Local<jni::Object<>>>(env, toFillExtrusionLayer(*layer).getFillExtrusionTranslateAnchor()));
}

jni::Local<jni::Object<>> FillExtrusionLayer::getFillExtrusionPattern(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(
            *convert<jni::Local<jni::Object<>>>(env, style::FillExtrusionLayer::getDefaultFillExtrusionPattern()));
    }
    return std::move(*convert<jni::Local<jni::Object<>>>(env, toFillExtrusionLayer(*layer).getFillExtrusionPattern()));
}

jni::Local<jni::Object<TransitionOptions>> FillExtrusionLayer::getFillExtrusionPatternTransition(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, mln::style::TransitionOptions()));
    }
    mln::style::TransitionOptions options = toFillExtrusionLayer(*layer).getFillExtrusionPatternTransition();
    return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
}

void FillExtrusionLayer::setFillExtrusionPatternTransition(jni::JNIEnv&, jlong duration, jlong delay) {
    auto layer = layerPtr.get();
    if (!layer) {
        return;
    }
    mln::style::TransitionOptions options;
    options.duration.emplace(mln::Milliseconds(duration));
    options.delay.emplace(mln::Milliseconds(delay));
    toFillExtrusionLayer(*layer).setFillExtrusionPatternTransition(options);
}

jni::Local<jni::Object<>> FillExtrusionLayer::getFillExtrusionHeight(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(
            *convert<jni::Local<jni::Object<>>>(env, style::FillExtrusionLayer::getDefaultFillExtrusionHeight()));
    }
    return std::move(*convert<jni::Local<jni::Object<>>>(env, toFillExtrusionLayer(*layer).getFillExtrusionHeight()));
}

jni::Local<jni::Object<TransitionOptions>> FillExtrusionLayer::getFillExtrusionHeightTransition(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, mln::style::TransitionOptions()));
    }
    mln::style::TransitionOptions options = toFillExtrusionLayer(*layer).getFillExtrusionHeightTransition();
    return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
}

void FillExtrusionLayer::setFillExtrusionHeightTransition(jni::JNIEnv&, jlong duration, jlong delay) {
    auto layer = layerPtr.get();
    if (!layer) {
        return;
    }
    mln::style::TransitionOptions options;
    options.duration.emplace(mln::Milliseconds(duration));
    options.delay.emplace(mln::Milliseconds(delay));
    toFillExtrusionLayer(*layer).setFillExtrusionHeightTransition(options);
}

jni::Local<jni::Object<>> FillExtrusionLayer::getFillExtrusionBase(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(
            *convert<jni::Local<jni::Object<>>>(env, style::FillExtrusionLayer::getDefaultFillExtrusionBase()));
    }
    return std::move(*convert<jni::Local<jni::Object<>>>(env, toFillExtrusionLayer(*layer).getFillExtrusionBase()));
}

jni::Local<jni::Object<TransitionOptions>> FillExtrusionLayer::getFillExtrusionBaseTransition(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, mln::style::TransitionOptions()));
    }
    mln::style::TransitionOptions options = toFillExtrusionLayer(*layer).getFillExtrusionBaseTransition();
    return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
}

void FillExtrusionLayer::setFillExtrusionBaseTransition(jni::JNIEnv&, jlong duration, jlong delay) {
    auto layer = layerPtr.get();
    if (!layer) {
        return;
    }
    mln::style::TransitionOptions options;
    options.duration.emplace(mln::Milliseconds(duration));
    options.delay.emplace(mln::Milliseconds(delay));
    toFillExtrusionLayer(*layer).setFillExtrusionBaseTransition(options);
}

jni::Local<jni::Object<>> FillExtrusionLayer::getFillExtrusionVerticalGradient(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<>>>(
            env, style::FillExtrusionLayer::getDefaultFillExtrusionVerticalGradient()));
    }
    return std::move(
        *convert<jni::Local<jni::Object<>>>(env, toFillExtrusionLayer(*layer).getFillExtrusionVerticalGradient()));
}

jni::Local<jni::Object<>> FillExtrusionLayer::getFillExtrusionShadowColor(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(
            *convert<jni::Local<jni::Object<>>>(env, style::FillExtrusionLayer::getDefaultFillExtrusionShadowColor()));
    }
    return std::move(
        *convert<jni::Local<jni::Object<>>>(env, toFillExtrusionLayer(*layer).getFillExtrusionShadowColor()));
}

jni::Local<jni::Object<TransitionOptions>> FillExtrusionLayer::getFillExtrusionShadowColorTransition(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, mln::style::TransitionOptions()));
    }
    mln::style::TransitionOptions options = toFillExtrusionLayer(*layer).getFillExtrusionShadowColorTransition();
    return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
}

void FillExtrusionLayer::setFillExtrusionShadowColorTransition(jni::JNIEnv&, jlong duration, jlong delay) {
    auto layer = layerPtr.get();
    if (!layer) {
        return;
    }
    mln::style::TransitionOptions options;
    options.duration.emplace(mln::Milliseconds(duration));
    options.delay.emplace(mln::Milliseconds(delay));
    toFillExtrusionLayer(*layer).setFillExtrusionShadowColorTransition(options);
}

jni::Local<jni::Object<>> FillExtrusionLayer::getFillExtrusionShadowOpacity(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<>>>(
            env, style::FillExtrusionLayer::getDefaultFillExtrusionShadowOpacity()));
    }
    return std::move(
        *convert<jni::Local<jni::Object<>>>(env, toFillExtrusionLayer(*layer).getFillExtrusionShadowOpacity()));
}

jni::Local<jni::Object<TransitionOptions>> FillExtrusionLayer::getFillExtrusionShadowOpacityTransition(
    jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, mln::style::TransitionOptions()));
    }
    mln::style::TransitionOptions options = toFillExtrusionLayer(*layer).getFillExtrusionShadowOpacityTransition();
    return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
}

void FillExtrusionLayer::setFillExtrusionShadowOpacityTransition(jni::JNIEnv&, jlong duration, jlong delay) {
    auto layer = layerPtr.get();
    if (!layer) {
        return;
    }
    mln::style::TransitionOptions options;
    options.duration.emplace(mln::Milliseconds(duration));
    options.delay.emplace(mln::Milliseconds(delay));
    toFillExtrusionLayer(*layer).setFillExtrusionShadowOpacityTransition(options);
}

jni::Local<jni::Object<>> FillExtrusionLayer::getFillExtrusionShadowLength(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(
            *convert<jni::Local<jni::Object<>>>(env, style::FillExtrusionLayer::getDefaultFillExtrusionShadowLength()));
    }
    return std::move(
        *convert<jni::Local<jni::Object<>>>(env, toFillExtrusionLayer(*layer).getFillExtrusionShadowLength()));
}

jni::Local<jni::Object<TransitionOptions>> FillExtrusionLayer::getFillExtrusionShadowLengthTransition(
    jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, mln::style::TransitionOptions()));
    }
    mln::style::TransitionOptions options = toFillExtrusionLayer(*layer).getFillExtrusionShadowLengthTransition();
    return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
}

void FillExtrusionLayer::setFillExtrusionShadowLengthTransition(jni::JNIEnv&, jlong duration, jlong delay) {
    auto layer = layerPtr.get();
    if (!layer) {
        return;
    }
    mln::style::TransitionOptions options;
    options.duration.emplace(mln::Milliseconds(duration));
    options.delay.emplace(mln::Milliseconds(delay));
    toFillExtrusionLayer(*layer).setFillExtrusionShadowLengthTransition(options);
}

jni::Local<jni::Object<>> FillExtrusionLayer::getFillExtrusionShadowAzimuth(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<>>>(
            env, style::FillExtrusionLayer::getDefaultFillExtrusionShadowAzimuth()));
    }
    return std::move(
        *convert<jni::Local<jni::Object<>>>(env, toFillExtrusionLayer(*layer).getFillExtrusionShadowAzimuth()));
}

jni::Local<jni::Object<TransitionOptions>> FillExtrusionLayer::getFillExtrusionShadowAzimuthTransition(
    jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, mln::style::TransitionOptions()));
    }
    mln::style::TransitionOptions options = toFillExtrusionLayer(*layer).getFillExtrusionShadowAzimuthTransition();
    return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
}

void FillExtrusionLayer::setFillExtrusionShadowAzimuthTransition(jni::JNIEnv&, jlong duration, jlong delay) {
    auto layer = layerPtr.get();
    if (!layer) {
        return;
    }
    mln::style::TransitionOptions options;
    options.duration.emplace(mln::Milliseconds(duration));
    options.delay.emplace(mln::Milliseconds(delay));
    toFillExtrusionLayer(*layer).setFillExtrusionShadowAzimuthTransition(options);
}

// FillExtrusionJavaLayerPeerFactory

FillExtrusionJavaLayerPeerFactory::~FillExtrusionJavaLayerPeerFactory() = default;

namespace {
jni::Local<jni::Object<Layer>> createJavaPeer(jni::JNIEnv& env, Layer* layer) {
    static auto& javaClass = jni::Class<FillExtrusionLayer>::Singleton(env);
    static auto constructor = javaClass.GetConstructor<jni::jlong>(env);
    return javaClass.New(env, constructor, reinterpret_cast<jni::jlong>(layer));
}
} // namespace

jni::Local<jni::Object<Layer>> FillExtrusionJavaLayerPeerFactory::createJavaLayerPeer(jni::JNIEnv& env,
                                                                                      mln::style::Layer& layer) {
    assert(layer.baseImpl->getTypeInfo() == getTypeInfo());
    return createJavaPeer(env, new FillExtrusionLayer(toFillExtrusionLayer(layer)));
}

jni::Local<jni::Object<Layer>> FillExtrusionJavaLayerPeerFactory::createJavaLayerPeer(
    jni::JNIEnv& env, std::unique_ptr<mln::style::Layer> layer) {
    assert(layer->baseImpl->getTypeInfo() == getTypeInfo());
    return createJavaPeer(env,
                          new FillExtrusionLayer(std::unique_ptr<mln::style::FillExtrusionLayer>(
                              static_cast<mln::style::FillExtrusionLayer*>(layer.release()))));
}

void FillExtrusionJavaLayerPeerFactory::registerNative(jni::JNIEnv& env) {
    // Lookup the class
    static auto& javaClass = jni::Class<FillExtrusionLayer>::Singleton(env);

#define METHOD(MethodPtr, name) jni::MakeNativePeerMethod<decltype(MethodPtr), (MethodPtr)>(name)

    // Register the peer
    jni::RegisterNativePeer<FillExtrusionLayer>(
        env,
        javaClass,
        "nativePtr",
        jni::MakePeer<FillExtrusionLayer, jni::String&, jni::String&>,
        "initialize",
        "finalize",
        METHOD(&FillExtrusionLayer::getFillExtrusionRoundedCornerDistance,
               "nativeGetFillExtrusionRoundedCornerDistance"),
        METHOD(&FillExtrusionLayer::getFillExtrusionOpacityTransition, "nativeGetFillExtrusionOpacityTransition"),
        METHOD(&FillExtrusionLayer::setFillExtrusionOpacityTransition, "nativeSetFillExtrusionOpacityTransition"),
        METHOD(&FillExtrusionLayer::getFillExtrusionOpacity, "nativeGetFillExtrusionOpacity"),
        METHOD(&FillExtrusionLayer::getFillExtrusionColorTransition, "nativeGetFillExtrusionColorTransition"),
        METHOD(&FillExtrusionLayer::setFillExtrusionColorTransition, "nativeSetFillExtrusionColorTransition"),
        METHOD(&FillExtrusionLayer::getFillExtrusionColor, "nativeGetFillExtrusionColor"),
        METHOD(&FillExtrusionLayer::getFillExtrusionTranslateTransition, "nativeGetFillExtrusionTranslateTransition"),
        METHOD(&FillExtrusionLayer::setFillExtrusionTranslateTransition, "nativeSetFillExtrusionTranslateTransition"),
        METHOD(&FillExtrusionLayer::getFillExtrusionTranslate, "nativeGetFillExtrusionTranslate"),
        METHOD(&FillExtrusionLayer::getFillExtrusionTranslateAnchor, "nativeGetFillExtrusionTranslateAnchor"),
        METHOD(&FillExtrusionLayer::getFillExtrusionPatternTransition, "nativeGetFillExtrusionPatternTransition"),
        METHOD(&FillExtrusionLayer::setFillExtrusionPatternTransition, "nativeSetFillExtrusionPatternTransition"),
        METHOD(&FillExtrusionLayer::getFillExtrusionPattern, "nativeGetFillExtrusionPattern"),
        METHOD(&FillExtrusionLayer::getFillExtrusionHeightTransition, "nativeGetFillExtrusionHeightTransition"),
        METHOD(&FillExtrusionLayer::setFillExtrusionHeightTransition, "nativeSetFillExtrusionHeightTransition"),
        METHOD(&FillExtrusionLayer::getFillExtrusionHeight, "nativeGetFillExtrusionHeight"),
        METHOD(&FillExtrusionLayer::getFillExtrusionBaseTransition, "nativeGetFillExtrusionBaseTransition"),
        METHOD(&FillExtrusionLayer::setFillExtrusionBaseTransition, "nativeSetFillExtrusionBaseTransition"),
        METHOD(&FillExtrusionLayer::getFillExtrusionBase, "nativeGetFillExtrusionBase"),
        METHOD(&FillExtrusionLayer::getFillExtrusionVerticalGradient, "nativeGetFillExtrusionVerticalGradient"),
        METHOD(&FillExtrusionLayer::getFillExtrusionShadowColorTransition,
               "nativeGetFillExtrusionShadowColorTransition"),
        METHOD(&FillExtrusionLayer::setFillExtrusionShadowColorTransition,
               "nativeSetFillExtrusionShadowColorTransition"),
        METHOD(&FillExtrusionLayer::getFillExtrusionShadowColor, "nativeGetFillExtrusionShadowColor"),
        METHOD(&FillExtrusionLayer::getFillExtrusionShadowOpacityTransition,
               "nativeGetFillExtrusionShadowOpacityTransition"),
        METHOD(&FillExtrusionLayer::setFillExtrusionShadowOpacityTransition,
               "nativeSetFillExtrusionShadowOpacityTransition"),
        METHOD(&FillExtrusionLayer::getFillExtrusionShadowOpacity, "nativeGetFillExtrusionShadowOpacity"),
        METHOD(&FillExtrusionLayer::getFillExtrusionShadowLengthTransition,
               "nativeGetFillExtrusionShadowLengthTransition"),
        METHOD(&FillExtrusionLayer::setFillExtrusionShadowLengthTransition,
               "nativeSetFillExtrusionShadowLengthTransition"),
        METHOD(&FillExtrusionLayer::getFillExtrusionShadowLength, "nativeGetFillExtrusionShadowLength"),
        METHOD(&FillExtrusionLayer::getFillExtrusionShadowAzimuthTransition,
               "nativeGetFillExtrusionShadowAzimuthTransition"),
        METHOD(&FillExtrusionLayer::setFillExtrusionShadowAzimuthTransition,
               "nativeSetFillExtrusionShadowAzimuthTransition"),
        METHOD(&FillExtrusionLayer::getFillExtrusionShadowAzimuth, "nativeGetFillExtrusionShadowAzimuth"));
}

} // namespace android
} // namespace mln
