#include "color.hpp"

namespace mln {
namespace android {
namespace conversion {

Result<mln::Color> Converter<mln::Color, int>::operator()(jni::JNIEnv&, const int& color) const {
    float r = (color >> 16) & 0xFF;
    float g = (color >> 8) & 0xFF;
    float b = (color) & 0xFF;
    float a = (color >> 24) & 0xFF;
    return {mln::Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f)};
}

} // namespace conversion
} // namespace android
} // namespace mln
