#include <jni.h>
#include <dlfcn.h>
#include <mln/plugin/plugin_api.h>
#include "ngon_layer.hpp"

namespace {
void throwRegistrationError(JNIEnv* env, const char* message) {
    env->ThrowNew(env->FindClass("java/lang/IllegalStateException"), message);
}
} // namespace

extern "C" JNIEXPORT void JNICALL Java_org_maplibre_plugins_ngon_NgonLayer_registerNative(JNIEnv* env,
                                                                                          jclass,
                                                                                          jstring libraryName) {
    const char* name = env->GetStringUTFChars(libraryName, nullptr);
    if (!name) return;
    // Resolve from the renderer MapLibre already loaded, without loading a second registry.
    void* host = dlopen(name, RTLD_NOW | RTLD_NOLOAD);
    env->ReleaseStringUTFChars(libraryName, name);
    if (!host) {
        throwRegistrationError(env, "Initialize MapLibre with the selected renderer before registering plugins");
        return;
    }

    auto registerPlugin = reinterpret_cast<mln_plugin_register_function_v1>(dlsym(host, "mln_plugin_register_v1"));
    if (!registerPlugin) {
        dlclose(host);
        throwRegistrationError(env, "The SDK does not expose plugins. Build with -Pmaplibre.with_plugins=true");
        return;
    }

    char error[512] = {};
    const auto status = mln_ngon_layer_register(registerPlugin, error, sizeof(error));
    dlclose(host); // MapLibre keeps its original library reference for the process lifetime.
    if (status != MLN_PLUGIN_STATUS_OK && status != MLN_PLUGIN_STATUS_ALREADY_REGISTERED) {
        throwRegistrationError(env, error);
    }
}
