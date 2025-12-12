/*

 *
 */

#include "JniCppConversionUtils.h"
#include <optional>

namespace glue_internal {
namespace jni {

// ------------------- JNI to C++ conversion functions ---------------------------------------------

std::string convert_string_from_jni(JNIEnv* const env, jstring java_string) {
    const char* jbuffer = env->GetStringUTFChars(java_string, nullptr);
    auto nresult = std::string(jbuffer);
    env->ReleaseStringUTFChars(java_string, jbuffer);
    return nresult;
}

std::string convert_from_jni(JNIEnv* const env, const JniReference<jobject>& jvalue, TypeId<std::string>) {
    if (!jvalue) {
        ::glue_internal::jni::throw_new_null_pointer_exception(env);
        return {};
    }

    return convert_string_from_jni(env, static_cast<jstring>(jvalue.get()));
}

std::string convert_from_jni(JNIEnv* const env, const JniReference<jstring>& jvalue, TypeId<std::string>) {
    if (!jvalue) {
        ::glue_internal::jni::throw_new_null_pointer_exception(env);
        return {};
    }

    return convert_string_from_jni(env, jvalue.get());
}

std::optional<std::string> convert_from_jni(JNIEnv* const env,
                                            const JniReference<jobject>& jvalue,
                                            TypeId<std::optional<std::string>>) {
    return jvalue ? std::optional<std::string>(convert_string_from_jni(env, static_cast<jstring>(jvalue.get())))
                  : std::optional<std::string>{};
}

std::optional<std::string> convert_from_jni(JNIEnv* const env,
                                            const JniReference<jstring>& jvalue,
                                            TypeId<std::optional<std::string>>) {
    return jvalue ? std::optional<std::string>(convert_string_from_jni(env, jvalue.get()))
                  : std::optional<std::string>{};
}

std::shared_ptr<std::vector<uint8_t>> convert_from_jni(JNIEnv* const env,
                                                       const JniReference<jbyteArray>& jvalue,
                                                       TypeId<std::shared_ptr<std::vector<uint8_t>>>) {
    if (!jvalue) {
        ::glue_internal::jni::throw_new_null_pointer_exception(env);
        return {};
    }

    size_t size = static_cast<size_t>(env->GetArrayLength(jvalue.get()));
    auto nresult = std::make_shared<std::vector<uint8_t>>(size);
    jbyte* jbuffer = reinterpret_cast<jbyte*>(nresult->data());
    env->GetByteArrayRegion(jvalue.get(), 0, size, jbuffer);
    return nresult;
}

std::optional<std::shared_ptr<std::vector<uint8_t>>> convert_from_jni(
    JNIEnv* const env,
    const JniReference<jbyteArray>& jvalue,
    TypeId<std::optional<std::shared_ptr<std::vector<uint8_t>>>>) {
    return jvalue ? std::optional<std::shared_ptr<std::vector<uint8_t>>>(
                        convert_from_jni(env, jvalue, TypeId<std::shared_ptr<std::vector<uint8_t>>>{}))
                  : std::optional<std::shared_ptr<std::vector<uint8_t>>>{};
}

jlong get_time_ms_epoch(JNIEnv* const env, const JniReference<jobject>& jvalue) noexcept {
    return call_java_method<jlong>(env, "java/util/Date", jvalue, "getTime", "()J");
}

::glue_internal::Locale convert_from_jni(JNIEnv* const env,
                                         const JniReference<jobject>& jvalue,
                                         TypeId<::glue_internal::Locale>) {
    if (!jvalue) {
        ::glue_internal::jni::throw_new_null_pointer_exception(env);
        return ::glue_internal::Locale();
    }

    const auto& locale_class = get_object_class(env, jvalue);
    const auto languageCode = call_java_method<jstring>(
        env, locale_class, jvalue, "getLanguage", "()Ljava/lang/String;");
    const auto countryCode = call_java_method<jstring>(env, locale_class, jvalue, "getCountry", "()Ljava/lang/String;");
    const auto scriptCode = call_java_method<jstring>(env, locale_class, jvalue, "getScript", "()Ljava/lang/String;");
    const auto languageTag = call_java_method<jstring>(
        env, locale_class, jvalue, "toLanguageTag", "()Ljava/lang/String;");

    return ::glue_internal::Locale(convert_from_jni(env, languageCode, TypeId<std::string>{}),
                                   convert_from_jni(env, countryCode, TypeId<std::string>{}),
                                   convert_from_jni(env, scriptCode, TypeId<std::string>{}),
                                   convert_from_jni(env, languageTag, TypeId<std::string>{}));
}

std::optional<::glue_internal::Locale> convert_from_jni(JNIEnv* const env,
                                                        const JniReference<jobject>& jvalue,
                                                        TypeId<std::optional<::glue_internal::Locale>>) {
    return jvalue ? std::optional<::glue_internal::Locale>(
                        convert_from_jni(env, jvalue, TypeId<::glue_internal::Locale>{}))
                  : std::optional<::glue_internal::Locale>{};
}

// -------------------- C++ to JNI conversion functions --------------------------------------------

JniReference<jstring> convert_to_jni(JNIEnv* const env, const std::string& nvalue) noexcept {
    return make_local_ref(env, env->NewStringUTF(nvalue.c_str()));
}

JniReference<jstring> convert_to_jni(JNIEnv* const env, const std::optional<std::string>& nvalue) noexcept {
    return nvalue ? make_local_ref(env, env->NewStringUTF(nvalue->c_str())) : JniReference<jstring>{};
}

JniReference<jbyteArray> convert_to_jni(JNIEnv* const env,
                                        const std::shared_ptr<std::vector<uint8_t>>& nvalue) noexcept {
    if (!nvalue) {
        return make_local_ref(env, env->NewByteArray(0));
    }

    const jsize size = static_cast<jsize>(nvalue->size());
    auto jresult = make_local_ref(env, env->NewByteArray(size));
    const jbyte* const jbuffer = reinterpret_cast<const jbyte*>(nvalue->data());
    env->SetByteArrayRegion(jresult.get(), 0, size, jbuffer);

    return jresult;
}

JniReference<jbyteArray> convert_to_jni(JNIEnv* const env,
                                        const std::optional<std::shared_ptr<std::vector<uint8_t>>>& nvalue) noexcept {
    return nvalue ? convert_to_jni(env, *nvalue) : JniReference<jbyteArray>{};
}

JniReference<jobject> create_date_new_object(JNIEnv* const env, const std::chrono::milliseconds& time_epoch) {
    const auto java_date_class = find_class(env, "java/util/Date");
    const jlong time_ms_epoch = time_epoch.count();

    const auto constructor_method_id = env->GetMethodID(java_date_class.get(), "<init>", "(J)V");
    return new_object(env, java_date_class, constructor_method_id, time_ms_epoch);
}

std::intmax_t get_duration_from_java_duration(JNIEnv* const env,
                                              const JniReference<jobject>& jvalue,
                                              const std::intmax_t dest_den,
                                              const std::intmax_t dest_num) {
    const auto& java_duration_class = get_cached_duration_class();
    const jlong seconds_value = call_java_method<jlong>(env, java_duration_class, jvalue, "getSeconds", "()J");
    const jint nano_value = call_java_method<jint>(env, java_duration_class, jvalue, "getNano", "()I");

    using namespace std::chrono;

    const auto seconds_division = std::lldiv(seconds_value * dest_den, dest_num);
    const auto combined_nano_value = duration_cast<nanoseconds>(seconds(seconds_division.rem)).count() + nano_value;
    const auto num = dest_den * nanoseconds::period::num;
    const auto den = dest_num * nanoseconds::period::den;
    const auto nano_division = std::lldiv(combined_nano_value * num, den);
    auto result_value = seconds_division.quot + nano_division.quot;

    // Rounding
    if (2 * nano_division.rem >= den) {
        result_value += 1;
    }

    return result_value;
}

JniReference<jobject> create_duration_new_object(JNIEnv* const env,
                                                 std::intmax_t seconds,
                                                 std::intmax_t nanos) noexcept {
    const auto& java_duration_class = get_cached_duration_class();
    const auto factory_method_id = env->GetStaticMethodID(
        java_duration_class.get(), "ofSeconds", "(JJ)Lcom/example/time/Duration;");
    return make_local_ref(env,
                          env->CallStaticObjectMethod(java_duration_class.get(), factory_method_id, seconds, nanos));
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, const ::glue_internal::Locale& nvalue) noexcept {
    const auto locale_builder_class = find_class(env, "java/util/Locale$Builder");
    const auto builder = create_object(env, locale_builder_class);
    const char* const set_method_signature = "(Ljava/lang/String;)Ljava/util/Locale$Builder;";
    if (nvalue.language_tag) {
        // BCP 47 language tag takes precedence if present.
        call_java_method<jobject>(env,
                                  locale_builder_class,
                                  builder,
                                  "setLanguageTag",
                                  set_method_signature,
                                  convert_to_jni(env, nvalue.language_tag));
        if (env->ExceptionOccurred()) return {};
    } else {
        // java.util.Locale has no constructor that takes "script" code,
        // so Locale.Builder has to be used instead to create a Locale from ISO codes.
        call_java_method<jobject>(env,
                                  locale_builder_class,
                                  builder,
                                  "setLanguage",
                                  set_method_signature,
                                  convert_to_jni(env, nvalue.language_code));
        if (env->ExceptionOccurred()) return {};

        call_java_method<jobject>(env,
                                  locale_builder_class,
                                  builder,
                                  "setRegion",
                                  set_method_signature,
                                  convert_to_jni(env, nvalue.country_code));
        if (env->ExceptionOccurred()) return {};

        call_java_method<jobject>(env,
                                  locale_builder_class,
                                  builder,
                                  "setScript",
                                  set_method_signature,
                                  convert_to_jni(env, nvalue.script_code));
        if (env->ExceptionOccurred()) return {};
    }
    return call_java_method<jobject>(env, locale_builder_class, builder, "build", "()Ljava/util/Locale;");
}

JniReference<jobject> convert_to_jni(JNIEnv* const env, const std::optional<::glue_internal::Locale>& nvalue) noexcept {
    return nvalue ? convert_to_jni(env, *nvalue) : JniReference<jobject>{};
}

} // namespace jni
} // namespace glue_internal
