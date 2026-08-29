#include "json_primitive.hpp"

namespace mln {
namespace android {
namespace gson {

void JsonPrimitive::registerNative(jni::JNIEnv &env) {
    jni::Class<JsonPrimitive>::Singleton(env);
}

} // namespace gson
} // namespace android
} // namespace mln
