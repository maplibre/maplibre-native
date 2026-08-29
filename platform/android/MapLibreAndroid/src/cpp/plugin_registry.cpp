#include "plugin_registry.hpp"

#include <mln/plugin/plugin_api.h>
#include <mln/plugin/plugin_registry.hpp>

#include <cstdint>

namespace mln {
namespace android {

jni::jboolean PluginRegistry::isRegistered(jni::JNIEnv& env,
                                           const jni::Class<PluginRegistry>&,
                                           const jni::String& pluginID) {
    return plugin::PluginRegistry::get().isRegistered(jni::Make<std::string>(env, pluginID));
}

jni::jint PluginRegistry::count(jni::JNIEnv&, const jni::Class<PluginRegistry>&) {
    return static_cast<jni::jint>(plugin::PluginRegistry::get().pluginIDs().size());
}

jni::Local<jni::String> PluginRegistry::idAt(jni::JNIEnv& env, const jni::Class<PluginRegistry>&, jni::jint index) {
    const auto ids = plugin::PluginRegistry::get().pluginIDs();
    if (index < 0 || static_cast<std::size_t>(index) >= ids.size()) {
        return jni::Make<jni::String>(env, std::string{});
    }
    return jni::Make<jni::String>(env, ids[static_cast<std::size_t>(index)]);
}

jni::jlong PluginRegistry::registrationFunctionAddress(jni::JNIEnv&, const jni::Class<PluginRegistry>&) {
    return static_cast<jni::jlong>(reinterpret_cast<std::uintptr_t>(&mln_plugin_register_v1));
}

void PluginRegistry::registerNative(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<PluginRegistry>::Singleton(env);
    jni::RegisterNatives(
        env,
        *javaClass,
        jni::MakeNativeMethod<decltype(&PluginRegistry::isRegistered), &PluginRegistry::isRegistered>(
            "nativeIsRegistered"),
        jni::MakeNativeMethod<decltype(&PluginRegistry::count), &PluginRegistry::count>("nativeCount"),
        jni::MakeNativeMethod<decltype(&PluginRegistry::idAt), &PluginRegistry::idAt>("nativeIdAt"),
        jni::MakeNativeMethod<decltype(&PluginRegistry::registrationFunctionAddress),
                              &PluginRegistry::registrationFunctionAddress>("nativeRegistrationFunctionAddress"));
}

} // namespace android
} // namespace mln
