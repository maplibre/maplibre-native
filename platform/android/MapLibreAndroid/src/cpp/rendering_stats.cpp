#include "rendering_stats.hpp"

namespace mbgl {
namespace android {

void RenderingStats::registerNative(jni::JNIEnv& env) {
    jni::Class<RenderingStats>::Singleton(env);
}

jni::Local<jni::Object<RenderingStats>> RenderingStats::Create(jni::JNIEnv& env) {
    auto& javaClass = jni::Class<RenderingStats>::Singleton(env);
    auto constructor = javaClass.GetConstructor(env);
    return javaClass.New(env, constructor);
}

void RenderingStats::Update(jni::JNIEnv& env,
                            jni::Object<RenderingStats>& javaObject,
                            const gfx::RenderingStats& stats) {
    static auto& javaClass = jni::Class<RenderingStats>::Singleton(env);

#define SetField(name, type)                                        \
    static auto name##Field = javaClass.GetField<type>(env, #name); \
    javaObject.Set(env, name##Field, static_cast<type>(stats.name));

    SetField(encodingTime, jni::jdouble);
    SetField(renderingTime, jni::jdouble);
    SetField(numFrames, jni::jint);
    SetField(numDrawCalls, jni::jint);
    SetField(totalDrawCalls, jni::jint);
    SetField(numCreatedTextures, jni::jint);
    SetField(numActiveTextures, jni::jint);
    SetField(numTextureBindings, jni::jint);
    SetField(numTextureUpdates, jni::jint);
    SetField(textureUpdateBytes, jni::jlong);
    SetField(totalBuffers, jni::jlong);
    SetField(totalBufferObjs, jni::jlong);
    SetField(bufferUpdates, jni::jlong);
    SetField(bufferObjUpdates, jni::jlong);
    SetField(bufferUpdateBytes, jni::jlong);
    SetField(numBuffers, jni::jint);
    SetField(numFrameBuffers, jni::jint);
    SetField(numIndexBuffers, jni::jint);
    SetField(indexUpdateBytes, jni::jlong);
    SetField(numVertexBuffers, jni::jint);
    SetField(vertexUpdateBytes, jni::jlong);
    SetField(numUniformBuffers, jni::jint);
    SetField(numUniformUpdates, jni::jint);
    SetField(uniformUpdateBytes, jni::jlong);
    SetField(memTextures, jni::jint);
    SetField(memBuffers, jni::jint);
    SetField(memIndexBuffers, jni::jint);
    SetField(memVertexBuffers, jni::jint);
    SetField(memUniformBuffers, jni::jint);
    SetField(stencilClears, jni::jint);
    SetField(stencilUpdates, jni::jint);

#undef SetField
}

} // namespace android
} // namespace mbgl
