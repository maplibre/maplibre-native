#include "tile_operation.hpp"

namespace mln {
namespace android {

jni::Local<jni::Object<TileOperation>> TileOperation::Create(jni::JNIEnv& env, mln::TileOperation op) {
    static auto& _class = jni::Class<mln::android::TileOperation>::Singleton(env);
    switch (op) {
        case mln::TileOperation::RequestedFromNetwork:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "RequestedFromNetwork"));
        case mln::TileOperation::RequestedFromCache:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "RequestedFromCache"));
        case mln::TileOperation::LoadFromCache:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "LoadFromCache"));
        case mln::TileOperation::LoadFromNetwork:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "LoadFromNetwork"));
        case mln::TileOperation::StartParse:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "StartParse"));
        case mln::TileOperation::EndParse:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "EndParse"));
        case mln::TileOperation::Error:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "Error"));
        case mln::TileOperation::Cancelled:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "Cancelled"));
        case mln::TileOperation::NullOp:
            return _class.Get(env, _class.GetStaticField<jni::Object<TileOperation>>(env, "NullOp"));
    }
}

void TileOperation::registerNative(jni::JNIEnv& env) {
    jni::Class<TileOperation>::Singleton(env);
}

} // namespace android
} // namespace mln
