/*

 *
 */

#include "BoxingConversionUtils.h"

#include "FieldAccessMethods.h"

namespace glue_internal
{
namespace jni
{

// -------------------- JNI object field getters --------------------------------------------------

namespace
{
jfieldID get_field_id(JNIEnv* const env,
                      const JniReference<jobject>& object,
                      const char* field_name,
                      const char* field_type) noexcept
{
    return env->GetFieldID(get_object_class(env, object).get(), field_name, field_type);
}

template <typename Result>
Result get_field_value_impl(JNIEnv* const env,
                            Result(JNIEnv::*method_ptr)(jobject, jfieldID),
                            const JniReference<jobject>& object,
                            const char* field_name,
                            const char* field_type) noexcept
{
    const auto field_id = get_field_id(env, object, field_name, field_type);
    return (env->*method_ptr)(object.get(), field_id);
}

template <typename JavaType, typename CppType>
void set_field_value_impl(JNIEnv* const env,
                          void(JNIEnv::*method_ptr)(jobject, jfieldID, JavaType),
                          const JniReference<jobject>& object,
                          const char* field_name,
                          const char* field_type,
                          const CppType value) noexcept
{
    const auto field_id = get_field_id(env, object, field_name, field_type);
    return (env->*method_ptr)(object.get(), field_id, value);
}

template <typename HostSignedType, typename CppType>
void set_unsigned_optional_field_value(
                            JNIEnv* const env,
                            const JniReference<jobject>& object,
                            const char* field_name,
                            const char* field_type,
                            const std::optional<CppType>& value) noexcept
{
    const auto signed_value = value
        ? std::optional<HostSignedType>(static_cast<HostSignedType>(*value))
        : std::optional<HostSignedType>{};
    set_converted_object_field_value(env, object, field_name, field_type, signed_value);
}

template<typename Result>
Result get_byte_array_field_value(JNIEnv* const env,
                                  const JniReference<jobject>& object,
                                  const char* field_name)
{
    const auto field_value = make_local_ref(env,
        static_cast<jbyteArray>(
            get_field_value_impl(env, &JNIEnv::GetObjectField, object, field_name, "[B")));
    return convert_from_jni(env, field_value, TypeId<Result>{});
}

} // namespace

bool
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<bool> ) noexcept
{
    return get_field_value_impl(env, &JNIEnv::GetBooleanField, object, field_name, "Z");
}

int8_t
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<int8_t> ) noexcept
{
    return get_field_value_impl(env, &JNIEnv::GetByteField, object, field_name, "B");
}

int16_t
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<int16_t> ) noexcept
{
    return get_field_value_impl(env, &JNIEnv::GetShortField, object, field_name, "S");
}

int32_t
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<int32_t> ) noexcept
{
    return get_field_value_impl(env, &JNIEnv::GetIntField, object, field_name, "I");
}

int64_t
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<int64_t> ) noexcept
{
    return get_field_value_impl(env, &JNIEnv::GetLongField, object, field_name, "J");
}

uint8_t
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<uint8_t> ) noexcept
{
    return static_cast<uint8_t>(
        get_field_value_impl(env, &JNIEnv::GetShortField, object, field_name, "S"));
}

uint16_t
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<uint16_t> ) noexcept
{
    return static_cast<uint16_t>(
        get_field_value_impl(env, &JNIEnv::GetIntField, object, field_name, "I"));
}

uint32_t
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<uint32_t> ) noexcept
{
    return static_cast<uint32_t>(
        get_field_value_impl(env, &JNIEnv::GetLongField, object, field_name, "J"));
}

uint64_t
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<uint64_t> ) noexcept
{
    return static_cast<uint64_t>(
        get_field_value_impl(env, &JNIEnv::GetLongField, object, field_name, "J"));
}

float
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<float> ) noexcept
{
    return get_field_value_impl(env, &JNIEnv::GetFloatField, object, field_name, "F");
}

double
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<double> ) noexcept
{
    return get_field_value_impl(env, &JNIEnv::GetDoubleField, object, field_name, "D");
}

std::string
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::string> )
{
    return get_converted_object_field_value<std::string>(env, object, field_name, "Ljava/lang/String;");
}

