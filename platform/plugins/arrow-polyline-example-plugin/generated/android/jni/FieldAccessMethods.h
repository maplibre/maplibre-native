/*

 *
 */

#pragma once

#include <jni.h>

#include "JniCppConversionUtils.h"
#include "JniReference.h"
#include "JniTypeId.h"
#include "glue_internal/Locale.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace glue_internal
{
namespace jni
{

// -------------------- JNI object field getters --------------------------------------------------

JNIEXPORT JniReference< jobject > get_object_field_value(
        JNIEnv* const env,
        const JniReference<jobject>& object,
        const char* field_name,
        const char* field_signature ) noexcept;

template <typename Result>
Result get_converted_object_field_value(
                            JNIEnv* const env,
                            const JniReference<jobject>& object,
                            const char* field_name,
                            const char* field_type) noexcept (noexcept(convert_from_jni(env, object, TypeId<Result>{})))
{
    return convert_from_jni(env, get_object_field_value(env, object, field_name, field_type), TypeId<Result>{});
}

JNIEXPORT bool get_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, TypeId<bool> ) noexcept;
JNIEXPORT int8_t get_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, TypeId<int8_t> ) noexcept;
JNIEXPORT int16_t get_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, TypeId<int16_t> ) noexcept;
JNIEXPORT int32_t get_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, TypeId<int32_t> ) noexcept;
JNIEXPORT int64_t get_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, TypeId<int64_t> ) noexcept;
JNIEXPORT uint8_t get_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, TypeId<uint8_t> ) noexcept;
JNIEXPORT uint16_t get_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, TypeId<uint16_t> ) noexcept;
JNIEXPORT uint32_t get_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, TypeId<uint32_t> ) noexcept;
JNIEXPORT uint64_t get_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, TypeId<uint64_t> ) noexcept;
JNIEXPORT float get_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, TypeId<float> ) noexcept;
JNIEXPORT double get_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, TypeId<double> ) noexcept;
JNIEXPORT ::std::string get_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, TypeId<::std::string> );

JNIEXPORT std::optional< bool > get_field_value( JNIEnv* const env,
                                           const JniReference< jobject >& object,
                                           const char* field_name,
                                           TypeId<std::optional< bool >> ) noexcept;
JNIEXPORT std::optional< int8_t > get_field_value( JNIEnv* const env,
                                             const JniReference< jobject >& object,
                                             const char* field_name,
                                             TypeId<std::optional< int8_t >> ) noexcept;
JNIEXPORT std::optional< int16_t > get_field_value( JNIEnv* const env,
                                              const JniReference< jobject >& object,
                                              const char* field_name,
                                              TypeId<std::optional< int16_t >> ) noexcept;
JNIEXPORT std::optional< int32_t > get_field_value( JNIEnv* const env,
                                              const JniReference< jobject >& object,
                                              const char* field_name,
                                              TypeId<std::optional< int32_t >> ) noexcept;
JNIEXPORT std::optional< int64_t > get_field_value( JNIEnv* const env,
                                              const JniReference< jobject >& object,
                                              const char* field_name,
                                              TypeId<std::optional< int64_t >> ) noexcept;
JNIEXPORT std::optional< uint8_t > get_field_value( JNIEnv* const env,
                                              const JniReference< jobject >& object,
                                              const char* field_name,
                                              TypeId<std::optional< uint8_t >> ) noexcept;
JNIEXPORT std::optional< uint16_t > get_field_value( JNIEnv* const env,
                                               const JniReference< jobject >& object,
                                               const char* field_name,
                                               TypeId<std::optional< uint16_t >> ) noexcept;
JNIEXPORT std::optional< uint32_t > get_field_value( JNIEnv* const env,
                                               const JniReference< jobject >& object,
                                               const char* field_name,
                                               TypeId<std::optional< uint32_t >> ) noexcept;
JNIEXPORT std::optional< uint64_t > get_field_value( JNIEnv* const env,
                                               const JniReference< jobject >& object,
                                               const char* field_name,
                                               TypeId<std::optional< uint64_t >> ) noexcept;
