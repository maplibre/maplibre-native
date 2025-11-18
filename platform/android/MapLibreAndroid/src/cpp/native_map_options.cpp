#include "native_map_options.hpp"

namespace mbgl {
namespace android {

void NativeMapOptions::registerNative(jni::JNIEnv &env) {
    jni::Class<NativeMapOptions>::Singleton(env);
}

NativeMapOptions::NativeMapOptions(jni::JNIEnv &_env, const jni::Object<NativeMapOptions> &_obj) {}

NativeMapOptions::~NativeMapOptions() {}

util::ActionJournalOptions NativeMapOptions::getActionJournalOptions(jni::JNIEnv &env,
                                                                     const jni::Object<NativeMapOptions> &obj) {
    auto &javaClass = jni::Class<NativeMapOptions>::Singleton(env);

    auto enabledField = javaClass.GetField<jni::jboolean>(env, "actionJournalEnabled");
    auto pathField = javaClass.GetField<jni::String>(env, "actionJournalPath");
    auto logFileSizeField = javaClass.GetField<jni::jlong>(env, "actionJournalLogFileSize");
    auto logFileCountField = javaClass.GetField<jni::jlong>(env, "actionJournalLogFileCount");
    auto renderingReportIntervalField = javaClass.GetField<jni::jint>(env, "actionJournalRenderingReportInterval");

    return util::ActionJournalOptions()
        .enable(obj.Get(env, enabledField))
        .withPath(jni::Make<std::string>(env, obj.Get(env, pathField)))
        .withLogFileSize(obj.Get(env, logFileSizeField))
        .withLogFileCount(obj.Get(env, logFileCountField))
        .withRenderingStatsReportInterval(obj.Get(env, renderingReportIntervalField));
}

float NativeMapOptions::pixelRatio(jni::JNIEnv &env, const jni::Object<NativeMapOptions> &obj) {
    auto &javaClass = jni::Class<NativeMapOptions>::Singleton(env);
    auto pixelRatioField = javaClass.GetField<float>(env, "pixelRatio");
    return obj.Get(env, pixelRatioField);
}

bool NativeMapOptions::crossSourceCollisions(jni::JNIEnv &env, const jni::Object<NativeMapOptions> &obj) {
    auto &javaClass = jni::Class<NativeMapOptions>::Singleton(env);
    auto crossSourceCollisionsField = javaClass.GetField<jni::jboolean>(env, "crossSourceCollisions");
    return obj.Get(env, crossSourceCollisionsField);
}

} // namespace android
} // namespace mbgl
