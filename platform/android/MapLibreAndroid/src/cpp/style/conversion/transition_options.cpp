#include "transition_options.hpp"

namespace mln {
namespace android {
namespace conversion {

Result<jni::Local<jni::Object<TransitionOptions>>>
Converter<jni::Local<jni::Object<TransitionOptions>>, mln::style::TransitionOptions>::operator()(
    jni::JNIEnv& env, const mln::style::TransitionOptions& value) const {
    return TransitionOptions::fromTransitionOptions(
        env,
        std::chrono::duration_cast<std::chrono::milliseconds>(value.duration.value_or(mln::Duration::zero())).count(),
        std::chrono::duration_cast<std::chrono::milliseconds>(value.delay.value_or(mln::Duration::zero())).count(),
        (jboolean)value.enablePlacementTransitions);
}

} // namespace conversion
} // namespace android
} // namespace mln