JNIEXPORT std::optional< float > get_field_value( JNIEnv* const env,
                                            const JniReference< jobject >& object,
                                            const char* field_name,
                                            TypeId<std::optional< float >> ) noexcept;
JNIEXPORT std::optional< double > get_field_value( JNIEnv* const env,
                                             const JniReference< jobject >& object,
                                             const char* field_name,
                                             TypeId<std::optional< double >> ) noexcept;
JNIEXPORT std::optional< ::std::string > get_field_value( JNIEnv* const env,
                                                    const JniReference< jobject >& object,
                                                    const char* field_name,
                                                    TypeId<std::optional< ::std::string >> );

JNIEXPORT ::std::shared_ptr<::std::vector<uint8_t>> get_field_value(
    JNIEnv* const env,
    const JniReference< jobject >& object,
    const char* field_name,
    TypeId<::std::shared_ptr<::std::vector<uint8_t>>> );
JNIEXPORT std::optional<::std::shared_ptr<::std::vector<uint8_t>>> get_field_value(
    JNIEnv* const env,
    const JniReference< jobject >& object,
    const char* field_name,
    TypeId<std::optional<::std::shared_ptr<::std::vector<uint8_t>>>> );

template<class Clock, class Duration>
std::chrono::time_point<Clock, Duration>
get_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, TypeId<std::chrono::time_point<Clock, Duration>>)
{
    return get_converted_object_field_value<std::chrono::time_point<Clock, Duration>>(
        env, object, field_name, "Ljava/util/Date;");
}

template<class Clock, class Duration>
std::optional<std::chrono::time_point<Clock, Duration>>
get_field_value(
    JNIEnv* const env, const JniReference<jobject >& object, const char* field_name,
    TypeId<std::optional<std::chrono::time_point<Clock, Duration>>>)
{    
    return get_converted_object_field_value<std::optional<std::chrono::time_point<Clock, Duration>>>(
        env, object, field_name, "Ljava/util/Date;");
}

JNIEXPORT ::glue_internal::Locale get_field_value(
    JNIEnv* const env,
    const JniReference< jobject >& object,
    const char* field_name,
    TypeId<::glue_internal::Locale> ) noexcept;
JNIEXPORT std::optional< ::glue_internal::Locale > get_field_value(
    JNIEnv* const env,
    const JniReference< jobject >& object,
    const char* field_name,
    TypeId<std::optional< ::glue_internal::Locale >> ) noexcept;

// -------------------- JNI object field setters --------------------------------------------------

JNIEXPORT void set_object_field_value(
                            JNIEnv* const env,
                            const JniReference<jobject>& object,
                            const char* field_name,
                            const char* fieldSignature,
                            const JniReference<jobject>& fieldValue ) noexcept;

template <typename CppType>
void set_converted_object_field_value(
                            JNIEnv* const env,
                            const JniReference<jobject>& object,
                            const char* field_name,
                            const char* field_type,
                            const CppType& value) noexcept (noexcept(convert_to_jni(env, value)))
{
    set_object_field_value(env, object, field_name, field_type, convert_to_jni(env, value));
}

JNIEXPORT void set_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, bool value ) noexcept;
JNIEXPORT void set_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, int8_t value ) noexcept;
JNIEXPORT void set_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, int16_t value ) noexcept;
JNIEXPORT void set_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, int32_t value ) noexcept;
JNIEXPORT void set_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, int64_t value ) noexcept;
JNIEXPORT void set_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, uint8_t value ) noexcept;
JNIEXPORT void set_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, uint16_t value ) noexcept;
JNIEXPORT void set_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, uint32_t value ) noexcept;
JNIEXPORT void set_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, uint64_t value ) noexcept;
JNIEXPORT void set_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, float value ) noexcept;
JNIEXPORT void set_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name, double value ) noexcept;
JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference<jobject>& object,
                      const char* field_name,
                      const std::string& fieldValue ) noexcept;

JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference< jobject >& object,
                      const char* field_name,
                      std::optional< bool > value ) noexcept;
JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference< jobject >& object,
                      const char* field_name,
                      std::optional< int8_t > value ) noexcept;
JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference< jobject >& object,
                      const char* field_name,
                      std::optional< int16_t > value ) noexcept;
JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference< jobject >& object,
                      const char* field_name,
                      std::optional< int32_t > value ) noexcept;
JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference< jobject >& object,
                      const char* field_name,
                      std::optional< int64_t > value ) noexcept;
JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference< jobject >& object,
                      const char* field_name,
                      std::optional< uint8_t > value ) noexcept;
JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference< jobject >& object,
                      const char* field_name,
                      std::optional< uint16_t > value ) noexcept;
JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference< jobject >& object,
                      const char* field_name,
                      std::optional< uint32_t > value ) noexcept;
JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference< jobject >& object,
                      const char* field_name,
                      std::optional< uint64_t > value ) noexcept;
JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference< jobject >& object,
                      const char* field_name,
                      std::optional< float > value ) noexcept;
JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference< jobject >& object,
                      const char* field_name,
                      std::optional< double > value ) noexcept;
JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference< jobject >& object,
                      const char* field_name,
                      std::optional< ::std::string > value ) noexcept;

JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference<jobject>& object,
                      const char* field_name,
                      const std::shared_ptr<::std::vector<uint8_t>>& field_value ) noexcept;
JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference<jobject>& object,
                      const char* field_name,
                      std::optional<std::shared_ptr<::std::vector<uint8_t>>> field_value ) noexcept;

template<class Clock, class Duration>
void
set_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name,
    const std::chrono::time_point<Clock, Duration>& field_value) noexcept
{
    set_converted_object_field_value(env, object, field_name, "Ljava/util/Date;", field_value);
}

template<class Clock, class Duration>
void
set_field_value(
    JNIEnv* const env, const JniReference<jobject>& object, const char* field_name,
    const std::optional<std::chrono::time_point<Clock, Duration>>& field_value) noexcept
{
    set_converted_object_field_value(env, object, field_name, "Ljava/util/Date;", field_value);
}

JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference<jobject>& object,
                      const char* field_name,
                      const ::glue_internal::Locale& fieldValue ) noexcept;
JNIEXPORT void set_field_value( JNIEnv* const env,
                      const JniReference<jobject>& object,
                      const char* field_name,
                      std::optional< ::glue_internal::Locale > fieldValue ) noexcept;

// -------------------- Templated JNI field accessors for Duration types --------------------------

template<class Rep, class Period>
JNIEXPORT ::std::chrono::duration<Rep, Period> get_field_value(
    JNIEnv* const env,
    const JniReference<jobject>& object,
    const char* field_name,
    TypeId<::std::chrono::duration<Rep, Period>> ) noexcept
{    
    return get_converted_object_field_value<::std::chrono::duration<Rep, Period>>(
        env, object, field_name, "Lcom/example/time/Duration;");
}

template<class Rep, class Period>
JNIEXPORT std::optional<::std::chrono::duration<Rep, Period>> get_field_value(
    JNIEnv* const env,
    const JniReference<jobject>& object,
    const char* field_name,
    TypeId<std::optional<::std::chrono::duration<Rep, Period>>> ) noexcept
{
    return get_converted_object_field_value<std::optional<::std::chrono::duration<Rep, Period>>>(
        env, object, field_name, "Lcom/example/time/Duration;");
}

template<class Rep, class Period>
JNIEXPORT void set_field_value(
    JNIEnv* const env,
    const JniReference<jobject>& object,
    const char* field_name,
    const ::std::chrono::duration<Rep, Period>& field_value ) noexcept
{
    set_converted_object_field_value(env, object, field_name, "Lcom/example/time/Duration;", field_value);
}

template<class Rep, class Period>
JNIEXPORT void set_field_value(
    JNIEnv* const env,
    const JniReference<jobject>& object,
    const char* field_name,
    std::optional<::std::chrono::duration<Rep, Period>> field_value ) noexcept
{
    set_converted_object_field_value(env, object, field_name, "Lcom/example/time/Duration;", field_value);
}

}
}
