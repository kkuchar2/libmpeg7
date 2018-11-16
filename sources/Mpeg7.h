/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/** @file Mpeg7.h
*  @brief Function prototypes for descriptor 
*  extraction and distance calculation.
*	
*  Additional error code returning method declaration is included.
*  
*  @author Krzysztof Lech Kucharski
*  @bug TinyXML2 throws exception/asserts, when xml
*  pointers point to existing data, but corrupted (series of thrash bytes).*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#ifndef _MPEG7_H
#define _MPEG7_H

#include "TOOLS/ErrorCode.h"
#include "DESCRIPTORS/DescriptorType.h"

// Descriptors
#include "DESCRIPTORS/COLOR/ColorLayout/ColorLayout.h"
#include "DESCRIPTORS/COLOR/ColorStructure/ColorStructure.h"
#include "DESCRIPTORS/COLOR/CTBrowsing/CTBrowsing.h"
#include "DESCRIPTORS/COLOR/DominantColor/DominantColor.h"
#include "DESCRIPTORS/COLOR/ScalableColor/ScalableColor.h"

#include "DESCRIPTORS/SHAPE/ContourShape/ContourShape.h"
#include "DESCRIPTORS/SHAPE/RegionShape/RegionShape.h"

#include "DESCRIPTORS/TEXTURE/EdgeHistogram/EdgeHistogram.h"
#include "DESCRIPTORS/TEXTURE/HomogeneusTexture/HomogeneousTexture.h"
#include "DESCRIPTORS/TEXTURE/TextureBrowsing/TextureBrowsing.h"

// Descriptor extraction
#include "DESCRIPTORS/DescriptorExtractor.h"

#include "DESCRIPTORS/COLOR/ColorLayout/ColorLayoutExtractor.h"
#include "DESCRIPTORS/COLOR/ColorStructure/ColorStructureExtractor.h"
#include "DESCRIPTORS/COLOR/CTBrowsing/CTBrowsingExtractor.h"
#include "DESCRIPTORS/COLOR/DominantColor/DominantColorExtractor.h"
#include "DESCRIPTORS/COLOR/ScalableColor/ScalableColorExtractor.h"

#include "DESCRIPTORS/SHAPE/ContourShape/ContourShapeExtractor.h"
#include "DESCRIPTORS/SHAPE/RegionShape/RegionShapeExtractor.h"

#include "DESCRIPTORS/TEXTURE/EdgeHistogram/EdgeHistogramExtractor.h"
#include "DESCRIPTORS/TEXTURE/HomogeneusTexture/HomogeneousTextureExtractor.h"
#include "DESCRIPTORS/TEXTURE/TextureBrowsing/TextureBrowsingExtractor.h"

// Descriptor distance
#include "DESCRIPTORS/DescriptorExtractor.h"

#include "DESCRIPTORS/COLOR/ColorLayout/ColorLayoutDistance.h"
#include "DESCRIPTORS/COLOR/ColorStructure/ColorStructureDistance.h"
#include "DESCRIPTORS/COLOR/CTBrowsing/CTBrowsingDistance.h"
#include "DESCRIPTORS/COLOR/DominantColor/DominantColorDistance.h"
#include "DESCRIPTORS/COLOR/ScalableColor/ScalableColorDistance.h"

#include "DESCRIPTORS/SHAPE/ContourShape/ContourShapeDistance.h"
#include "DESCRIPTORS/SHAPE/RegionShape/RegionShapeDistance.h"

#include "DESCRIPTORS/TEXTURE/EdgeHistogram/EdgeHistogramDistance.h"
#include "DESCRIPTORS/TEXTURE/HomogeneusTexture/HomogeneousTextureDistance.h"
#include "DESCRIPTORS/TEXTURE/TextureBrowsing/TextureBrowsingDistance.h"

#include <iomanip>

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/** @brief 
* Returns char * created from error
* integer code passed as argument
*
* @param error   - Error code
* @return char * - created char from error code */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
const char * message(int error);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/** @brief 
* Detects, what operating system is being used.
* On Windows __declspec(dllexport) is required
* to export methods to .dll file.
* 
* On Linux declarator is not needed.
*
* To support cross-plaform usage, MODULE_API,
* based on operating system
* decides, when additional delarator should be used
* and adds it, when necessary. */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#if defined(_WIN32) || defined(_WIN64)
    #define MODULE_API __declspec(dllexport) // Windows
