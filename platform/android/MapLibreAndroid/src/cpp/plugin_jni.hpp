#pragma once

#include <jni/jni.hpp>
#include <vector>

namespace mbgl {
namespace android {

using PluginJniRegistrationFn = void (*)(jni::JNIEnv&);

void addPluginJniRegistration(PluginJniRegistrationFn fn);
void registerAllPluginJniNatives(jni::JNIEnv& env);

// Optional convenience macro for plugins that want to register JNI native
// methods through MapLibre's JNI initialization flow.
//
// Usage:
//   void myPluginRegisterNatives(jni::JNIEnv& env) {
//     // Register JNI methods here
//   }
//   MLN_PLUGIN_REGISTER_JNI_NATIVES(myPluginRegisterNatives)
//
// This macro is NOT required. Plugins with their own binding systems
// (e.g., Gluecodium, manual JNI) can skip this entirely and handle
// JNI registration independently.
#define MLN_PLUGIN_REGISTER_JNI_NATIVES(registerFn)               \
    namespace {                                                   \
    struct AutoRegisterJni_##registerFn {                         \
        AutoRegisterJni_##registerFn() {                          \
            mbgl::android::addPluginJniRegistration(&registerFn); \
        }                                                         \
    } autoRegisterJni_##registerFn;                               \
    }

} // namespace android
} // namespace mbgl
