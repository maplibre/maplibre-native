#pragma once

#include "json_element.hpp"

#include <jni/jni.hpp>

namespace mln {
namespace android {
namespace gson {

class JsonArray {
public:
    using SuperTag = JsonElement;
    static constexpr auto Name() { return "com/google/gson/JsonArray"; };

    static jni::Local<jni::Object<JsonArray>> New(jni::JNIEnv&, const std::vector<mln::Value>&);
    static std::vector<mln::Value> convert(JNIEnv&, const jni::Object<JsonArray>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace gson
} // namespace android
} // namespace mln