#else
    #define MODULE_API                       // Linux
#endif

extern "C" {
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ extractDescriptor ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/** @brief Extracts descriptor from image at specified path location and with given parameters.
*
* First argument decides, which descriptor extraction has to be performed. 
* Supported descriptor types are definied in DescriptorType.h file.
*
* Second argument points to local image path char data.
* Image will be loaded by OpenCV, therefore path can only be local
*	
* Third argument points to parameters data used later by extraction method.
*
* Method returns const char pointer to the data of extraction result information.
* If extraction was performed properly and without any errors, result will
* point to xml data char sequence for extracted descriptor, otherwise it will point to error
* code stored in char.
*
* Any results or exceptions detected by this method will result with proper error code.
*
* @param descriptorType - Descriptor type as integer (enumeration value of supported types)
* @param imgURL		    - Path to image location
* @param params		    - Parameters used for extraction
* @return               - Pointer to result information (xml or error code) */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    MODULE_API const char * extractDescriptor (DescriptorType descriptorType, const char * imgURL, const char ** params);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ extractDescriptorFromData ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/** @brief Extracts descriptor from image file raw data with given parameters.
*
* First argument decides, which descriptor extraction has to be performed. 
* Supported descriptor types are definied in DescriptorType.h file.
*
* Second argument points to image file raw data (pointer to bytes stored in 
* unsigned char array), which can be taken from local or network image at user side.
*
* OpenCV will decode the data into proper cv::Mat object. If data is corrupted or 
* improper, library will return error code as result. (OpenCV might print also 
* interal error messages in some cases)
*
* Third argument points to parameters data used later by extraction method.
*
* Method returns const char pointer to the data of extraction result information.
* If extraction was performed properly and without any errors, result will
* point to xml data char sequence of extracted descriptor, otherwise it will point 
* to error code stored in char.
*
* Any results or exceptions detected by this method will result with proper error code.*
*
* @param descriptorType - Descriptor type as integer (enumeration value of supported types)
* @param data		    - Pointer to image raw data (bytes stored in unsigned char array)
* @param params         - Parameters used for extraction
*
* @return               - Pointer to result information (xml or error code) */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    MODULE_API const char * extractDescriptorFromData (DescriptorType descriptorType, unsigned char * data, int size, const char ** params);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ getDistance ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/** @brief Calculates distance from two xml strings with given parameters.
*
* First and second argument points to specific decriptor xml data
* after extraction.
*
* Third argument points to parameters data used later by method calculating distance
*
* Only one descriptor distance calculating mechanism in library uses parameters,
* which is HomogeneousTexture descriptor.
*
* XML parsing is done using TinyXML2 code.
*
* Method returns const char pointer to the data of distance calculation result information.
* If calculation was performed properly and without any errors, result will
* point to distance data between two descriptors (double as series of chars), 
* otherwise it will point to error code stored in char.
*
* Any results or exceptions detected by this method will result with proper error code.
*
* @param xml1   - Pointer to first descriptor's xml data
* @param xml2	 - Pointer to second descriptor's xml data
* @param params - Parameters used for distance calculation
*
* @return       - Pointer to result information (distance or error code) */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    MODULE_API const char * getDistance (const char * xml1, const char * xml2, const char ** params);
     
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ freeResultPointer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/** @brief Frees char pointer from C memory
*
* While using library from ctypes in Python, user must use that function to release
* results coming out (xml/distance/error code) in order to prevent memory leak. 
* When pympeg7 module is used, it is performed automaticly.
*
* Then again: when pympeg7 module is not used, IT IS UP TO USER TO CALL THIS FUNCTION. :)
*
* @param ptr - Pointer to library result */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    MODULE_API void freeResultPointer(char * ptr);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
}

#endif /* _MPEG7_H */