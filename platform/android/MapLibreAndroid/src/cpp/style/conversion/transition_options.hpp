#pragma once

#include "../../conversion/conversion.hpp"
#include "../transition_options.hpp"

#include <mln/style/transition_options.hpp>

#include <jni/jni.hpp>

namespace mln {
namespace android {
namespace conversion {

template <>
struct Converter<jni::Local<jni::Object<TransitionOptions>>, mln::style::TransitionOptions> {
    Result<jni::Local<jni::Object<TransitionOptions>>> operator()(jni::JNIEnv&,
                                                                  const mln::style::TransitionOptions&) const;
};

} // namespace conversion
} // namespace android
} // namespace mln
