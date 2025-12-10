/*

 *
 */

#include "com_example_GluecodiumPlugin_ArrowPolylineConfig__Conversion.h"
#include "com_example_GluecodiumPlugin_ArrowPolylineExample.h"
#include "com_example_GluecodiumPlugin_ArrowPolylineExample__Conversion.h"
#include "com_example_GluecodiumPlugin_LatLng__Conversion.h"
#include "com_example_GluecodiumPlugin_MaplibrePlugin__Conversion.h"
#include "ArrayConversionUtils.h"
#include "JniClassCache.h"
#include "JniNativeHandle.h"
#include "JniReference.h"
#include "JniThrowNewException.h"
#include "JniWrapperCache.h"

extern "C" {

jlong
Java_com_example_GluecodiumPlugin_ArrowPolylineExample_create(JNIEnv* _jenv, jobject _jinstance)

{





    auto _result = ::GluecodiumPlugin::ArrowPolylineExample::create();

    auto nSharedPtr = new (::std::nothrow) ::std::shared_ptr< ::GluecodiumPlugin::ArrowPolylineExample >(_result);
    if (nSharedPtr == nullptr)
    {
        ::glue_internal::jni::throw_new_out_of_memory_exception(_jenv);;
        return 0;
    }
    return reinterpret_cast<jlong>(nSharedPtr);
}

void
Java_com_example_GluecodiumPlugin_ArrowPolylineExample_addArrowPolyline(JNIEnv* _jenv, jobject _jinstance, jobject jcoordinates, jobject jconfig)

{



    ::std::vector< ::GluecodiumPlugin::LatLng > coordinates = ::glue_internal::jni::convert_from_jni(_jenv,
            ::glue_internal::jni::make_non_releasing_ref(jcoordinates),
            ::glue_internal::jni::TypeId<::std::vector< ::GluecodiumPlugin::LatLng >>{});



    ::GluecodiumPlugin::ArrowPolylineConfig config = ::glue_internal::jni::convert_from_jni(_jenv,
            ::glue_internal::jni::make_non_releasing_ref(jconfig),
            ::glue_internal::jni::TypeId<::GluecodiumPlugin::ArrowPolylineConfig>{});



    auto pInstanceSharedPointer = reinterpret_cast<std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample>*> (

        ::glue_internal::jni::get_class_native_handle(_jenv,_jinstance));




    (*pInstanceSharedPointer)->add_arrow_polyline(coordinates,config);

}

void
Java_com_example_GluecodiumPlugin_ArrowPolylineExample_removeArrowPolyline(JNIEnv* _jenv, jobject _jinstance)

{



    auto pInstanceSharedPointer = reinterpret_cast<std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample>*> (

        ::glue_internal::jni::get_class_native_handle(_jenv,_jinstance));




    (*pInstanceSharedPointer)->remove_arrow_polyline();

}



JNIEXPORT void JNICALL
Java_com_example_GluecodiumPlugin_ArrowPolylineExample_disposeNativeHandle(JNIEnv* _jenv, jobject _jinstance, jlong _jpointerRef)
{
    auto p_nobj = reinterpret_cast<std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample>*>(_jpointerRef);
    ::glue_internal::jni::JniWrapperCache::remove_cached_wrapper(_jenv, *p_nobj);
    delete p_nobj;
}

JNIEXPORT void JNICALL
Java_com_example_GluecodiumPlugin_ArrowPolylineExample_cacheThisInstance(JNIEnv* _jenv, jobject _jinstance)
{
    auto jobj = ::glue_internal::jni::make_non_releasing_ref(_jinstance);
    auto long_ptr = ::glue_internal::jni::get_class_native_handle(_jenv, jobj);
    auto nobj = *reinterpret_cast<std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample>*>(long_ptr);

    ::glue_internal::jni::JniWrapperCache::cache_wrapper(_jenv, nobj, jobj);

}
}
