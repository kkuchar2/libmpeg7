#pragma once

#include "TOOLS/ErrorCode.h"
#include "DESCRIPTORS/DescriptorType.h"

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

const char * message(int error);

#if defined(_WIN32) || defined(_WIN64)
    #define MODULE_API __declspec(dllexport)
#else
    #define MODULE_API
#endif

extern "C" {
    MODULE_API const char * extractDescriptor (DescriptorType descriptorType, const char * imgURL, const char ** params);
    MODULE_API const char * extractDescriptorFromData (DescriptorType descriptorType, unsigned char * data, int size, const char ** params);
    MODULE_API const char * getDistance (const char * xml1, const char * xml2, const char ** params);
    MODULE_API void freeResultPointer(char * ptr);
}