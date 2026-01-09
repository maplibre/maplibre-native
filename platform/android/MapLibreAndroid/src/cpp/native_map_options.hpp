#pragma once

#include <jni.h>
#include <jni/jni.hpp>
#include <mbgl/util/action_journal_options.hpp>

namespace mbgl {
namespace android {

class NativeMapOptions {
public:
    static constexpr auto Name() { return "org/maplibre/android/maps/NativeMapOptions"; };

    static void registerNative(jni::JNIEnv&);

    NativeMapOptions(jni::JNIEnv&, const jni::Object<NativeMapOptions>&);
    virtual ~NativeMapOptions();

    static util::ActionJournalOptions getActionJournalOptions(jni::JNIEnv&, const jni::Object<NativeMapOptions>&);
    static float pixelRatio(jni::JNIEnv&, const jni::Object<NativeMapOptions>&);
    static bool crossSourceCollisions(jni::JNIEnv&, const jni::Object<NativeMapOptions>&);
};

} // namespace android
} // namespace mbgl
