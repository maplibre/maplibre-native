#pragma once

#include <mln/util/feature.hpp>

#include <jni/jni.hpp>

namespace mln {
namespace android {
namespace gson {

class JsonElement {
public:
    using SuperTag = jni::ObjectTag;
    static constexpr auto Name() { return "com/google/gson/JsonElement"; };

    static jni::Local<jni::Object<JsonElement>> New(jni::JNIEnv&, const mln::Value&);
    static mln::Value convert(JNIEnv&, const jni::Object<JsonElement>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace gson
} // namespace android
} // namespace mln
