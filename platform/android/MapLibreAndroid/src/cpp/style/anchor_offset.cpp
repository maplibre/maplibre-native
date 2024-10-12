#include "anchor_offset.hpp"

namespace mbgl {
namespace android {

void AnchorOffset::registerNative(jni::JNIEnv& env) {
    jni::Class<AnchorOffset>::Singleton(env);
}

} // namespace android
} // namespace mbgl