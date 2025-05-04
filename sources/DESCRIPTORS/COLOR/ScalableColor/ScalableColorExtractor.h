/** @file   ScalableColorExtractor.h
 *  @brief  Scalable Color class for extraction.
 *  @author Krzysztof Lech Kucharski
 *  @bug    No bugs detected.                            */

#ifndef _SCALABLE_COLOR_EXTRACTOR_H 
#define _SCALABLE_COLOR_EXTRACTOR_H 

#include "../../DescriptorExtractor.h"
#include "../ScalableColor/ScalableColor.h"

class ScalableColorExtractor : public DescriptorExtractor {
    private:
        ScalableColor * descriptor = nullptr;

        // RGB -> HSV and with quantization
        int * rgb2hsv(int r, int g, int b, int hue_quant, int sat_quant, int val_quant);

        // LUT
        static const double H[16][16];
        static const int    tabelle[5][255];
        static const int    sorttab[256];
        static const int    scalableColorQuantValues[256][3];
    public:
	    ScalableColorExtractor();
	    Descriptor * extract(Image & image, const char ** params);
        ~ScalableColorExtractor();
};

#endif