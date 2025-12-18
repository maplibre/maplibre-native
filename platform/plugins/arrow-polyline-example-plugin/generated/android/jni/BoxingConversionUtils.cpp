/*

 *
 */

#include "BoxingConversionUtils.h"

#include "JniCallJavaMethod.h"
#include "JniCppConversionUtils.h"

namespace {

jint unbox_int_value(JNIEnv* const env, const ::glue_internal::jni::JniReference<jobject>& jvalue) noexcept {
    return ::glue_internal::jni::call_java_method<jint>(env, "java/lang/Integer", jvalue, "intValue", "()I");
}

jlong unbox_long_value(JNIEnv* const env, const ::glue_internal::jni::JniReference<jobject>& jvalue) noexcept {
    return ::glue_internal::jni::call_java_method<jlong>(env, "java/lang/Long", jvalue, "longValue", "()J");
}

template <typename T>
::glue_internal::jni::JniReference<jobject> box_value_in_object(JNIEnv* const env,
                                                                const char* const class_name,
                                                                const char* const signature,
                                                                const T param) noexcept {
    const auto java_class = ::glue_internal::jni::find_class(env, class_name);
    const auto constructor_id = env->GetMethodID(java_class.get(), "<init>", signature);
    return ::glue_internal::jni::new_object(env, java_class, constructor_id, param);
}

template <typename T>
::glue_internal::jni::JniReference<jobject> box_uint_in_object(JNIEnv* const env, const T param) noexcept {
    return box_value_in_object(env, "java/lang/Long", "(J)V", static_cast<int64_t>(param));
}

template <>
::glue_internal::jni::JniReference<jobject> box_uint_in_object(JNIEnv* const env, const uint8_t param) noexcept {
    return box_value_in_object(env, "java/lang/Short", "(S)V", static_cast<int16_t>(param));
}

template <>
::glue_internal::jni::JniReference<jobject> box_uint_in_object(JNIEnv* const env, const uint16_t param) noexcept {
    return box_value_in_object(env, "java/lang/Integer", "(I)V", static_cast<int32_t>(param));
}

template <typename T>
::glue_internal::jni::JniReference<jobject> convert_optional_to_jni(JNIEnv* const env,
                                                                    const std::optional<T>& nvalue) noexcept {
    return nvalue ? ::glue_internal::jni::convert_to_jni(env, *nvalue) : ::glue_internal::jni::JniReference<jobject>{};
}

template <typename Result, typename JResult>
std::optional<Result> convert_optional_from_jni(JNIEnv* const env,
                                                const ::glue_internal::jni::JniReference<jobject>& jvalue,
                                                const char* const method_name,
                                                const char* const method_signature) noexcept {
    if (!jvalue) {
        return {};
    }
    const auto unboxed_value = ::glue_internal::jni::call_java_method<JResult>(
        env, jvalue, method_name, method_signature);
    return std::optional<Result>(unboxed_value);
}
} // namespace

