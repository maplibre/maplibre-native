
#include "plugin_file_source.hpp"

using namespace mbgl::android;

// Putting these here (should probably move out to a more dedicated JNI place within
// the native repo at some point, but this means we don't have to update
// the jni.hpp dependency
namespace jni {

struct ByteBufferTag {
    static constexpr auto Name() { return "java/nio/ByteBuffer"; }
};

template <>
struct TagTraits<ByteBufferTag> {
    using SuperType = Object<ObjectTag>;
    using UntaggedType = jobject;
};

} // namespace jni

void PluginFileSource::registerNative(jni::JNIEnv& env) {
    jni::Class<PluginFileSource>::Singleton(env);
}

jni::Global<jni::Object<PluginProtocolHandlerResource>> PluginFileSource::createJavaResource(
    jni::JNIEnv& env, const mbgl::Resource& resource) {
    jni::Global<jni::Object<PluginProtocolHandlerResource>> tempResult = jni::NewGlobal(
        env, PluginProtocolHandlerResource::Create(env));
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
                                           const mbgl::Resource& resource) {
    static auto& javaClass = jni::Class<PluginProtocolHandlerResource>::Singleton(env);

    static auto resourceKindField = javaClass.GetField<jni::jint>(env, "kind");
    javaObject.Set(env, resourceKindField, static_cast<jni::jint>(resource.kind));

    static auto resourceURLField = javaClass.GetField<jni::String>(env, "resourceURL");
    auto str = jni::Make<jni::String>(env, resource.url); // wrap the jstring
    javaObject.Set(env, resourceURLField, str);
}

void PluginProtocolHandlerResponse::registerNative(jni::JNIEnv& env) {
    jni::Class<PluginProtocolHandlerResponse>::Singleton(env);
}

jni::Local<jni::Object<PluginProtocolHandlerResponse>> PluginProtocolHandlerResponse::Create(jni::JNIEnv& env) {
    auto& javaClass = jni::Class<PluginProtocolHandlerResponse>::Singleton(env);
    auto constructor = javaClass.GetConstructor(env);
    return javaClass.New(env, constructor);
}

void PluginProtocolHandlerResponse::Update(jni::JNIEnv& env,
                                           [[maybe_unused]] jni::Object<PluginProtocolHandlerResponse>& javaObject,
                                           [[maybe_unused]] mbgl::Response& response) {
    static auto& javaClass = jni::Class<PluginProtocolHandlerResponse>::Singleton(env);
    static auto dataField = javaClass.GetField<jni::Object<jni::ByteBufferTag>>(env, "data");
    auto objectValue = javaObject.Get(env, dataField);
    auto objectRef = jobject(objectValue.get());
    void* bufPtr = env.GetDirectBufferAddress(objectRef);
    jsize length = env.GetDirectBufferCapacity(objectRef);
    response.data = std::make_shared<std::string>((const char*)bufPtr, length);
}
