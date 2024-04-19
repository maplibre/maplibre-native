#include "maplibre.hpp"

namespace mbgl {
namespace android {

jni::Local<jni::Object<AssetManager>> MapLibre::getAssetManager(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<MapLibre>::Singleton(env);
    auto method = javaClass.GetStaticMethod<jni::Object<AssetManager>()>(env, "getAssetManager");
    return javaClass.Call(env, method);
}

jboolean MapLibre::hasInstance(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<MapLibre>::Singleton(env);
    auto method = javaClass.GetStaticMethod<jboolean()>(env, "hasInstance");
    return javaClass.Call(env, method);
}

void MapLibre::registerNative(jni::JNIEnv& env) {
    jni::Class<MapLibre>::Singleton(env);
}

} // namespace android
} // namespace mbgl
