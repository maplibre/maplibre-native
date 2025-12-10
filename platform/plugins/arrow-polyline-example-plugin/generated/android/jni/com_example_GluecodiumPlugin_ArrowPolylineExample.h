/*

 *
 */

#pragma once

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong JNICALL Java_com_example_GluecodiumPlugin_ArrowPolylineExample_create(JNIEnv* _jenv,
                                                                                      jobject _jinstance);
JNIEXPORT void JNICALL Java_com_example_GluecodiumPlugin_ArrowPolylineExample_addArrowPolyline(JNIEnv* _jenv,
                                                                                               jobject _jinstance,
                                                                                               jobject jcoordinates,
                                                                                               jobject jconfig);
JNIEXPORT void JNICALL Java_com_example_GluecodiumPlugin_ArrowPolylineExample_removeArrowPolyline(JNIEnv* _jenv,
                                                                                                  jobject _jinstance);

#ifdef __cplusplus
}
#endif
