/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                               JNI SECTION
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/** @file   com_ztv_mpeg7_libmpeg7pw.cpp
 *  @brief 
 *
 * Methods for:
 * - Extraction from image having local path
 * - Extraction for image raw data passed as argument (pure file byte data)
 * - Distance calculation from two xml descriptors
 *
 * Each method has following steps:
 * 1. C data types declaration
 * 2. Arguments check
 * 3. Parse from JNI data types to declared C data type
 * 4. Extraction or distance calculation
 * 5. Unneeded memory release
 * 6. Returning result as jstring
 * 
 *  @author Krzysztof Kucharski
 *  @bug    No bugs detected. */
 /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <jni.h>

#include "../Mpeg7.h"
#include "com_mpeg7_bridge_libmpeg7.h"

/** @brief
 * Creates jstring message from const char *
 * integer code passed as argument
 * @param env - JNI environment pointer
 * @param message - const char * original message
 * @return jstring - created jstring from original message */
jstring createJniMessage(JNIEnv * env, const char * message);

/** @brief
* Creates const char ** array of parameters from Java
* String array of parameters and filters proper parameter combinations:
* 
* Input (Java)                   | Output (C)
* ------------------------------ | ------------------------------------------
* 1. empty                       | [NULL]
* 2. [null]                      | [NULL]
* 3. [value]                     | [value, NULL]
* 4. [multiple not null values]  | [value_1, value_2, (...), value_n, NULL]
*
* Other input combinations cause returning error code.
*
* @param env - JNI environment pointer
* @param parameters - array of Java String values
* @return const char ** - created C array */
const char ** getParameters(JNIEnv * env, jobjectArray parameters);

/** @brief
* Creates unsigned char * array of image file data from
* Java byte[] array
*
* @param env - JNI environment pointer
* @param data - Java byte[] array
* @return unsigned char * - created C array */
unsigned char * getImageBuffer(JNIEnv * env, jbyteArray data, int size);

/** @brief
* MPEG-7 Descriptor extraction function from local image path.
*
* @param env - JNI environment pointer
* @param descriptorType - integer indicating descriptor type (DescriptorType)
* @param imgURL - local image path
* @param parameters - additional extraction parameters array
* @return jstring - XML result of extraction */
JNIEXPORT jstring JNICALL Java_com_mpeg7_bridge_libmpeg7_extractDescriptor
(JNIEnv * env, jobject, jint descriptorType, jstring imgURL, jobjectArray parameters) {
    /* ---------------- 1. Declare C data for extraction: ----------------- */
 
    int desType;		         // Type of descriptor
    const char * imgPath = nullptr; // Path to image
    const char ** params = nullptr; // Parameters

    /* ---------------- 2. Check arguments: ------------------------------  */

    if (!descriptorType) {
        return createJniMessage(env, message(JNI_DESCRIPTOR_TYPE_NULL));
    }

    if (!imgURL) {
        return createJniMessage(env, message(JNI_IMG_NULL));
    }

    if (!parameters) {
        return createJniMessage(env, message(JNI_PARAMS_NULL));
    }
  
    /* ---------------- 3. Parse JNI data to C data: ---------------------- */
  
    // Parse descriptor type
    desType = descriptorType;

    // Parse image path
    imgPath = env->GetStringUTFChars(imgURL, (jboolean *) nullptr);


    if (imgPath == nullptr) {
        return createJniMessage(env, message(JNI_IMG_NULL));
    }

    // Parse parameters
    try {
        params = getParameters(env, parameters);
    }
    catch (ErrorCode exception) {
        return createJniMessage(env, message(exception));
    }

    if (params == nullptr) {
        return createJniMessage(env, message(JNI_PARAMS_NULL));
    }

    /* ---------------- 4. Extract: -------------------------------------- */

    const char * result = extractDescriptor(static_cast<DescriptorType>(desType), imgPath, params);

    /* ---------------- 5. Cleanup: -------------------------------------- */

    // a) Descriptor type as int does not have to be released

    /* b) Release image path - only from JNI env, 
          because it was accessed directly from there */
    env->ReleaseStringUTFChars(imgURL, imgPath);    
    
    // c) Release parameters - from JNI env and from C
    const int param_len = env->GetArrayLength(parameters);

    // from env
    if (param_len != 0) {
        for (int i = 0; i < param_len; i++) {
            // Releases each array element:
            env->ReleaseStringUTFChars(static_cast<jstring>(env->GetObjectArrayElement(parameters, i)), params[i]);
        }
    }
    // from C
    delete[] params;

    /* ---------------- 6. Return extraction result --------------------- */
    return createJniMessage(env, result);
}

