#pragma once

#include <mbgl/tile/tile_operation.hpp>

#include <jni/jni.hpp>

namespace mbgl {
namespace android {

class TileOperation {
public:
    static constexpr auto Name() { return "org/maplibre/android/tile/TileOperation"; };

    static jni::Local<jni::Object<TileOperation>> Create(jni::JNIEnv&, mbgl::TileOperation);

    static void registerNative(jni::JNIEnv& env);
};

} // namespace android
} // namespace mbgl
