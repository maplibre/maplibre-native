/*

 *
 */

#pragma once

#include "GluecodiumPlugin/LatLng.h"
#include "JniReference.h"
#include "JniTypeId.h"
#include <optional>

namespace glue_internal
{
namespace jni
{
JNIEXPORT ::GluecodiumPlugin::LatLng convert_from_jni(JNIEnv* _jenv, const JniReference<jobject>& _jinput, TypeId<::GluecodiumPlugin::LatLng>);
JNIEXPORT std::optional<::GluecodiumPlugin::LatLng> convert_from_jni(JNIEnv* _jenv, const JniReference<jobject>& _jinput, TypeId<std::optional<::GluecodiumPlugin::LatLng>>);
JNIEXPORT JniReference<jobject> convert_to_jni(JNIEnv* _jenv, const ::GluecodiumPlugin::LatLng& _ninput);
JNIEXPORT JniReference<jobject> convert_to_jni(JNIEnv* _jenv, const std::optional<::GluecodiumPlugin::LatLng> _ninput);
}
}