/** @brief
* MPEG-7 Descriptor extraction function from image file data.
*
* @param env - JNI environment pointer
* @param descriptorType - integer indicating descriptor type (DescriptorType)
* @param data - image file data stored in byte array
* @param parameters - additional extraction parameters array
* @return jstring - XML result of extraction */
JNIEXPORT jstring JNICALL Java_com_mpeg7_bridge_libmpeg7_extractDescriptorFromData
(JNIEnv * env, jobject, jint descriptorType, jbyteArray data, jobjectArray parameters) {
    /* ---------------- 1. Declare C data for extraction: ----------------- */

    int desType;		         // Type of descriptor
    const char * imgPath = nullptr; // Path to image

    /* ---------------- 2. Check arguments: ------------------------------  */
    if (!descriptorType) {
        return createJniMessage(env, message(JNI_DESCRIPTOR_TYPE_NULL));
    }

    if (!data) {
        return createJniMessage(env, message(JNI_IMG_NULL));
    }

    if (!parameters) {
        return createJniMessage(env, message(JNI_PARAMS_NULL));
    }

    /* ---------------- 3. Parse JNI data to C data: ---------------------- */

    // Parse descriptor type
    desType = descriptorType;

    // Parse image data
    const jsize data_len = env->GetArrayLength(data);

    unsigned char * imgData = getImageBuffer(env, data, data_len);

    // Parse parameters
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

    /* ---------------- 4. Extract: -------------------------------------- */
    const char * result = extractDescriptorFromData(static_cast<DescriptorType>(desType), imgData, data_len, params);

    /* ---------------- 5. Cleanup: -------------------------------------- */
    // a) Descriptor type as int does not have to be released

    // b) Delete imgData from C
    delete[] imgData;

    // c) Release parameters - from JNI env and from C
    const int param_len = env->GetArrayLength(parameters);

    // from env
    if (param_len != 0) {
        for (int i = 0; i < param_len; i++) {
            // Releases each array element:
            env->ReleaseStringUTFChars(static_cast<jstring>(env->GetObjectArrayElement(parameters, i)), params[i]);
        }
    }
    // from C
    delete[] params;
    /* ---------------- 6. Return extraction result --------------------- */
    return createJniMessage(env, result);
}

/** @brief
* MPEG-7 Descriptor comparison function from two XML extraction results.
*
* @param env - JNI environment pointer
* @param xml1 - first XML string
* @param xml2 - second xml string
* @param parameters - additional extraction parameters array
* @return jstring - floating point number (double) converted to jstring as result of comparison (distance) */
JNIEXPORT jstring JNICALL Java_com_mpeg7_bridge_libmpeg7_calculateDistance
(JNIEnv * env, jobject, jstring xml1, jstring xml2, jobjectArray parameters) {
    /* ---------------- 1. Declare C data for distance calculation: ----------------- */

    const char *  xml1_str = nullptr; // xml1
    const char *  xml2_str = nullptr; // xml2
    const char ** params   = nullptr; // parameters

    /* ---------------- 2. Check arguments: ------------------------------  */

    if (!xml1 || !xml2) {
        return createJniMessage(env, message(JNI_XML_NULL));
    }

    if (!parameters) {
        return createJniMessage(env, message(JNI_PARAMS_NULL));
    }

    /* ---------------- 3. Parse JNI data to C data: ---------------------- */

    // Parse xmls
    xml1_str = env->GetStringUTFChars(xml1, (jboolean *) nullptr);
    xml2_str = env->GetStringUTFChars(xml2, (jboolean *) nullptr);

    if (xml1_str == nullptr || xml2_str == nullptr) {
        return createJniMessage(env, message(JNI_XML_NULL));
    }

    // Parse parameters
    try {
        params = getParameters(env, parameters);
    }
    catch (ErrorCode exception) {
        return createJniMessage(env, message(exception));
    }

    if (params == nullptr) {
        return createJniMessage(env, message(JNI_PARAMS_NULL));
    }

    /* ---------------- 4. Get distance: -------------------------------------- */
    const char * result = getDistance(xml1_str, xml2_str, params);

    /* ---------------- 5. Cleanup: -------------------------------------- */
    // a) Release xml1:
    env->ReleaseStringUTFChars(xml1, xml1_str);

    // b) Release xml2:
    env->ReleaseStringUTFChars(xml2, xml2_str);

    // c) Release parameters - from JNI env and from C
    const int param_len = env->GetArrayLength(parameters);

    // from env
    if (param_len != 0) {
        for (int i = 0; i < param_len; i++) {
            // Releases each array element:
            env->ReleaseStringUTFChars(static_cast<jstring>(env->GetObjectArrayElement(parameters, i)), params[i]);
        }
    }
    // from C
    delete[] params;

    /* ---------------- 6. Return distance result --------------------- */
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

    // 1. Parameters size = 0
    if (stringCount == 0) {
        params = new const char * [stringCount + 1];
        params[0] = nullptr;
    }
    // 2, 3 - Parameters size = 1
    else if (stringCount == 1) {
        const auto string = static_cast<jstring>(env->GetObjectArrayElement(parameters, 0));

        // 2. Single value NULL
        if (string == nullptr) {
            params = new const char * [stringCount];
            params[0] = nullptr;
        }
        // 3. Single value != NULL
        else {
            params = new const char *[stringCount + 1];
            params[0] = env->GetStringUTFChars(string, (jboolean *) nullptr);
            params[1] = nullptr;
        }
    }
    // 4. Paramters size > 1
    else {
        params = new const char * [stringCount + 1];

        for (int i = 0; i < stringCount; i++) {
            const auto string = static_cast<jstring>(env->GetObjectArrayElement(parameters, i));

            // Check if NULL
            if (string == nullptr) {
                delete[] params; // Cleanup to prevent memory leak
                throw JNI_PARAMS_NULL;
            }
            // Otherwise get value
            params[i] = env->GetStringUTFChars(string, (jboolean *) nullptr);
        }
        params[stringCount] = nullptr;
    }

    return params;
}

unsigned char * getImageBuffer(JNIEnv * env, jbyteArray data, int size) {
    // Get pointer to JNI array
    jbyte * jniBufferPtr = env->GetByteArrayElements(data, (jboolean *) nullptr);

    // Copy values to C array
    const auto imgBuffer = new unsigned char[size];

    int i = 0;

    for (int i = 0; i < size; i++) {
        imgBuffer[i] = jniBufferPtr[i];
    }

    // Release JNI array
    env->ReleaseByteArrayElements(data, jniBufferPtr, 0);

    return imgBuffer;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */