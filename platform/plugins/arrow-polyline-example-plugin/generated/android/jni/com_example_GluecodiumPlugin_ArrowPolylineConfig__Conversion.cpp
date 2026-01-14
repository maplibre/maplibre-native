/*

 *
 */

#include "com_example_GluecodiumPlugin_ArrowPolylineConfig__Conversion.h"
#include "ArrayConversionUtils.h"
#include "FieldAccessMethods.h"
#include "JniCallJavaMethod.h"
#include "JniClassCache.h"

namespace glue_internal {
namespace jni {

::GluecodiumPlugin::ArrowPolylineConfig convert_from_jni(JNIEnv* _jenv,
                                                         const JniReference<jobject>& _jinput,
                                                         TypeId<::GluecodiumPlugin::ArrowPolylineConfig>) {
    ::GluecodiumPlugin::ArrowPolylineConfig _nout{};
    double n_head_length = ::glue_internal::jni::get_field_value(_jenv, _jinput, "headLength", TypeId<double>{});
    _nout.head_length = n_head_length;
    double n_head_angle = ::glue_internal::jni::get_field_value(_jenv, _jinput, "headAngle", TypeId<double>{});
    _nout.head_angle = n_head_angle;
    ::std::string n_line_color = ::glue_internal::jni::get_field_value(
        _jenv, _jinput, "lineColor", TypeId<::std::string>{});
    _nout.line_color = n_line_color;
    double n_line_width = ::glue_internal::jni::get_field_value(_jenv, _jinput, "lineWidth", TypeId<double>{});
    _nout.line_width = n_line_width;
    return _nout;
}

std::optional<::GluecodiumPlugin::ArrowPolylineConfig> convert_from_jni(
    JNIEnv* _jenv,
    const JniReference<jobject>& _jinput,
    TypeId<std::optional<::GluecodiumPlugin::ArrowPolylineConfig>>) {
    return _jinput ? std::optional<::GluecodiumPlugin::ArrowPolylineConfig>(
                         convert_from_jni(_jenv, _jinput, TypeId<::GluecodiumPlugin::ArrowPolylineConfig>{}))
                   : std::optional<::GluecodiumPlugin::ArrowPolylineConfig>{};
}

REGISTER_JNI_CLASS_CACHE("com/example/GluecodiumPlugin/ArrowPolylineConfig",
                         com_example_GluecodiumPlugin_ArrowPolylineConfig,
                         ::GluecodiumPlugin::ArrowPolylineConfig)

JniReference<jobject> convert_to_jni(JNIEnv* _jenv, const ::GluecodiumPlugin::ArrowPolylineConfig& _ninput) {
    auto& javaClass = CachedJavaClass<::GluecodiumPlugin::ArrowPolylineConfig>::java_class;
    auto _jresult = ::glue_internal::jni::alloc_object(_jenv, javaClass);

    ::glue_internal::jni::set_field_value(_jenv, _jresult, "headLength", _ninput.head_length);

    ::glue_internal::jni::set_field_value(_jenv, _jresult, "headAngle", _ninput.head_angle);

    ::glue_internal::jni::set_field_value(_jenv, _jresult, "lineColor", _ninput.line_color);

    ::glue_internal::jni::set_field_value(_jenv, _jresult, "lineWidth", _ninput.line_width);
    return _jresult;
}

JniReference<jobject> convert_to_jni(JNIEnv* _jenv,
                                     const std::optional<::GluecodiumPlugin::ArrowPolylineConfig> _ninput) {
    return _ninput ? convert_to_jni(_jenv, *_ninput) : JniReference<jobject>{};
}

} // namespace jni
} // namespace glue_internal
