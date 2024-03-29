#pragma once

#include <jni/jni.hpp>

#include "bitmap.hpp"

namespace mbgl {
namespace android {

class BitmapFactory {
public:
    static constexpr auto Name() { return "android/graphics/BitmapFactory"; };
    static void registerNative(jni::JNIEnv&);

    static jni::Local<jni::Object<Bitmap>> DecodeByteArray(jni::JNIEnv&,
                                                           jni::Array<jni::jbyte>& data,
                                                           jni::jint offset,
                                                           jni::jint length);
};

} // namespace android
} // namespace mbgl
