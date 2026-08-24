#pragma once

#include <mln/storage/offline.hpp>
#include <jni/jni.hpp>

namespace mln {
namespace android {

class OfflineRegionDefinition {
public:
    static constexpr auto Name() { return "org/maplibre/android/offline/OfflineRegionDefinition"; };

    static void registerNative(jni::JNIEnv&);

    static mln::OfflineRegionDefinition getDefinition(JNIEnv& env,
                                                      const jni::Object<OfflineRegionDefinition>& jDefinition);
};

class OfflineTilePyramidRegionDefinition {
public:
    using SuperTag = OfflineRegionDefinition;
    static constexpr auto Name() {
        return "org/maplibre/android/offline/"
               "OfflineTilePyramidRegionDefinition";
    };

    static jni::Local<jni::Object<OfflineRegionDefinition>> New(jni::JNIEnv&,
                                                                const mln::OfflineTilePyramidRegionDefinition&);

    static mln::OfflineTilePyramidRegionDefinition getDefinition(
        jni::JNIEnv&, const jni::Object<OfflineTilePyramidRegionDefinition>&);

    static void registerNative(jni::JNIEnv&);
};

class OfflineGeometryRegionDefinition {
public:
    using SuperTag = OfflineRegionDefinition;
    static constexpr auto Name() { return "org/maplibre/android/offline/OfflineGeometryRegionDefinition"; };

    static jni::Local<jni::Object<OfflineRegionDefinition>> New(jni::JNIEnv&,
                                                                const mln::OfflineGeometryRegionDefinition&);

    static mln::OfflineGeometryRegionDefinition getDefinition(jni::JNIEnv&,
                                                              const jni::Object<OfflineGeometryRegionDefinition>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
