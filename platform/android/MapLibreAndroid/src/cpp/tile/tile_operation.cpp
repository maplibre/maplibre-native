#include "tile_operation.hpp"

namespace mbgl {
namespace android {

jni::Local<jni::Object<TileOperation>> TileOperation::Create(jni::JNIEnv& env, mbgl::TileOperation op) {
    static auto& _class = jni::Class<mbgl::android::TileOperation>::Singleton(env);
    switch (op) {
        case mbgl::TileOperation::RequestedFromNetwork:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "RequestedFromNetwork"));
        case mbgl::TileOperation::RequestedFromCache:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "RequestedFromCache"));
        case mbgl::TileOperation::LoadFromCache:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "LoadFromCache"));
        case mbgl::TileOperation::LoadFromNetwork:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "LoadFromNetwork"));
        case mbgl::TileOperation::StartParse:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "StartParse"));
        case mbgl::TileOperation::EndParse:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "EndParse"));
        case mbgl::TileOperation::Error:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "Error"));
        case mbgl::TileOperation::Cancelled:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "Cancelled"));
        case mbgl::TileOperation::NullOp:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "NullOp"));
    }
}

void TileOperation::registerNative(jni::JNIEnv& env) {
    jni::Class<TileOperation>::Singleton(env);
}

} // namespace android
} // namespace mbgl
