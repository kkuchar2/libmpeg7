#include <jni.h>

#include "../Mpeg7.h"
#include "com_mpeg7_bridge_libmpeg7.h"

jstring createJniMessage(JNIEnv * env, const char * message);
const char ** getParameters(JNIEnv * env, jobjectArray parameters);
unsigned char * getImageBuffer(JNIEnv * env, jbyteArray data, int size);

JNIEXPORT jstring JNICALL Java_com_mpeg7_bridge_libmpeg7_extractDescriptor
(JNIEnv * env, jobject, jint descriptorType, jstring imgURL, jobjectArray parameters) {
    int desType;
    const char * imgPath = nullptr;
    const char ** params = nullptr;

    if (!descriptorType) {
        return createJniMessage(env, message(JNI_DESCRIPTOR_TYPE_NULL));
    }

    if (!imgURL) {
        return createJniMessage(env, message(JNI_IMG_NULL));
    }

    if (!parameters) {
        return createJniMessage(env, message(JNI_PARAMS_NULL));
    }
  
    desType = descriptorType;

    imgPath = env->GetStringUTFChars(imgURL, (jboolean *) nullptr);

    if (imgPath == nullptr) {
        return createJniMessage(env, message(JNI_IMG_NULL));
    }

    try {
        params = getParameters(env, parameters);
    }
    catch (ErrorCode exception) {
        return createJniMessage(env, message(exception));
    }

    if (params == nullptr) {
        return createJniMessage(env, message(JNI_PARAMS_NULL));
    }

    const char * result = extractDescriptor(static_cast<DescriptorType>(desType), imgPath, params);

    env->ReleaseStringUTFChars(imgURL, imgPath);    
    
    const int param_len = env->GetArrayLength(parameters);

    if (param_len != 0) {
        for (int i = 0; i < param_len; i++) {
            env->ReleaseStringUTFChars(static_cast<jstring>(env->GetObjectArrayElement(parameters, i)), params[i]);
        }
    }
    delete[] params;

    return createJniMessage(env, result);
}

JNIEXPORT jstring JNICALL Java_com_mpeg7_bridge_libmpeg7_extractDescriptorFromData
(JNIEnv * env, jobject, jint descriptorType, jbyteArray data, jobjectArray parameters) {
    int desType;
    const char * imgPath = nullptr;

    if (!descriptorType) {
        return createJniMessage(env, message(JNI_DESCRIPTOR_TYPE_NULL));
    }

    if (!data) {
        return createJniMessage(env, message(JNI_IMG_NULL));
    }

    if (!parameters) {
        return createJniMessage(env, message(JNI_PARAMS_NULL));
    }

    desType = descriptorType;

    const jsize data_len = env->GetArrayLength(data);

    unsigned char * imgData = getImageBuffer(env, data, data_len);

    const char ** params = nullptr;

    try {
        params = getParameters(env, parameters);
    }
    catch (ErrorCode exception) {
        env->NewStringUTF(message(exception));
    }

    if (params == nullptr) {
        env->NewStringUTF(message(JNI_PARAMS_NULL));
    }

    const char * result = extractDescriptorFromData(static_cast<DescriptorType>(desType), imgData, data_len, params);

    delete[] imgData;

    const int param_len = env->GetArrayLength(parameters);

    if (param_len != 0) {
        for (int i = 0; i < param_len; i++) {
            env->ReleaseStringUTFChars(static_cast<jstring>(env->GetObjectArrayElement(parameters, i)), params[i]);
        }
    }
    delete[] params;
    return createJniMessage(env, result);
}

JNIEXPORT jstring JNICALL Java_com_mpeg7_bridge_libmpeg7_calculateDistance
(JNIEnv * env, jobject, jstring xml1, jstring xml2, jobjectArray parameters) {
    const char *  xml1_str = nullptr;
    const char *  xml2_str = nullptr;
    const char ** params   = nullptr;

    if (!xml1 || !xml2) {
        return createJniMessage(env, message(JNI_XML_NULL));
    }

    if (!parameters) {
        return createJniMessage(env, message(JNI_PARAMS_NULL));
    }

    xml1_str = env->GetStringUTFChars(xml1, (jboolean *) nullptr);
    xml2_str = env->GetStringUTFChars(xml2, (jboolean *) nullptr);

    if (xml1_str == nullptr || xml2_str == nullptr) {
        return createJniMessage(env, message(JNI_XML_NULL));
    }

    try {
        params = getParameters(env, parameters);
    }
    catch (ErrorCode exception) {
        return createJniMessage(env, message(exception));
    }

    if (params == nullptr) {
        return createJniMessage(env, message(JNI_PARAMS_NULL));
    }

    const char * result = getDistance(xml1_str, xml2_str, params);

    env->ReleaseStringUTFChars(xml1, xml1_str);
    env->ReleaseStringUTFChars(xml2, xml2_str);

    const int param_len = env->GetArrayLength(parameters);

    if (param_len != 0) {
        for (int i = 0; i < param_len; i++) {
            env->ReleaseStringUTFChars(static_cast<jstring>(env->GetObjectArrayElement(parameters, i)), params[i]);
        }
    }
    delete[] params;

    return createJniMessage(env, result);
}

jstring createJniMessage(JNIEnv * env, const char * message) {
    const jstring jniMessage = env->NewStringUTF(message);
    delete[] message;
    return jniMessage;
}

const char ** getParameters(JNIEnv * env, jobjectArray parameters) {
    const char ** params = nullptr;

    const int stringCount = env->GetArrayLength(parameters);

    if (stringCount == 0) {
        params = new const char * [stringCount + 1];
        params[0] = nullptr;
    }
    else if (stringCount == 1) {
        const auto string = static_cast<jstring>(env->GetObjectArrayElement(parameters, 0));

        if (string == nullptr) {
            params = new const char * [stringCount];
            params[0] = nullptr;
        }
        else {
            params = new const char *[stringCount + 1];
            params[0] = env->GetStringUTFChars(string, (jboolean *) nullptr);
            params[1] = nullptr;
        }
    }
    else {
        params = new const char * [stringCount + 1];

        for (int i = 0; i < stringCount; i++) {
            const auto string = static_cast<jstring>(env->GetObjectArrayElement(parameters, i));

            if (string == nullptr) {
                delete[] params;
                throw JNI_PARAMS_NULL;
            }
            params[i] = env->GetStringUTFChars(string, (jboolean *) nullptr);
        }
        params[stringCount] = nullptr;
    }

    return params;
}

unsigned char * getImageBuffer(JNIEnv * env, jbyteArray data, int size) {
    jbyte * jniBufferPtr = env->GetByteArrayElements(data, (jboolean *) nullptr);

    const auto imgBuffer = new unsigned char[size];

    int i = 0;

    for (int i = 0; i < size; i++) {
        imgBuffer[i] = jniBufferPtr[i];
    }

    env->ReleaseByteArrayElements(data, jniBufferPtr, 0);

    return imgBuffer;
}
