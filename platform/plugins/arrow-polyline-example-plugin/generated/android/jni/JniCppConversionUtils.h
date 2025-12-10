/*

 *
 */

#pragma once

#include <jni.h>

#include "JniCallJavaMethod.h"
#include "JniClassCache.h"
#include "JniReference.h"
#include "JniThrowNewException.h"
#include "JniTypeId.h"
#include "glue_internal/Locale.h"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace glue_internal
{
namespace jni
{

// ------------------- JNI to C++ conversion functions ---------------------------------------------

/**
 * Converts a JNI jstring to an std::string.
 */
JNIEXPORT std::string convert_from_jni( JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<std::string> );
JNIEXPORT std::string convert_from_jni( JNIEnv* const env, const JniReference<jstring>& jvalue, TypeId<std::string> );
JNIEXPORT std::optional<std::string> convert_from_jni(
    JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<std::optional<std::string>> );
JNIEXPORT std::optional<std::string> convert_from_jni(
    JNIEnv* const env, const JniReference<jstring>& jvalue, TypeId<std::optional<std::string>> );

/**
 * Converts a jbyteArray to a byte buffer
 */
JNIEXPORT std::shared_ptr<::std::vector<uint8_t>> convert_from_jni(
    JNIEnv* const env, const JniReference<jbyteArray>& jvalue, TypeId<std::shared_ptr<::std::vector<uint8_t>>> );
JNIEXPORT std::optional<std::shared_ptr<::std::vector<uint8_t>>> convert_from_jni(
    JNIEnv* const env, const JniReference<jbyteArray>& jvalue,
    TypeId<std::optional<std::shared_ptr<::std::vector<uint8_t>>>> );

/**
 * Converts a Java Date object to an std::chrono::time_point.
 */

JNIEXPORT jlong get_time_ms_epoch(JNIEnv* const env, const JniReference<jobject>& jvalue) noexcept;

template<class Clock, class Duration>
std::chrono::time_point<Clock, Duration>
convert_from_jni(JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<std::chrono::time_point<Clock, Duration>>) {
    if (!jvalue)
    {
        ::glue_internal::jni::throw_new_null_pointer_exception(env);
        return {};
    }

    const jlong time_ms_epoch = get_time_ms_epoch(env, jvalue);

    using namespace std::chrono;
    return time_point<Clock, Duration>(duration_cast<Duration>(milliseconds(time_ms_epoch)));
}

template<class Clock, class Duration>
std::optional<std::chrono::time_point<Clock, Duration>>
convert_from_jni(
    JNIEnv* const env,
    const JniReference<jobject>& jvalue,
    TypeId<std::optional<std::chrono::time_point<Clock, Duration>>>
) {
    return jvalue
        ? std::optional<std::chrono::time_point<Clock, Duration>>(
            convert_from_jni(env, jvalue, TypeId<std::chrono::time_point<Clock, Duration>>{}))
        : std::optional<std::chrono::time_point<Clock, Duration>>{};
}

/**
 * Converts a Java Duration object to an std::chrono::duration<>.
 */

JNIEXPORT std::intmax_t get_duration_from_java_duration(JNIEnv* const env,
                                              const JniReference<jobject>& jvalue,
                                              std::intmax_t dest_den,
                                              std::intmax_t dest_num);

template<class Rep, class Period>
std::chrono::duration<Rep, Period>
convert_from_jni(JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<std::chrono::duration<Rep, Period>>)
{
    if (!jvalue) {
        ::glue_internal::jni::throw_new_null_pointer_exception(env);
        return {};
    }

    const auto result_value = get_duration_from_java_duration(env, jvalue, Period::den, Period::num);

    return std::chrono::duration<Rep, Period>(result_value);
}

template<class Rep, class Period>
std::optional<std::chrono::duration<Rep, Period>>
convert_from_jni(
    JNIEnv* const env, const JniReference<jobject>& jvalue,
    TypeId<std::optional<std::chrono::duration<Rep, Period>>>)
{
    return jvalue
        ? std::optional<std::chrono::duration<Rep, Period>>(
            convert_from_jni( env, jvalue, TypeId<std::chrono::duration<Rep, Period>>{}))
        : std::optional<std::chrono::duration<Rep, Period>>{};
}

/**
 * Converts a Java Locale object to ::glue_internal::Locale.
 */
JNIEXPORT ::glue_internal::Locale convert_from_jni(
    JNIEnv* const env, const JniReference<jobject>& jvalue,
    TypeId<::glue_internal::Locale>);
JNIEXPORT std::optional<::glue_internal::Locale> convert_from_jni(
    JNIEnv* const env, const JniReference<jobject>& jvalue,
    TypeId<std::optional<::glue_internal::Locale>>);

// -------------------- C++ to JNI conversion functions --------------------------------------------

/**
 * Converts an std::string to a JNI jstring
 */
JNIEXPORT JniReference<jstring> convert_to_jni( JNIEnv* const env, const std::string& nvalue ) noexcept;
JNIEXPORT JniReference<jstring> convert_to_jni( JNIEnv* const env, const std::optional<std::string>& nvalue ) noexcept;

/**
 * Converts a byte buffer to a jbyteArray
 */
JNIEXPORT JniReference<jbyteArray> convert_to_jni(
    JNIEnv* const env, const std::shared_ptr< ::std::vector< uint8_t > >& nvalue ) noexcept;
JNIEXPORT JniReference<jbyteArray> convert_to_jni(
    JNIEnv* const env, const std::optional< std::shared_ptr< ::std::vector< uint8_t > > >& nvalue ) noexcept;

/**
 * Converts an std::chrono::time_point to a Java Date object.
 */

JNIEXPORT JniReference<jobject> create_date_new_object(JNIEnv* const env, const std::chrono::milliseconds& time_epoch);

template<class Clock, class Duration>
JniReference<jobject>
convert_to_jni(JNIEnv* const env, const std::chrono::time_point<Clock, Duration>& nvalue)
{
    return create_date_new_object(env, std::chrono::duration_cast<std::chrono::milliseconds>(nvalue.time_since_epoch()));
}

template<class Clock, class Duration>
JniReference<jobject>
convert_to_jni(JNIEnv* const env, const std::optional<std::chrono::time_point<Clock, Duration>>& nvalue)
{
    return nvalue ? convert_to_jni(env, *nvalue) : JniReference<jobject>{};
}

/**
 * Converts an std::chrono::duration<> to a Java Duration object.
 */

JNIEXPORT JniReference<jobject>
create_duration_new_object(JNIEnv* const env, std::intmax_t seconds, std::intmax_t nanos) noexcept;

template<class Rep, class Period>
JniReference<jobject>
convert_to_jni(JNIEnv* const env, const std::chrono::duration<Rep, Period>& nvalue) {
    using namespace std::chrono;
    const auto seconds_duration = duration_cast<seconds>(nvalue);
    const auto seconds_value = duration_cast<seconds>(nvalue).count();
    const auto nanos_adjustment = duration_cast<nanoseconds>(nvalue - seconds_duration).count();
    return create_duration_new_object(env, seconds_value, nanos_adjustment);
}

/**
 * Converts ::glue_internal::Locale to a Java Locale object.
 */
JNIEXPORT JniReference<jobject> convert_to_jni(
    JNIEnv* const env, const ::glue_internal::Locale& nvalue) noexcept;
JNIEXPORT JniReference<jobject> convert_to_jni(
    JNIEnv* const env, const std::optional<::glue_internal::Locale>& nvalue) noexcept;

template<class Rep, class Period>
JNIEXPORT JniReference<jobject> convert_to_jni(
    JNIEnv* const env, const std::optional<std::chrono::duration<Rep, Period>>& nvalue )
{
    return nvalue ? convert_to_jni(env, *nvalue) : JniReference<jobject>{};
}


// -------------------- std::optional<std::function<>> conversion functions -----------------------------

template<class R, class... Args>
std::optional<std::function<R(Args...)>>
convert_from_jni(JNIEnv* _jenv, const JniReference<jobject>& _jinput, TypeId<std::optional<std::function<R(Args...)>>>)
{
    return _jinput
        ? convert_from_jni(_jenv, _jinput, TypeId<std::function<R(Args...)>>{})
        : std::optional<std::function<R(Args...)>>{};
}

template<class R, class... Args>
JniReference<jobject>
convert_to_jni(JNIEnv* _jenv, const std::optional<std::function<R(Args...)>> _ninput)
{
    return _ninput ? convert_to_jni(_jenv, *_ninput) : JniReference<jobject>{};
}

// -------------------- createCppProxy() default implementation ------------------------------------

template<class T>
void createCppProxy(JNIEnv* /*env*/, const JniReference<jobject>& /*obj*/, ::std::shared_ptr<T>& /*result*/) {}

}
}
