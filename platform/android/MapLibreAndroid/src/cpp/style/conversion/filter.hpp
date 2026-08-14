#pragma once

#include <mbgl/style/filter.hpp>

#include <jni/jni.hpp>

namespace mln {
namespace android {
namespace conversion {

std::optional<mln::style::Filter> toFilter(jni::JNIEnv&, const jni::Array<jni::Object<>>&);

} // namespace conversion
} // namespace android
} // namespace mln