std::optional<bool>
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::optional<bool>> ) noexcept
{
    return get_converted_object_field_value<std::optional<bool>>(
        env, object, field_name, "Ljava/lang/Boolean;");
}

std::optional<int8_t>
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::optional<int8_t>> ) noexcept
{
    return get_converted_object_field_value<std::optional<int8_t>>(
        env, object, field_name, "Ljava/lang/Byte;");
}

std::optional<int16_t>
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::optional<int16_t>> ) noexcept
{
    return get_converted_object_field_value<std::optional<int16_t>>(
        env, object, field_name, "Ljava/lang/Short;");
}

std::optional<int32_t>
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::optional<int32_t>> ) noexcept
{
    return get_converted_object_field_value<std::optional<int32_t>>(
        env, object, field_name, "Ljava/lang/Integer;");
}

std::optional<int64_t>
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::optional<int64_t>> ) noexcept
{
    return get_converted_object_field_value<std::optional<int64_t>>(
        env, object, field_name, "Ljava/lang/Long;");
}

std::optional<uint8_t>
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::optional<uint8_t>> ) noexcept
{
    return get_converted_object_field_value<std::optional<uint8_t>>(
        env, object, field_name, "Ljava/lang/Short;");
}

std::optional<uint16_t>
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::optional<uint16_t>> ) noexcept
{
    return get_converted_object_field_value<std::optional<uint16_t>>(
        env, object, field_name, "Ljava/lang/Integer;");
}

std::optional<uint32_t>
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::optional<uint32_t>> ) noexcept
{
    return get_converted_object_field_value<std::optional<uint32_t>>(
        env, object, field_name, "Ljava/lang/Long;");
}

std::optional<uint64_t>
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::optional<uint64_t>> ) noexcept
{
    return get_converted_object_field_value<std::optional<uint64_t>>(
        env, object, field_name, "Ljava/lang/Long;");
}

std::optional<float>
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::optional<float>> ) noexcept
{
    return get_converted_object_field_value<std::optional<float>>(
        env, object, field_name, "Ljava/lang/Float;");
}

std::optional<double>
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::optional<double>> ) noexcept
{
    return get_converted_object_field_value<std::optional<double>>(
        env, object, field_name, "Ljava/lang/Double;");
}

std::optional<std::string>
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::optional<std::string>> )
{
    return get_converted_object_field_value<std::optional<std::string>>(
        env, object, field_name, "Ljava/lang/String;");
}

std::shared_ptr<std::vector<uint8_t>>
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::shared_ptr<std::vector<uint8_t>>> )
{
    return get_byte_array_field_value<std::shared_ptr<std::vector<uint8_t>>>(env, object, field_name);
}

std::optional<std::shared_ptr<std::vector<uint8_t>>>
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::optional<std::shared_ptr<std::vector<uint8_t>>>> )
{
    return get_byte_array_field_value<std::optional<std::shared_ptr<std::vector<uint8_t>>>>(env, object, field_name);
}

::glue_internal::Locale
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<::glue_internal::Locale> ) noexcept
{   
    return get_converted_object_field_value<::glue_internal::Locale>(
        env, object, field_name, "Ljava/util/Locale;");
}

std::optional< ::glue_internal::Locale >
get_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 TypeId<std::optional< ::glue_internal::Locale >> ) noexcept
{
    return get_converted_object_field_value<std::optional< ::glue_internal::Locale >>(
        env, object, field_name, "Ljava/util/Locale;");
}

JniReference<jobject>
get_object_field_value( JNIEnv* const env,
                        const JniReference<jobject>& object,
                        const char* field_name,
                        const char* field_signature ) noexcept
{
    return make_local_ref(env,
        get_field_value_impl(env, &JNIEnv::GetObjectField, object, field_name, field_signature));
}

