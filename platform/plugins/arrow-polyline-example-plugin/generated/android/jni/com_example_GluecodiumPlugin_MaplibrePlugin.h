/*

 *
 */

#pragma once

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong JNICALL
Java_com_example_GluecodiumPlugin_MaplibrePlugin_create(JNIEnv* _jenv, jobject _jinstance);

JNIEXPORT jlong JNICALL
Java_com_example_GluecodiumPlugin_MaplibrePlugin_getPtr(JNIEnv* _jenv, jobject _jinstance);




#ifdef __cplusplus
}
#endif
