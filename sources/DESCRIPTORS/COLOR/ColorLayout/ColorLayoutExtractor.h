/** @file   ColorLayoutExtractor.h
*  @brief  Color Layout class for extraction.
*  @author Krzysztof Lech Kucharski
*  @bug    No bugs detected.                 */

#pragma once

#include "../../DescriptorExtractor.h"
#include "../ColorLayout/ColorLayout.h"

class ColorLayoutExtractor : public DescriptorExtractor {
	private:
        ColorLayout * descriptor = nullptr;
        
        // LUT
		static const double DCT_Coefficients[8][8];
		static const unsigned char ZigZagScanCoefficients[64];

        // Ico creation from representative colors
		void CreateSmallImage(Image &image, short small_img[3][64]);

        // DCT
		void FastDiscreteCosineTransform(short * block);

        // Coefficients quantization
		int YDCQuantization(int i);
		int CDCQuantization(int i);
		int ACQuantization(int i);

	public:
		ColorLayoutExtractor();
		Descriptor * extract(Image & image, const char ** params);
        ~ColorLayoutExtractor();
};