namespace glue_internal {
namespace jni {
JniReference<jobject> convert_to_jni(JNIEnv* const env, const bool nvalue) noexcept {
    return box_value_in_object<jboolean>(env, "java/lang/Boolean", "(Z)V", nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, const double nvalue) noexcept {
    return box_value_in_object<jdouble>(env, "java/lang/Double", "(D)V", nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, const float nvalue) noexcept {
    return box_value_in_object<jfloat>(env, "java/lang/Float", "(F)V", nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, const int8_t nvalue) noexcept {
    return box_value_in_object<jbyte>(env, "java/lang/Byte", "(B)V", nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, const int16_t nvalue) noexcept {
    return box_value_in_object<jshort>(env, "java/lang/Short", "(S)V", nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, const int32_t nvalue) noexcept {
    return box_value_in_object<jint>(env, "java/lang/Integer", "(I)V", nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, const int64_t nvalue) noexcept {
    return box_value_in_object<jlong>(env, "java/lang/Long", "(J)V", nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, const uint8_t nvalue) noexcept {
    return box_uint_in_object(env, nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, const uint16_t nvalue) noexcept {
    return box_uint_in_object(env, nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, const uint32_t nvalue) noexcept {
    return box_uint_in_object(env, nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, const uint64_t nvalue) noexcept {
    return box_uint_in_object(env, nvalue);
}

bool convert_from_jni(JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<bool>) noexcept {
    return call_java_method<jboolean>(env, "java/lang/Boolean", jvalue, "booleanValue", "()Z");
}

double convert_from_jni(JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<double>) noexcept {
    return call_java_method<jdouble>(env, "java/lang/Double", jvalue, "doubleValue", "()D");
}

float convert_from_jni(JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<float>) noexcept {
    return call_java_method<jfloat>(env, "java/lang/Float", jvalue, "floatValue", "()F");
}

int8_t convert_from_jni(JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<int8_t>) noexcept {
    return call_java_method<jbyte>(env, "java/lang/Byte", jvalue, "byteValue", "()B");
}

int16_t convert_from_jni(JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<int16_t>) noexcept {
    return call_java_method<jshort>(env, "java/lang/Short", jvalue, "shortValue", "()S");
}

int32_t convert_from_jni(JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<int32_t>) noexcept {
    return unbox_int_value(env, jvalue);
}

int64_t convert_from_jni(JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<int64_t>) noexcept {
    return unbox_long_value(env, jvalue);
}

uint8_t convert_from_jni(JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<uint8_t>) noexcept {
    return call_java_method<jshort>(env, "java/lang/Short", jvalue, "shortValue", "()S");
}

uint16_t convert_from_jni(JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<uint16_t>) noexcept {
    return unbox_int_value(env, jvalue);
}

uint32_t convert_from_jni(JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<uint32_t>) noexcept {
    return unbox_long_value(env, jvalue);
}

uint64_t convert_from_jni(JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<uint64_t>) noexcept {
    return unbox_long_value(env, jvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, std::optional<bool> nvalue) noexcept {
    return convert_optional_to_jni(env, nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, std::optional<float> nvalue) noexcept {
    return convert_optional_to_jni(env, nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, std::optional<double> nvalue) noexcept {
    return convert_optional_to_jni(env, nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, std::optional<int8_t> nvalue) noexcept {
    return convert_optional_to_jni(env, nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, std::optional<int16_t> nvalue) noexcept {
    return convert_optional_to_jni(env, nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, std::optional<int32_t> nvalue) noexcept {
    return convert_optional_to_jni(env, nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, std::optional<int64_t> nvalue) noexcept {
    return convert_optional_to_jni(env, nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, std::optional<uint8_t> nvalue) noexcept {
    return convert_optional_to_jni(env, nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, std::optional<uint16_t> nvalue) noexcept {
    return convert_optional_to_jni(env, nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, std::optional<uint32_t> nvalue) noexcept {
    return convert_optional_to_jni(env, nvalue);
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, std::optional<uint64_t> nvalue) noexcept {
    return convert_optional_to_jni(env, nvalue);
}

std::optional<bool> convert_from_jni(JNIEnv* const env,
                                     const JniReference<jobject>& jvalue,
                                     TypeId<std::optional<bool>>) noexcept {
    return convert_optional_from_jni<bool, jboolean>(env, jvalue, "booleanValue", "()Z");
}

std::optional<float> convert_from_jni(JNIEnv* const env,
                                      const JniReference<jobject>& jvalue,
                                      TypeId<std::optional<float>>) noexcept {
    return convert_optional_from_jni<float, jfloat>(env, jvalue, "floatValue", "()F");
}

std::optional<double> convert_from_jni(JNIEnv* const env,
                                       const JniReference<jobject>& jvalue,
                                       TypeId<std::optional<double>>) noexcept {
    return convert_optional_from_jni<double, jdouble>(env, jvalue, "doubleValue", "()D");
}

std::optional<int8_t> convert_from_jni(JNIEnv* const env,
                                       const JniReference<jobject>& jvalue,
                                       TypeId<std::optional<int8_t>>) noexcept {
    return convert_optional_from_jni<int8_t, int8_t>(env, jvalue, "byteValue", "()B");
}

std::optional<int16_t> convert_from_jni(JNIEnv* const env,
                                        const JniReference<jobject>& jvalue,
                                        TypeId<std::optional<int16_t>>) noexcept {
    return convert_optional_from_jni<int16_t, int16_t>(env, jvalue, "shortValue", "()S");
}

std::optional<int32_t> convert_from_jni(JNIEnv* const env,
                                        const JniReference<jobject>& jvalue,
                                        TypeId<std::optional<int32_t>>) noexcept {
    return convert_optional_from_jni<int32_t, int32_t>(env, jvalue, "intValue", "()I");
}

std::optional<int64_t> convert_from_jni(JNIEnv* const env,
                                        const JniReference<jobject>& jvalue,
                                        TypeId<std::optional<int64_t>>) noexcept {
    return convert_optional_from_jni<int64_t, int64_t>(env, jvalue, "longValue", "()J");
}

std::optional<uint8_t> convert_from_jni(JNIEnv* const env,
                                        const JniReference<jobject>& jvalue,
                                        TypeId<std::optional<uint8_t>>) noexcept {
    return convert_optional_from_jni<uint8_t, int16_t>(env, jvalue, "shortValue", "()S");
}

std::optional<uint16_t> convert_from_jni(JNIEnv* const env,
                                         const JniReference<jobject>& jvalue,
                                         TypeId<std::optional<uint16_t>>) noexcept {
    return convert_optional_from_jni<uint16_t, int32_t>(env, jvalue, "intValue", "()I");
}

std::optional<uint32_t> convert_from_jni(JNIEnv* const env,
                                         const JniReference<jobject>& jvalue,
                                         TypeId<std::optional<uint32_t>>) noexcept {
    return convert_optional_from_jni<uint32_t, int64_t>(env, jvalue, "longValue", "()J");
}

std::optional<uint64_t> convert_from_jni(JNIEnv* const env,
                                         const JniReference<jobject>& jvalue,
                                         TypeId<std::optional<uint64_t>>) noexcept {
    return convert_optional_from_jni<uint64_t, int64_t>(env, jvalue, "longValue", "()J");
}

} // namespace jni
} // namespace glue_internal
