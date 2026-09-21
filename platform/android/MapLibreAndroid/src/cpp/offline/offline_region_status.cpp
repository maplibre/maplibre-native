#include "offline_region_status.hpp"

namespace mln {
namespace android {

jni::Local<jni::Object<OfflineRegionStatus>> OfflineRegionStatus::New(jni::JNIEnv& env,
                                                                      mln::OfflineRegionStatus status) {
    // Convert to jint
    jint downloadState;
    switch (status.downloadState) {
        case mln::OfflineRegionDownloadState::Inactive:
            downloadState = 0;
            break;
        case mln::OfflineRegionDownloadState::Active:
            downloadState = 1;
            break;
    }

    // Create java object
    static auto& javaClass = jni::Class<OfflineRegionStatus>::Singleton(env);
    static auto constructor = javaClass.GetConstructor<jint, jlong, jlong, jlong, jlong, jlong, jboolean>(env);
    return javaClass.New(env,
                         constructor,
                         downloadState,
                         jlong(status.completedResourceCount),
                         jlong(status.completedResourceSize),
                         jlong(status.completedTileCount),
                         jlong(status.completedTileSize),
                         jlong(status.requiredResourceCount),
                         jboolean(status.requiredResourceCountIsPrecise));
}

void OfflineRegionStatus::registerNative(jni::JNIEnv& env) {
    jni::Class<OfflineRegionStatus>::Singleton(env);
}

} // namespace android
} // namespace mln
