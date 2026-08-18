#include "asset_manager_file_source.hpp"

#include <mln/platform/settings.hpp>
#include <mln/storage/file_source_request.hpp>
#include <mln/storage/resource.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/storage/response.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/thread.hpp>
#include <mln/util/url.hpp>
#include <mln/util/util.hpp>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <optional>
#include <string_view>

namespace mln {
namespace {

constexpr std::string_view androidAssetPathPrefix = "/android_asset/";

void stripLeadingSlashes(std::string& path) {
    path.erase(0, path.find_first_not_of('/'));
}

std::optional<std::string> androidAssetPathFromUrl(const std::string& url) {
    if (url.starts_with(util::ASSET_PROTOCOL)) {
        std::string path = util::percentDecode(url.substr(std::char_traits<char>::length(util::ASSET_PROTOCOL)));
        stripLeadingSlashes(path);
        return path;
    }

    if (url.starts_with(util::FILE_PROTOCOL)) {
        std::string path = util::percentDecode(url.substr(std::char_traits<char>::length(util::FILE_PROTOCOL)));
        if (!path.starts_with(androidAssetPathPrefix)) {
            return std::nullopt;
        }
        path.erase(0, androidAssetPathPrefix.size());
        stripLeadingSlashes(path);
        return path;
    }

    return std::nullopt;
}

} // namespace

class AssetManagerFileSource::Impl {
public:
    Impl(ActorRef<Impl>,
         AAssetManager* assetManager_,
         const ResourceOptions resourceOptions_,
         const ClientOptions clientOptions_)
        : resourceOptions(resourceOptions_.clone()),
          clientOptions(clientOptions_.clone()),
          assetManager(assetManager_) {}

    void request(const std::string& url, ActorRef<FileSourceRequest> req) {
        Response response;

        // Note: AssetManager already prepends "assets" to the filename.
        const auto path = androidAssetPathFromUrl(url);
        AAsset* asset = path ? AAssetManager_open(assetManager, path->c_str(), AASSET_MODE_BUFFER) : nullptr;
        if (asset) {
            response.data = std::make_shared<std::string>(reinterpret_cast<const char*>(AAsset_getBuffer(asset)),
                                                          AAsset_getLength64(asset));
            AAsset_close(asset);
        } else {
            response.error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound,
                                                               "Could not read asset");
        }

        req.invoke(&FileSourceRequest::setResponse, response);
    }

    void setResourceOptions(ResourceOptions options) { resourceOptions = options; }

    ResourceOptions getResourceOptions() { return resourceOptions.clone(); }

    void setClientOptions(ClientOptions options) { clientOptions = options; }

    ClientOptions getClientOptions() { return clientOptions.clone(); }

private:
    AAssetManager* assetManager;
    ResourceOptions resourceOptions;
    ClientOptions clientOptions;
};

AssetManagerFileSource::AssetManagerFileSource(jni::JNIEnv& env,
                                               const jni::Object<android::AssetManager>& assetManager_,
                                               const ResourceOptions resourceOptions,
                                               const ClientOptions clientOptions)
    : assetManager(jni::NewGlobal<jni::EnvAttachingDeleter>(env, assetManager_)),
      impl(std::make_unique<util::Thread<Impl>>(
          util::makeThreadPrioritySetter(platform::EXPERIMENTAL_THREAD_PRIORITY_FILE),
          "AssetManagerFileSource",
          AAssetManager_fromJava(&env, jni::Unwrap(assetManager.get())),
          resourceOptions.clone(),
          clientOptions.clone())) {}

AssetManagerFileSource::~AssetManagerFileSource() = default;

std::unique_ptr<AsyncRequest> AssetManagerFileSource::request(const Resource& resource, Callback callback) {
    auto req = std::make_unique<FileSourceRequest>(std::move(callback));

    impl->actor().invoke(&Impl::request, resource.url, req->actor());

    return std::move(req);
}

bool AssetManagerFileSource::canRequest(const Resource& resource) const {
    return androidAssetPathFromUrl(resource.url).has_value();
}

void AssetManagerFileSource::setResourceOptions(ResourceOptions options) {
    impl->actor().invoke(&Impl::setResourceOptions, options.clone());
}

ResourceOptions AssetManagerFileSource::getResourceOptions() {
    return impl->actor().ask(&Impl::getResourceOptions).get();
}

void AssetManagerFileSource::setClientOptions(ClientOptions options) {
    impl->actor().invoke(&Impl::setClientOptions, options.clone());
}

ClientOptions AssetManagerFileSource::getClientOptions() {
    return impl->actor().ask(&Impl::getClientOptions).get();
}

} // namespace mln
