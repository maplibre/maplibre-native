/*

 *
 */

#include "com_example_GluecodiumPlugin_LatLng__Conversion.h"
#include "ArrayConversionUtils.h"
#include "FieldAccessMethods.h"
#include "JniCallJavaMethod.h"
#include "JniClassCache.h"

namespace glue_internal
{
namespace jni
{

::GluecodiumPlugin::LatLng
convert_from_jni(JNIEnv* _jenv, const JniReference<jobject>& _jinput, TypeId<::GluecodiumPlugin::LatLng>)
{
    ::GluecodiumPlugin::LatLng _nout{};
    double n_latitude = ::glue_internal::jni::get_field_value(
        _jenv,
        _jinput,
        "latitude",
        TypeId<double>{} );
    _nout.latitude = n_latitude;
    double n_longitude = ::glue_internal::jni::get_field_value(
        _jenv,
        _jinput,
        "longitude",
        TypeId<double>{} );
    _nout.longitude = n_longitude;
    return _nout;
}

std::optional<::GluecodiumPlugin::LatLng>
convert_from_jni(JNIEnv* _jenv, const JniReference<jobject>& _jinput, TypeId<std::optional<::GluecodiumPlugin::LatLng>>)
{
    return _jinput
        ? std::optional<::GluecodiumPlugin::LatLng>(convert_from_jni(_jenv, _jinput, TypeId<::GluecodiumPlugin::LatLng>{}))
        : std::optional<::GluecodiumPlugin::LatLng>{};
}

REGISTER_JNI_CLASS_CACHE("com/example/GluecodiumPlugin/LatLng", com_example_GluecodiumPlugin_LatLng, ::GluecodiumPlugin::LatLng)

JniReference<jobject>
convert_to_jni(JNIEnv* _jenv, const ::GluecodiumPlugin::LatLng& _ninput)
{
    auto& javaClass = CachedJavaClass<::GluecodiumPlugin::LatLng>::java_class;
    auto _jresult = ::glue_internal::jni::alloc_object(_jenv, javaClass);

    ::glue_internal::jni::set_field_value(_jenv, _jresult, "latitude", _ninput.latitude);

    ::glue_internal::jni::set_field_value(_jenv, _jresult, "longitude", _ninput.longitude);
    return _jresult;
}

JniReference<jobject>
convert_to_jni(JNIEnv* _jenv, const std::optional<::GluecodiumPlugin::LatLng> _ninput)
{
    return _ninput ? convert_to_jni(_jenv, *_ninput) : JniReference<jobject>{};
}

}
}
