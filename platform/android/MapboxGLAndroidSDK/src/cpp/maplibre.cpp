#include "maplibre.hpp"

namespace mbgl {
namespace android {

jni::Local<jni::Object<AssetManager>> Maplibre::getAssetManager(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<Maplibre>::Singleton(env);
    auto method = javaClass.GetStaticMethod<jni::Object<AssetManager>()>(env, "getAssetManager");
    return javaClass.Call(env, method);
}

jboolean Maplibre::hasInstance(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<Maplibre>::Singleton(env);
    auto method = javaClass.GetStaticMethod<jboolean()>(env, "hasInstance");
    return javaClass.Call(env, method);
}

void Maplibre::registerNative(jni::JNIEnv& env) {
    jni::Class<Maplibre>::Singleton(env);
}

} // namespace android
} // namespace mbgl
