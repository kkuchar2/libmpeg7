#include <jni.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jstring JNICALL Java_com_mpeg7_bridge_libmpeg7_extractDescriptor
        (JNIEnv *, jobject, jint, jstring, jobjectArray);

JNIEXPORT jstring JNICALL Java_com_mpeg7_bridge_libmpeg7_extractDescriptorFromData
        (JNIEnv *, jobject, jint, jbyteArray, jobjectArray);

JNIEXPORT jstring JNICALL Java_com_mpeg7_bridge_libmpeg7_calculateDistance
        (JNIEnv *, jobject, jstring, jstring, jobjectArray);

#ifdef __cplusplus
}
#endif
