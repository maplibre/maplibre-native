/*

 *
 */

#pragma once

#include "GluecodiumPlugin/ArrowPolylineConfig.h"
#include "JniReference.h"
#include "JniTypeId.h"
#include <optional>

namespace glue_internal
{
namespace jni
{
JNIEXPORT ::GluecodiumPlugin::ArrowPolylineConfig convert_from_jni(JNIEnv* _jenv, const JniReference<jobject>& _jinput, TypeId<::GluecodiumPlugin::ArrowPolylineConfig>);
JNIEXPORT std::optional<::GluecodiumPlugin::ArrowPolylineConfig> convert_from_jni(JNIEnv* _jenv, const JniReference<jobject>& _jinput, TypeId<std::optional<::GluecodiumPlugin::ArrowPolylineConfig>>);
JNIEXPORT JniReference<jobject> convert_to_jni(JNIEnv* _jenv, const ::GluecodiumPlugin::ArrowPolylineConfig& _ninput);
JNIEXPORT JniReference<jobject> convert_to_jni(JNIEnv* _jenv, const std::optional<::GluecodiumPlugin::ArrowPolylineConfig> _ninput);
}
}
