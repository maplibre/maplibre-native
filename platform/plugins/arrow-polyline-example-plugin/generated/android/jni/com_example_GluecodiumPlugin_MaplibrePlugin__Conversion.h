/*

 *
 */

#pragma once

#include "GluecodiumPlugin/MaplibrePlugin.h"
#include "JniReference.h"
#include "JniTypeId.h"
#include <memory>
#include <optional>

namespace glue_internal
{
namespace jni
{

JNIEXPORT std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin> convert_from_jni(JNIEnv* _env, const JniReference<jobject>& _jobj, TypeId<std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>>);
JNIEXPORT JniReference<jobject> convert_to_jni(JNIEnv* _jenv, const std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>& _ninput);

}
}
