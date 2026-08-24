#pragma once

#include <mln/tile/tile_operation.hpp>

#include <jni/jni.hpp>

namespace mln {
namespace android {

class TileOperation {
public:
    static constexpr auto Name() { return "org/maplibre/android/tile/TileOperation"; };

    static jni::Local<jni::Object<TileOperation>> Create(jni::JNIEnv&, mln::TileOperation);

    static void registerNative(jni::JNIEnv& env);
};

} // namespace android
} // namespace mln
