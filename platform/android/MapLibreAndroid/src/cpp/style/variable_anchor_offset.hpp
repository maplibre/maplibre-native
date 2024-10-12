#pragma once

#include <mbgl/util/noncopyable.hpp>

#include <jni/jni.hpp>
#include <mbgl/util/variable_anchor_offset_collection.hpp>

namespace mbgl {
namespace android {

using SuperTag = jni::ObjectTag;
class VariableAnchorOffset : private mbgl::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/style/types/VariableAnchorOffset"; };

    static jni::Local<jni::Object<VariableAnchorOffset>> New(jni::JNIEnv &, const VariableAnchorOffsetCollection &value);

    static void registerNative(jni::JNIEnv &);
};

} // namespace android
} // namespace mbgl