// -------------------- JNI object field setters --------------------------------------------------

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 bool value ) noexcept
{
    set_field_value_impl(env, &JNIEnv::SetBooleanField, object, field_name, "Z", value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 int8_t value ) noexcept
{
    set_field_value_impl(env, &JNIEnv::SetByteField, object, field_name, "B", value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 int16_t value ) noexcept
{
    set_field_value_impl(env, &JNIEnv::SetShortField, object, field_name, "S", value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 int32_t value ) noexcept
{
    set_field_value_impl(env, &JNIEnv::SetIntField, object, field_name, "I", value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 int64_t value ) noexcept
{
    set_field_value_impl(env, &JNIEnv::SetLongField, object, field_name, "J", value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 uint8_t value ) noexcept
{
    set_field_value_impl(env, &JNIEnv::SetShortField, object, field_name, "S", static_cast<int16_t>(value));
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 uint16_t value ) noexcept
{
    set_field_value_impl(env, &JNIEnv::SetIntField, object, field_name, "I", static_cast<int32_t>(value));
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 uint32_t value ) noexcept
{
    set_field_value_impl(env, &JNIEnv::SetLongField, object, field_name, "J", static_cast<int64_t>(value));
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 uint64_t value ) noexcept
{
    set_field_value_impl(env, &JNIEnv::SetLongField, object, field_name, "J", static_cast<int64_t>(value));
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 float value ) noexcept
{
    set_field_value_impl(env, &JNIEnv::SetFloatField, object, field_name, "F", value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 double value ) noexcept
{
    set_field_value_impl(env, &JNIEnv::SetDoubleField, object, field_name, "D", value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 const std::string& field_value ) noexcept
{
    set_converted_object_field_value(env, object, field_name, "Ljava/lang/String;", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 std::optional<bool> field_value ) noexcept
{
    set_converted_object_field_value(env, object, field_name, "Ljava/lang/Boolean;", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 std::optional<float> field_value ) noexcept
{
    set_converted_object_field_value(env, object, field_name, "Ljava/lang/Float;", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 std::optional<double> field_value ) noexcept
{
    set_converted_object_field_value(env, object, field_name, "Ljava/lang/Double;", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 std::optional<int8_t> field_value ) noexcept
{
    set_converted_object_field_value(env, object, field_name, "Ljava/lang/Byte;", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 std::optional<int16_t> field_value ) noexcept
{
    set_converted_object_field_value(env, object, field_name, "Ljava/lang/Short;", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 std::optional<int32_t> field_value ) noexcept
{
    set_converted_object_field_value(env, object, field_name, "Ljava/lang/Integer;", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 std::optional<int64_t> field_value ) noexcept
{
    set_converted_object_field_value(env, object, field_name, "Ljava/lang/Long;", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 std::optional<uint8_t> field_value ) noexcept
{
    set_unsigned_optional_field_value<int16_t>(env, object, field_name, "Ljava/lang/Short;", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 std::optional<uint16_t> field_value ) noexcept
{
    set_unsigned_optional_field_value<int32_t>(env, object, field_name, "Ljava/lang/Integer;", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 std::optional<uint32_t> field_value ) noexcept
{
    set_unsigned_optional_field_value<int64_t>(env, object, field_name, "Ljava/lang/Long;", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 std::optional<uint64_t> field_value ) noexcept
{
    set_unsigned_optional_field_value<int64_t>(env, object, field_name, "Ljava/lang/Long;", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 std::optional<std::string> field_value ) noexcept
{
    set_converted_object_field_value(env, object, field_name, "Ljava/lang/String;", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 const std::shared_ptr<std::vector<uint8_t>>& field_value ) noexcept
{
    set_converted_object_field_value(env, object, field_name, "[B", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 std::optional<std::shared_ptr<std::vector<uint8_t>>> field_value ) noexcept
{
    set_converted_object_field_value(env, object, field_name, "[B", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 const ::glue_internal::Locale& field_value ) noexcept
{
    set_converted_object_field_value(env, object, field_name, "Ljava/util/Locale;", field_value);
}

void
set_field_value( JNIEnv* const env,
                 const JniReference<jobject>& object,
                 const char* field_name,
                 std::optional< ::glue_internal::Locale > field_value ) noexcept
{
    set_converted_object_field_value(env, object, field_name, "Ljava/util/Locale;", field_value);
}

void
set_object_field_value( JNIEnv* const env,
                        const JniReference<jobject>& object,
                        const char* field_name,
                        const char* field_signature,
                        const JniReference<jobject>& field_value ) noexcept
{
    set_field_value_impl(env, &JNIEnv::SetObjectField, object, field_name, field_signature, field_value.get());   
}
}
}
