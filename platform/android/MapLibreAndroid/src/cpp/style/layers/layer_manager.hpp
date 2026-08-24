#pragma once

#include <mln/layermanager/layer_manager.hpp>
#include <mln/map/map.hpp>
#include <mln/style/layer.hpp>

#include "layer.hpp"

#include <jni/jni.hpp>

#include <memory>
#include <vector>

namespace mln {
namespace android {

/**
 * @brief A singleton class forwarding calls to the corresponding  \c
 * JavaLayerPeerFactory instance.
 */
class LayerManagerAndroid final : public mln::LayerManager {
public:
    ~LayerManagerAndroid() final;
    static LayerManagerAndroid* get() noexcept;

    jni::Local<jni::Object<Layer>> createJavaLayerPeer(jni::JNIEnv&, mln::style::Layer&);
    jni::Local<jni::Object<Layer>> createJavaLayerPeer(jni::JNIEnv& env, std::unique_ptr<mln::style::Layer>);

    void registerNative(jni::JNIEnv&);

private:
    LayerManagerAndroid();
    /**
     * @brief Enables a layer type for both JSON style and runtime API.
     */
    void addLayerType(std::unique_ptr<JavaLayerPeerFactory>);
    /**
     * @brief Enables a layer type for JSON style only.
     *
     * We might not want to expose runtime API for some layer types
     * in order to save binary size - JNI glue code for these layer types
     * won't be added to the binary.
     */
    void addLayerTypeCoreOnly(std::unique_ptr<mln::LayerFactory>);

    void registerCoreFactory(mln::LayerFactory*);
    JavaLayerPeerFactory* getPeerFactory(const mln::style::LayerTypeInfo*);
    // mln::LayerManager overrides.
    LayerFactory* getFactory(const std::string& type) noexcept final;
    LayerFactory* getFactory(const mln::style::LayerTypeInfo* info) noexcept final;

    std::vector<std::unique_ptr<JavaLayerPeerFactory>> peerFactories;
    std::vector<std::unique_ptr<mln::LayerFactory>> coreFactories;
    std::map<std::string, mln::LayerFactory*> typeToFactory;
};

} // namespace android
} // namespace mln
