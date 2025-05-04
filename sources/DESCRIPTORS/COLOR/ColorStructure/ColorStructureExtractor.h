/** @file   ColorStructureExtractor.h
 *  @brief  Color Structure class for extraction.
 *  @author Krzysztof Lech Kucharski
 *  @bug    No bugs detected.                    */

#pragma once

#define	BASE_QUANT_SPACE       256
#define	BASE_QUANT_SPACE_INDEX 3
#define NUM_COLOR_QUANT_SPACE  4
#define	MAX_SUB_SPACE          5

#include "../../DescriptorExtractor.h"
#include "../ColorStructure/ColorStructure.h"

class ColorStructureExtractor : public DescriptorExtractor {
	private:
        ColorStructure * descriptor = nullptr;

        // Final buffer for quantized coefficients
        unsigned long * quantizedImageBuffer = nullptr;

        // Target coefficients size
        int targetSize;

        // Color Transformations
        void RGB2HMMD(int R, int G, int B, int & H, int & S, int & D);
        int QuantHMMD(int H, int S, int D, int N);

        int UnifyBins(unsigned long Norm, int targetSize);
        int GetColorQuantSpace(int size);
        int TransformBinIndex(int iOrig, int iOrigColorQuantSpace, int iNewColorQuantSpace);
        int BuildTransformTable(int iOrigColorQuantSpace, int iNewColorQuantSpace);
        int GetBinSize(int iColorQuantSpace);
        int QuantAmplNonLinear(unsigned long Norm);

        // LUT
        static const unsigned char cqt256_128[256];
        static const unsigned char cqt256_064[256];
        static const unsigned char cqt256_032[256];
        static const unsigned char cqt128_064[128];
        static const unsigned char cqt128_032[128];
        static const unsigned char cqt064_032[64];

        static const int diffThresh[NUM_COLOR_QUANT_SPACE][MAX_SUB_SPACE + 1];
        static const int nHueLevels[NUM_COLOR_QUANT_SPACE][MAX_SUB_SPACE];
        static const int nSumLevels[NUM_COLOR_QUANT_SPACE][MAX_SUB_SPACE];
        static const int nCumLevels[NUM_COLOR_QUANT_SPACE][MAX_SUB_SPACE];

        static const unsigned char * colorQuantTable[NUM_COLOR_QUANT_SPACE];
        static const unsigned char * colorQuantTransform[NUM_COLOR_QUANT_SPACE][NUM_COLOR_QUANT_SPACE];
        static const double amplThresh[6];
        static const int nAmplLevels[6];
        
	public:
		ColorStructureExtractor();
		Descriptor * extract(Image & image, const char ** params);
        ~ColorStructureExtractor();
};
