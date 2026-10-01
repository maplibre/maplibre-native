#pragma once

#include "conversion.hpp"

#include <mln/util/color.hpp>

namespace mln {
namespace android {
namespace conversion {

template <>
struct Converter<mln::Color, int> {
    Result<mln::Color> operator()(jni::JNIEnv&, const int& color) const;
};

} // namespace conversion
} // namespace android
} // namespace mln
