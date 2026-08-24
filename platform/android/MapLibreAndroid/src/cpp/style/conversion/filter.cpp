#include "filter.hpp"
#include "../android_conversion.hpp"

#include <mln/style/conversion.hpp>
#include <mln/style/conversion/filter.hpp>

namespace mln {
namespace android {
namespace conversion {

std::optional<mln::style::Filter> toFilter(jni::JNIEnv& env, const jni::Array<jni::Object<>>& jfilter) {
    std::optional<mln::style::Filter> filter;
    if (jfilter) {
        mln::style::conversion::Error error;
        auto converted = mln::style::conversion::convert<mln::style::Filter>(Value(env, jfilter), error);
        if (!converted) {
            mln::Log::Error(mln::Event::JNI, "Error converting filter: " + error.message);
        }
        filter = std::move(*converted);
    }
    return filter;
}

} // namespace conversion
} // namespace android
} // namespace mln
