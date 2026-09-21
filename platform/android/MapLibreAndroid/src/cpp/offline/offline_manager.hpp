#pragma once

#include <mln/storage/offline.hpp>
#include <jni/jni.hpp>

#include "../file_source.hpp"
#include "offline_region.hpp"
#include "offline_region_definition.hpp"
#include "../java_types.hpp"

#include <memory>

namespace mln {

namespace android {

class OfflineManager {
public:
    class ListOfflineRegionsCallback {
    public:
        static constexpr auto Name() {
            return "org/maplibre/android/offline/"
                   "OfflineManager$ListOfflineRegionsCallback";
        }

        static void onError(jni::JNIEnv&,
                            const jni::Object<OfflineManager::ListOfflineRegionsCallback>&,
                            std::exception_ptr);

        static void onList(jni::JNIEnv&,
                           const jni::Object<FileSource>&,
                           const jni::Object<OfflineManager::ListOfflineRegionsCallback>&,
                           mln::OfflineRegions&);
    };
    class GetOfflineRegionCallback {
    public:
        static constexpr auto Name() {
            return "org/maplibre/android/offline/"
                   "OfflineManager$GetOfflineRegionCallback";
        }

        static void onError(jni::JNIEnv&,
                            const jni::Object<OfflineManager::GetOfflineRegionCallback>&,
                            std::exception_ptr);

        static void onRegionNotFound(jni::JNIEnv&,
                                     const jni::Object<FileSource>&,
                                     const jni::Object<OfflineManager::GetOfflineRegionCallback>&);

        static void onRegion(jni::JNIEnv&,
                             const jni::Object<FileSource>&,
                             const jni::Object<OfflineManager::GetOfflineRegionCallback>&,
                             mln::OfflineRegion&);
    };

    class CreateOfflineRegionCallback {
    public:
        static constexpr auto Name() {
            return "org/maplibre/android/offline/"
                   "OfflineManager$CreateOfflineRegionCallback";
        }

        static void onError(jni::JNIEnv&,
                            const jni::Object<OfflineManager::CreateOfflineRegionCallback>&,
                            std::exception_ptr);

        static void onCreate(jni::JNIEnv&,
                             const jni::Object<FileSource>&,
                             const jni::Object<OfflineManager::CreateOfflineRegionCallback>&,
                             mln::OfflineRegion&);
    };

    class MergeOfflineRegionsCallback {
    public:
        static constexpr auto Name() {
            return "org/maplibre/android/offline/"
                   "OfflineManager$MergeOfflineRegionsCallback";
        }

        static void onError(jni::JNIEnv&,
                            const jni::Object<OfflineManager::MergeOfflineRegionsCallback>&,
                            std::exception_ptr);

        static void onMerge(jni::JNIEnv&,
                            const jni::Object<FileSource>&,
                            const jni::Object<MergeOfflineRegionsCallback>&,
                            mln::OfflineRegions&);
    };

    struct FileSourceCallback {
        static constexpr auto Name() {
            return "org/maplibre/android/offline/"
                   "OfflineManager$FileSourceCallback";
        }

        static void onSuccess(jni::JNIEnv&, const jni::Object<OfflineManager::FileSourceCallback>&);

        static void onError(jni::JNIEnv&, const jni::Object<OfflineManager::FileSourceCallback>&, const jni::String&);
    };

    static constexpr auto Name() { return "org/maplibre/android/offline/OfflineManager"; };

    static void registerNative(jni::JNIEnv&);

    OfflineManager(jni::JNIEnv&, const jni::Object<FileSource>&);
    ~OfflineManager();

    void setOfflineMapboxTileCountLimit(jni::JNIEnv&, jni::jlong limit);

    void listOfflineRegions(jni::JNIEnv&,
                            const jni::Object<FileSource>&,
                            const jni::Object<ListOfflineRegionsCallback>& callback);

    void getOfflineRegion(jni::JNIEnv&,
                          const jni::Object<FileSource>&,
                          const int64_t regionID,
                          const jni::Object<GetOfflineRegionCallback>& callback);

    void createOfflineRegion(jni::JNIEnv&,
                             const jni::Object<FileSource>& jFileSource_,
                             const jni::Object<OfflineRegionDefinition>& definition,
                             const jni::Array<jni::jbyte>& metadata,
                             const jni::Object<OfflineManager::CreateOfflineRegionCallback>& callback);

    void mergeOfflineRegions(jni::JNIEnv&,
                             const jni::Object<FileSource>&,
                             const jni::String&,
                             const jni::Object<MergeOfflineRegionsCallback>&);

    void putResourceWithUrl(jni::JNIEnv&,
                            const jni::String& url,
                            const jni::Array<jni::jbyte>& data,
                            jlong modified,
                            jlong expires,
                            const jni::String& eTag,
                            jboolean mustRevalidate);

    void resetDatabase(jni::JNIEnv&, const jni::Object<FileSourceCallback>& callback_);

    void packDatabase(jni::JNIEnv&, const jni::Object<FileSourceCallback>& callback_);

    void invalidateAmbientCache(jni::JNIEnv&, const jni::Object<FileSourceCallback>& callback_);

    void clearAmbientCache(jni::JNIEnv&, const jni::Object<FileSourceCallback>& callback_);

    void setMaximumAmbientCacheSize(jni::JNIEnv&,
                                    const jni::jlong size,
                                    const jni::Object<FileSourceCallback>& callback_);

    void runPackDatabaseAutomatically(jni::JNIEnv&, jboolean autopack);

private:
    std::shared_ptr<mln::DatabaseFileSource> fileSource;
};

} // namespace android
} // namespace mln
