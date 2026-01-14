/*

 *
 */

#include "FieldAccessMethods.h"
#include "JniNativeHandle.h"

namespace glue_internal {
namespace jni {

std::int64_t get_class_native_handle(JNIEnv* const env, const JniReference<jobject>& jobj) noexcept {
    return get_field_value(env, jobj, "nativeHandle", TypeId<int64_t>{});
}

std::int64_t get_class_native_handle(JNIEnv* const env, const jobject jobj) noexcept {
    return get_class_native_handle(env, jni::make_non_releasing_ref(jobj));
}

} // namespace jni
} // namespace glue_internal
