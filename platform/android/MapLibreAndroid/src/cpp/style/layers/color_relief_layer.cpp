// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include "color_relief_layer.hpp"

#include <string>

#include "../conversion/property_value.hpp"
#include "../conversion/transition_options.hpp"

#include <mln/style/layer_impl.hpp>

namespace mln {
namespace android {

inline mln::style::ColorReliefLayer& toColorReliefLayer(mln::style::Layer& layer) {
    return static_cast<mln::style::ColorReliefLayer&>(layer);
}

/**
 * Creates an owning peer object (for layers not attached to the map) from the JVM side
 */
ColorReliefLayer::ColorReliefLayer(jni::JNIEnv& env, jni::String& layerId, jni::String& sourceId)
    : Layer(std::make_unique<mln::style::ColorReliefLayer>(jni::Make<std::string>(env, layerId),
                                                           jni::Make<std::string>(env, sourceId))) {}

/**
 * Creates a non-owning peer object (for layers currently attached to the map)
 */
ColorReliefLayer::ColorReliefLayer(mln::style::ColorReliefLayer& coreLayer)
    : Layer(coreLayer) {}

/**
 * Creates an owning peer object (for layers not attached to the map)
 */
ColorReliefLayer::ColorReliefLayer(std::unique_ptr<mln::style::ColorReliefLayer> coreLayer)
    : Layer(std::move(coreLayer)) {}

ColorReliefLayer::~ColorReliefLayer() = default;

// Property getters

jni::Local<jni::Object<>> ColorReliefLayer::getColorReliefOpacity(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(
            *convert<jni::Local<jni::Object<>>>(env, style::ColorReliefLayer::getDefaultColorReliefOpacity()));
    }
    return std::move(*convert<jni::Local<jni::Object<>>>(env, toColorReliefLayer(*layer).getColorReliefOpacity()));
}

jni::Local<jni::Object<TransitionOptions>> ColorReliefLayer::getColorReliefOpacityTransition(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, mln::style::TransitionOptions()));
    }
    mln::style::TransitionOptions options = toColorReliefLayer(*layer).getColorReliefOpacityTransition();
    return std::move(*convert<jni::Local<jni::Object<TransitionOptions>>>(env, options));
}

void ColorReliefLayer::setColorReliefOpacityTransition(jni::JNIEnv&, jlong duration, jlong delay) {
    auto layer = layerPtr.get();
    if (!layer) {
        return;
    }
    mln::style::TransitionOptions options;
    options.duration.emplace(mln::Milliseconds(duration));
    options.delay.emplace(mln::Milliseconds(delay));
    toColorReliefLayer(*layer).setColorReliefOpacityTransition(options);
}

jni::Local<jni::Object<>> ColorReliefLayer::getColorReliefColor(jni::JNIEnv& env) {
    using namespace mln::android::conversion;
    auto layer = layerPtr.get();
    if (!layer) {
        return std::move(
            *convert<jni::Local<jni::Object<>>>(env, style::ColorReliefLayer::getDefaultColorReliefColor()));
    }
    return std::move(*convert<jni::Local<jni::Object<>>>(env, toColorReliefLayer(*layer).getColorReliefColor()));
}

// ColorReliefJavaLayerPeerFactory

ColorReliefJavaLayerPeerFactory::~ColorReliefJavaLayerPeerFactory() = default;

namespace {
jni::Local<jni::Object<Layer>> createJavaPeer(jni::JNIEnv& env, Layer* layer) {
    static auto& javaClass = jni::Class<ColorReliefLayer>::Singleton(env);
    static auto constructor = javaClass.GetConstructor<jni::jlong>(env);
    return javaClass.New(env, constructor, reinterpret_cast<jni::jlong>(layer));
}
} // namespace

jni::Local<jni::Object<Layer>> ColorReliefJavaLayerPeerFactory::createJavaLayerPeer(jni::JNIEnv& env,
                                                                                    mln::style::Layer& layer) {
    assert(layer.baseImpl->getTypeInfo() == getTypeInfo());
    return createJavaPeer(env, new ColorReliefLayer(toColorReliefLayer(layer)));
}

jni::Local<jni::Object<Layer>> ColorReliefJavaLayerPeerFactory::createJavaLayerPeer(
    jni::JNIEnv& env, std::unique_ptr<mln::style::Layer> layer) {
    assert(layer->baseImpl->getTypeInfo() == getTypeInfo());
    return createJavaPeer(env,
                          new ColorReliefLayer(std::unique_ptr<mln::style::ColorReliefLayer>(
                              static_cast<mln::style::ColorReliefLayer*>(layer.release()))));
}

void ColorReliefJavaLayerPeerFactory::registerNative(jni::JNIEnv& env) {
    // Lookup the class
    static auto& javaClass = jni::Class<ColorReliefLayer>::Singleton(env);

#define METHOD(MethodPtr, name) jni::MakeNativePeerMethod<decltype(MethodPtr), (MethodPtr)>(name)

    // Register the peer
    jni::RegisterNativePeer<ColorReliefLayer>(
        env,
        javaClass,
        "nativePtr",
        jni::MakePeer<ColorReliefLayer, jni::String&, jni::String&>,
        "initialize",
        "finalize",
        METHOD(&ColorReliefLayer::getColorReliefOpacityTransition, "nativeGetColorReliefOpacityTransition"),
        METHOD(&ColorReliefLayer::setColorReliefOpacityTransition, "nativeSetColorReliefOpacityTransition"),
        METHOD(&ColorReliefLayer::getColorReliefOpacity, "nativeGetColorReliefOpacity"),
        METHOD(&ColorReliefLayer::getColorReliefColor, "nativeGetColorReliefColor"));
}

} // namespace android
} // namespace mln
