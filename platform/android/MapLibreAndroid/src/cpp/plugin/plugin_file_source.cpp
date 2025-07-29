
#include "plugin_file_source.hpp"

using namespace mbgl::android;

void PluginFileSource::registerNative(jni::JNIEnv& env) {
    jni::Class<PluginFileSource>::Singleton(env);
}

jni::Global<jni::Object<PluginProtocolHandlerResource>> PluginFileSource::createJavaResource(jni::JNIEnv& env,
                                                                          const mbgl::Resource &resource) {
    jni::Global<jni::Object<PluginProtocolHandlerResource>> tempResult = jni::NewGlobal(env, PluginProtocolHandlerResource::Create(env));
    PluginProtocolHandlerResource::Update(env, tempResult, resource);
    return tempResult;
}

void PluginProtocolHandlerResource::registerNative(jni::JNIEnv& env) {
    jni::Class<PluginProtocolHandlerResource>::Singleton(env);
}

jni::Local<jni::Object<PluginProtocolHandlerResource>> PluginProtocolHandlerResource::Create(jni::JNIEnv& env) {
    auto& javaClass = jni::Class<PluginProtocolHandlerResource>::Singleton(env);
    auto constructor = javaClass.GetConstructor(env);
    return javaClass.New(env, constructor);
}

void PluginProtocolHandlerResource::Update(jni::JNIEnv& env,
                                                  jni::Object<PluginProtocolHandlerResource>& javaObject,
                                                  const mbgl::Resource &resource) {

    static auto &javaClass = jni::Class<PluginProtocolHandlerResource>::Singleton(env);

    static auto resourceKindField = javaClass.GetField<jni::jint>(env, "kind");
    javaObject.Set(env, resourceKindField, static_cast<jni::jint>(resource.kind));

    static auto resourceURLField = javaClass.GetField<jni::String>(env, "resourceURL");
    auto str = jni::Make<jni::String>(env, resource.url); // wrap the jstring
    javaObject.Set(env, resourceURLField, str);

}







/*
mbgl::PluginFileSource PluginFileSource::getFileSource(jni::JNIEnv&,
                                                              const jni::Object<PluginFileSource>&) {

    ResourceOptions resourceOptions;
    ClientOptions clientOptions;
    mbgl::PluginFileSource tempResult(resourceOptions, clientOptions);

    return  tempResult;

}
*/