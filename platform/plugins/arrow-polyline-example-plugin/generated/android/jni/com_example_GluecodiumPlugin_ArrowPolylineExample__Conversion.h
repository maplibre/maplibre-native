/*

 *
 */

#pragma once

#include "GluecodiumPlugin/ArrowPolylineExample.h"
#include "com_example_GluecodiumPlugin_ArrowPolylineConfig__Conversion.h"
#include "com_example_GluecodiumPlugin_LatLng__Conversion.h"
#include "com_example_GluecodiumPlugin_MaplibrePlugin__Conversion.h"
#include "JniReference.h"
#include "JniTypeId.h"
#include <memory>
#include <optional>

namespace glue_internal
{
namespace jni
{

JNIEXPORT std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample> convert_from_jni(JNIEnv* _env, const JniReference<jobject>& _jobj, TypeId<std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample>>);
JNIEXPORT JniReference<jobject> convert_to_jni(JNIEnv* _jenv, const std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample>& _ninput);

}
}
