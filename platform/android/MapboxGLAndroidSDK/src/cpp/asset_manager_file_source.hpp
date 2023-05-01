#pragma once

#include <mbgl/storage/file_source.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/util/client_options.hpp>

#include "asset_manager.hpp"

#include <jni/jni.hpp>

namespace mbgl {

namespace util {
template <typename T>
class Thread;
} // namespace util

class AssetManagerFileSource : public FileSource {
public:
    AssetManagerFileSource(jni::JNIEnv&,
                           const jni::Object<android::AssetManager>&,
                           const ResourceOptions,
                           const ClientOptions);
    ~AssetManagerFileSource() override;

    std::unique_ptr<AsyncRequest> request(const Resource&, Callback) override;
    bool canRequest(const Resource&) const override;

    void setResourceOptions(ResourceOptions options) override;
    ResourceOptions getResourceOptions() override;

    void setClientOptions(ClientOptions options) override;
    ClientOptions getClientOptions() override;

private:
    class Impl;

    jni::Global<jni::Object<android::AssetManager>> assetManager;
    std::unique_ptr<util::Thread<Impl>> impl;
};

} // namespace mbgl
