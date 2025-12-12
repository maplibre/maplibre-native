/*

 *
 */

#include "com_example_GluecodiumPlugin_MaplibrePlugin.h"
#include "com_example_GluecodiumPlugin_MaplibrePlugin__Conversion.h"
#include "ArrayConversionUtils.h"
#include "JniClassCache.h"
#include "JniNativeHandle.h"
#include "JniReference.h"
#include "JniThrowNewException.h"
#include "JniWrapperCache.h"

extern "C" {

jlong Java_com_example_GluecodiumPlugin_MaplibrePlugin_create(JNIEnv* _jenv, jobject _jinstance)

{
    auto _result = ::GluecodiumPlugin::MaplibrePlugin::create();

    auto nSharedPtr = new (::std::nothrow)::std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>(_result);
    if (nSharedPtr == nullptr) {
        ::glue_internal::jni::throw_new_out_of_memory_exception(_jenv);
        ;
        return 0;
    }
    return reinterpret_cast<jlong>(nSharedPtr);
}

jlong Java_com_example_GluecodiumPlugin_MaplibrePlugin_getPtr(JNIEnv* _jenv, jobject _jinstance)

{
    auto pInstanceSharedPointer = reinterpret_cast<std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>*>(

        ::glue_internal::jni::get_class_native_handle(_jenv, _jinstance));

    auto _result = (*pInstanceSharedPointer)->get_ptr();

    return _result;
}

JNIEXPORT void JNICALL Java_com_example_GluecodiumPlugin_MaplibrePlugin_disposeNativeHandle(JNIEnv* _jenv,
                                                                                            jobject _jinstance,
                                                                                            jlong _jpointerRef) {
    auto p_nobj = reinterpret_cast<std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>*>(_jpointerRef);
    ::glue_internal::jni::JniWrapperCache::remove_cached_wrapper(_jenv, *p_nobj);
    delete p_nobj;
}

JNIEXPORT void JNICALL Java_com_example_GluecodiumPlugin_MaplibrePlugin_cacheThisInstance(JNIEnv* _jenv,
                                                                                          jobject _jinstance) {
    auto jobj = ::glue_internal::jni::make_non_releasing_ref(_jinstance);
    auto long_ptr = ::glue_internal::jni::get_class_native_handle(_jenv, jobj);
    auto nobj = *reinterpret_cast<std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>*>(long_ptr);

    ::glue_internal::jni::JniWrapperCache::cache_wrapper(_jenv, nobj, jobj);
}
}
