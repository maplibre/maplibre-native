#include <jni.h>
#include <string>
#include <mbgl/plugin/plugin_manager.hpp>
#include "PluginLayerExampleAndroidRenderingCPP.h"


extern "C" JNIEXPORT jstring JNICALL
Java_org_maplibre_maplibrepluginsexamplelibrary_NativeLib_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_org_maplibre_maplibrepluginsexamplelibrary_NativeLib_registerPlugins(
        JNIEnv* env,
        jobject /* this */) {
    // Register plugins here

    // Register the android plugin here
    auto cppAndroidMapLayerType = std::make_shared<app::AndroidPluginLayerType>();
    auto *pm = mbgl::plugin::PluginManager::get();
    pm->addMapLayerType(cppAndroidMapLayerType);


}