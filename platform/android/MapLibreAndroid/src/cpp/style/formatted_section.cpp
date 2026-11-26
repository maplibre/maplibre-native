#include "formatted_section.hpp"

namespace mln {
namespace android {

void FormattedSection::registerNative(jni::JNIEnv& env) {
    jni::Class<FormattedSection>::Singleton(env);
}

} // namespace android
} // namespace mln
