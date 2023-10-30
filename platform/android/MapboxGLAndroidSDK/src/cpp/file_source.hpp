#pragma once

#include <mbgl/storage/database_file_source.hpp>
#include <mbgl/storage/online_file_source.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/storage/resource_transform.hpp>

#include "asset_manager.hpp"

#include <jni/jni.hpp>

#include "util/tile_server_options.hpp"

namespace mbgl {

template <typename T>
class Actor;

namespace android {

/**
 * Peer class for the Android ResourceLoader holder. Ensures that a single core
 * ResourceLoader of a ResourceLoader type is used.
 */
class ResourceLoader {
public:
    static constexpr auto Name() { return "org/maplibre/android/storage/FileSource"; };

    struct ResourceTransformCallback {
        static constexpr auto Name() {
            return "org/maplibre/android/storage/"
                   "FileSource$ResourceTransformCallback";
        }

        static std::string onURL(jni::JNIEnv&,
                                 const jni::Object<ResourceLoader::ResourceTransformCallback>&,
                                 int,
                                 std::string);
    };

    struct ResourcesCachePathChangeCallback {
        static constexpr auto Name() {
            return "org/maplibre/android/storage/"
                   "FileSource$ResourcesCachePathChangeCallback";
        }

        static void onSuccess(jni::JNIEnv&,
                              const jni::Object<ResourceLoader::ResourcesCachePathChangeCallback>&,
                              const jni::String&);

        static void onError(jni::JNIEnv&,
                            const jni::Object<ResourceLoader::ResourcesCachePathChangeCallback>&,
                            const jni::String&);
    };

    ResourceLoader(jni::JNIEnv&, const jni::String&, const jni::String&, const jni::Object<TileServerOptions>&);

    ~ResourceLoader();

    void setTileServerOptions(jni::JNIEnv& _env, const jni::Object<TileServerOptions>& _options);

    jni::Local<jni::String> getApiKey(jni::JNIEnv&);

    void setApiKey(jni::JNIEnv&, const jni::String&);

    jni::Local<jni::String> getAPIBaseUrl(jni::JNIEnv&);

    void setAPIBaseUrl(jni::JNIEnv&, const jni::String&);

    void setResourceTransform(jni::JNIEnv&, const jni::Object<ResourceLoader::ResourceTransformCallback>&);

    void setResourceCachePath(jni::JNIEnv&,
                              const jni::String&,
                              const jni::Object<ResourceLoader::ResourcesCachePathChangeCallback>&);

    void resume(jni::JNIEnv&);

    void pause(jni::JNIEnv&);

    jni::jboolean isResumed(jni::JNIEnv&);

    static ResourceLoader* getNativePeer(jni::JNIEnv&, const jni::Object<ResourceLoader>&);

    static mbgl::ResourceOptions getSharedResourceOptions(jni::JNIEnv&, const jni::Object<ResourceLoader>&);

    static mbgl::ClientOptions getSharedClientOptions(jni::JNIEnv&, const jni::Object<ResourceLoader>&);

    static void registerNative(jni::JNIEnv&);

private:
    const std::string DATABASE_FILE = "/mbgl-offline.db";
    std::optional<int> activationCounter;
    mbgl::ResourceOptions resourceOptions;
    mbgl::ClientOptions clientOptions;
    std::unique_ptr<Actor<ResourceTransform::TransformCallback>> resourceTransform;
    std::function<void()> pathChangeCallback;
    std::shared_ptr<mbgl::DatabaseFileSource> databaseSource;
    std::shared_ptr<mbgl::ResourceLoader> onlineSource;
    std::shared_ptr<mbgl::ResourceLoader> resourceLoader;
};

} // namespace android
} // namespace mbgl
