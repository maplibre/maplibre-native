#pragma once

#include <mbgl/style/filter.hpp>

#include <jni/jni.hpp>

namespace mbgl {
namespace android {
namespace conversion {

std::optional<mbgl::style::Filter> toFilter(jni::JNIEnv&, const jni::Array<jni::Object<>>&);

} // namespace conversion
} // namespace android
} // namespace mbgl