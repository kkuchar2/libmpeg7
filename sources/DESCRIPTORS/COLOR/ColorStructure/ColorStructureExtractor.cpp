#include "ColorStructureExtractor.h"

ColorStructureExtractor::ColorStructureExtractor(): targetSize(0) {
    descriptor = new ColorStructure();
}

Descriptor * ColorStructureExtractor::extract(Image & image, const char ** params) {
    // Load parameters
    try {
        descriptor->loadParameters(params);
    }
    catch (ErrorCode exception) {
        throw exception;
    }

    // Get image information
    const int imageWidth  = image.getWidth();
    const int imageHeight = image.getHeight();
    const int imageSize   = image.getSize();

    /* Initial extraction always to Base size (XM) 
    Size of m_Data initially will be 256, and all elements will be set to 0 (KK) */
    if (!descriptor->SetSize(BASE_QUANT_SPACE)) {
        for (int i = 0; i < BASE_QUANT_SPACE; i++) {
            descriptor->SetElement(i, 0);
        }
    }

    // Image data, alpha channel is not needed (used later)
    const unsigned char * channel_R = image.getChannel_R();
    const unsigned char * channel_G = image.getChannel_G();
    const unsigned char * channel_B = image.getChannel_B();

    // Convert color space and quantize 
    int i;
    int H, S, D;

    // Buffer for image data after RGB to HMMD conversion and quantization
    quantizedImageBuffer = new unsigned long[imageSize];

    for (i = 0; i < imageSize; i++) {
        RGB2HMMD(channel_R[i], channel_G[i], channel_B[i], H, S, D);
        quantizedImageBuffer[i] = QuantHMMD(H, S, D, BASE_QUANT_SPACE_INDEX);
    }

    delete[] channel_R;
    delete[] channel_G;
    delete[] channel_B;

    // Extract histogram of color structure
    unsigned long col, row, index;
    unsigned long Norm;
    unsigned long modifiedImageWidth, modifiedImageHeight, moduloSlideHeight;

    unsigned long * slideHist;
    unsigned long * pAdd;
    unsigned long * pDel;
    unsigned long * pAddStop;
    unsigned char * pAddAlpha;
    unsigned char * pDelAlpha;
    
    // Buffer for alpha channel (if present)
    unsigned char * alphaChannelBuffer = image.getTransparencyPresent() ? image.getChannel_A() : nullptr;

    // Determine working dimensions
    const double logArea = log(imageWidth * imageHeight) / log(2.);
    int scalePower = static_cast<int>(floor(0.5 * logArea - 8 + 0.5));
    scalePower     = std::max(0, scalePower);

    const unsigned long subSample   = 1 << scalePower;
    const unsigned long slideWidth  = 8 * subSample;
    const unsigned long slideHeight = 8 * subSample;

    modifiedImageWidth  = imageWidth  - (slideWidth - 1);
    modifiedImageHeight = imageHeight - (slideHeight - 1);

    /* (XM) How many actual rows from the first row of a slideHist 
    to the first row of the next slideHist. 
    If slideHeight is not a multiple of subSample, and n is on a subSample, 
    then n + slideHeight won't be on a subSample */

    if (slideHeight % subSample == 0) {
        moduloSlideHeight = slideHeight;
    }
    else {
        moduloSlideHeight = slideHeight + (subSample - slideHeight % subSample);
    }

    // Allocate local sliding window histogram
    slideHist = new unsigned long[BASE_QUANT_SPACE];

    if (!slideHist) {
        throw COL_STRUCT_SLIDE_HISTOGRAM_ALLOCATION_FAILED;
    }

    // Loop through columns
    for (col = 0; col < modifiedImageWidth; col += subSample) {
        // Reset and fill in the first (top of image) full sliding window histograms
        memset(slideHist, 0, BASE_QUANT_SPACE * sizeof(unsigned long));

        for (row = 0; row < slideHeight; row += subSample) {
            pAdd      = & quantizedImageBuffer[row * imageWidth + col];
            pAddAlpha = & alphaChannelBuffer [row * imageWidth + col];
            pAddStop  = pAdd + slideWidth;

            for (; pAdd < pAddStop; pAdd += subSample, pAddAlpha += subSample) {
                if (!alphaChannelBuffer || *pAddAlpha) {
                    slideHist[*pAdd]++;
                }
            }
        }

        // Update histogram from first sliding window histograms
        for (index = 0; index < BASE_QUANT_SPACE; index++) {
            if (slideHist[index]) {
               descriptor->SetElement(index, descriptor->GetElement(index) + 1);
            }
        }

        // Slide the window down the rest of the rows
        for (row = subSample; row < modifiedImageHeight; row += subSample) {
            pDel      = & quantizedImageBuffer[(row - subSample)                     * imageWidth + col];
            pDelAlpha = & alphaChannelBuffer [(row - subSample)				     * imageWidth + col];
            pAdd      = & quantizedImageBuffer[(row + moduloSlideHeight - subSample) * imageWidth + col];
            pAddAlpha = & alphaChannelBuffer [(row + moduloSlideHeight - subSample) * imageWidth + col];

            pAddStop  = pAdd + slideWidth;

            for (; pAdd < pAddStop; pDel += subSample, pAdd += subSample,
                pDelAlpha += subSample, pAddAlpha += subSample) {

                if (!alphaChannelBuffer || *pDelAlpha) {
                    slideHist[*pDel]--;
                }
                if (!alphaChannelBuffer || *pAddAlpha) {
                    slideHist[*pAdd]++;
                }
            }

            // Update histogram from sliding window histogram
            for (index = 0; index < BASE_QUANT_SPACE; index++) {
                if (slideHist[index]) {
                    descriptor->SetElement(index, descriptor->GetElement(index) + 1);
                }
            }
        }
    }

    // Free memory
    delete[] slideHist;
    delete[] quantizedImageBuffer;

    Norm = ((imageHeight - slideHeight) / subSample + 1) * ((imageWidth - slideWidth) / subSample + 1);

    // Requantize color space to Target size
    try {
        UnifyBins(Norm, descriptor->GetTargetSize());
    }
    catch (ErrorCode exception) {
        throw exception;
    }

    // Quantize the Bin Amplitude
    try {
        QuantAmplNonLinear(Norm);
    }
    catch (ErrorCode exception) {
        throw exception;
    }

    return descriptor;
}

void ColorStructureExtractor::RGB2HMMD(const int R, const int G, const int B, int & H, int & S, int & D) {
    int max, min;
    float hue;

    max = R;

    if (max < G) {
        max = G;
    }
    if (max < B) {
        max = B;
    }

    min = R;

    if (min > G) {
        min = G;
    }
    if (min > B) {
        min = B;
    }

    if (max == min) { // ( R == G == B ) -> exactly gray (XM)
        hue = -1; //hue will be undefinied (XM)
    }

    else { // Hue
        if (R == max) {
            hue = ((G - B) / static_cast<float>(max - min));
        }

        else if (G == max) {
            hue = static_cast<float>((2.0 + (B - R) / static_cast<float>(max - min)));
        }

        else if (B == max) {
            hue = static_cast<float>((4.0 + (R - G) / static_cast<float>(max - min)));
        }

        hue *= 60;
        if (hue < 0.0) {
            hue += 360;
        }
    }

    H = static_cast<long>(hue + 0.5);				  //range [0,360]
    S = static_cast<long>((max + min) / 2.0 + 0.5); //range [0,255]
    D = static_cast<long>(max - min + 0.5);		  //range [0,255]
}

int ColorStructureExtractor::QuantHMMD(const int H, const int S, const int D, const int N) {
    /* colorQuantTable was initialized with zeros before, so it is not used
    I'll leave it here in case of future usage (KK) */
    if (colorQuantTable[N]) {
        return colorQuantTable[N][(H << 16) + (S << 8) + D];
    }

    /* (XM)
    Note: lower threshold boundary is inclusive, 
    i.e. diffThresh[..][m] <= (D of subspace m) < diffThresh[..][m+1] */

    /* Quantize the Difference component, find the Subspace */
    int iSub = 0;

    while (diffThresh[N][iSub + 1] <= D) {
        iSub++;
    }

    /* Quantize the Hue component */
    int Hindex = static_cast<int>((H / 360.0) * nHueLevels[N][iSub]);

    if (H == 360) {
        Hindex = 0;
    }

    /* Quantize the Sum component
    The min value of Sum in a subspace is 0.5 * diffThresh (see HMMD slice) */
    int Sindex = static_cast<int>(floor((S - 0.5 * diffThresh[N][iSub]) * nSumLevels[N][iSub] / (255 - diffThresh[N][iSub])));

    if (Sindex >= nSumLevels[N][iSub]) {
        Sindex = nSumLevels[N][iSub] - 1;
    }

    /*  (XM)
    The following quantization of Sum is more uniform and doesn't require the bounds check
    int Sindex = 
    (int)floor((S - 0.5 * diffThresh[N][iSub]) * nSumLevels[N][iSub] / (256 - diffThresh[N][iSub]));*/
    return nCumLevels[N][iSub] + Hindex*nSumLevels[N][iSub] + Sindex;
}

int ColorStructureExtractor::UnifyBins(const unsigned long Norm, const int targetSize) {
    /* (KK)
    Method is quantizing all coefficients (initially of default size 256) 
    to target size specified by user (e.g. 32) */

    int iTargBin; // target coeff array
    int iOrigBin; // original coeff array

    const int nOrigSize = descriptor->GetSize(); // this original size is 256

    // If target size is original size (so 256) - there is nothing to do
    if (targetSize == nOrigSize) {
        return 0;
    }

    // Otherwise prepare new array
    const auto pBin = new double[targetSize];

    // Fill it with zeros
    for (iTargBin = 0; iTargBin < targetSize; iTargBin++) {
        pBin[iTargBin] = 0.0;
    }

    /* Transform original array to smaller target array */
    int iTargSpace, iOrigSpace;
    try {
        iTargSpace = GetColorQuantSpace(targetSize);
        iOrigSpace = GetColorQuantSpace(nOrigSize);
    }
    catch (ErrorCode exception) {
        delete[] pBin; // Cleanup (KK)
        throw exception;
    }

    // Unify
    for (iOrigBin = 0; iOrigBin < nOrigSize; iOrigBin++) {
        try {
            iTargBin = TransformBinIndex(iOrigBin, iOrigSpace, iTargSpace);
        }
        // Catch any exception (KK)
        catch (ErrorCode exception) { 
            // Memory allocation failed, cleanup first
            delete[] pBin;
            // Throw exception one level higher
            throw exception;
        }
        pBin[iTargBin] += descriptor->GetElement(iOrigBin);
    }

    // Resize the descriptor
    descriptor->SetSize(descriptor->GetTargetSize());

    // Clip and insert
    for (iTargBin = 0; iTargBin < targetSize; iTargBin++) {
        if (pBin[iTargBin] > Norm) {
            pBin[iTargBin] = Norm;
        }

       descriptor->SetElement(iTargBin, static_cast<int>(pBin[iTargBin]));
    }

    delete[] pBin;
    return 0;
}

int ColorStructureExtractor::TransformBinIndex(const int iOrig, const int iOrigColorQuantSpace, const int iNewColorQuantSpace) {
    // Build transform table if not already present
    if (!colorQuantTransform[iOrigColorQuantSpace][iNewColorQuantSpace]) {
        BuildTransformTable(iOrigColorQuantSpace, iNewColorQuantSpace);
    }

    if (!colorQuantTransform[iOrigColorQuantSpace][iNewColorQuantSpace]) {
        throw COL_STRUCT_BUILD_TRANSFORM_TABLE_ERROR; // (KK)
    }

    return colorQuantTransform[iOrigColorQuantSpace][iNewColorQuantSpace][iOrig];
}

int ColorStructureExtractor::BuildTransformTable(const int iOrigColorQuantSpace, const int iNewColorQuantSpace) {
    const int maxMatchTest = 27; // Allow deviation +/- 1 in 3D (XM)
    int iMatch[maxMatchTest], nMatch[maxMatchTest], nUniqueMatch = 0;
    int iOrig, iNew;
    int H, S, D;
    int iTest;

    // Allocate
    const auto tbl = new unsigned char[GetBinSize(iOrigColorQuantSpace)];

    if (!tbl) {
        throw COL_STRUCT_BUILD_TRANSFORM_TABLE_ERROR; // (KK)
    }

    // Loop through originating cells
    for (iOrig = 0; iOrig < GetBinSize(iOrigColorQuantSpace); iOrig++) {

        // Loop through 24 bit HMMD values
        for (H = 0; H < 361; H++) {
            for (D = 0; D < 256; D++) {

                const int beginS = (D + 1) >> 1;
                const int endS = 256 - (D >> 1);

                for (S = beginS; S < endS; S++) {
                    // Check match and get new space
                    if (QuantHMMD(H, S, D, iOrigColorQuantSpace) == iOrig) {

                        iNew = QuantHMMD(H, S, D, iNewColorQuantSpace);

                        // Find result in Match array
                        for (iTest = 0; iTest < nUniqueMatch; iTest++) {
                            if (iMatch[iTest] == iNew)
                                break;
                        }

                        // Accumulate stats on best match
                        if (iTest == nUniqueMatch) {
                            // (KK) Replace assert(nUniqueMatch < maxMatchTest); with exception
                            if (nUniqueMatch >= maxMatchTest) {
                                delete[] tbl;
                                throw COL_STRUCT_BUILD_TRANSFORM_TABLE_ERROR;
                            }

                            iMatch[nUniqueMatch] = iNew;
                            nMatch[nUniqueMatch] = 1;
                            nUniqueMatch++;
                        }
                        else {
                            nMatch[iTest] ++;
                        }
                    }
                }
            }
        }

        // Report on inexact matches
        if (nUniqueMatch > 1) {
            int nTotal = 0;
            for (iTest = 0; iTest < nUniqueMatch; iTest++) {
                nTotal += nMatch[iTest];
            }
        }
        if (!nUniqueMatch) {   
            delete[] tbl;  // Cleanup
            throw COL_STRUCT_BUILD_TRANSFORM_TABLE_ERROR; // (KK)
        }

        // Select the best match
        int iBest = -1, nBest = 0;
        for (iTest = 0; iTest < nUniqueMatch; iTest++) {
            if (nMatch[iTest] > nBest) {
                nBest = nMatch[iTest];
                iBest = iMatch[iTest];
            }
        }

        // Update table and reset stats
        tbl[iOrig] = iBest;
        nUniqueMatch = 0;

    } // Loop through orig indices

    int nPrec = GetBinSize(iNewColorQuantSpace) > 100 ? 3 : 2;

    if (colorQuantTransform[iOrigColorQuantSpace][iNewColorQuantSpace]) {
        delete colorQuantTransform[iOrigColorQuantSpace][iNewColorQuantSpace];
    }

    colorQuantTransform[iOrigColorQuantSpace][iNewColorQuantSpace] = tbl;
    return 1;
}

int ColorStructureExtractor::QuantAmplNonLinear(const unsigned long Norm) {
    unsigned long iBin, TotalNoOfBins, iQuant;
    const int nAmplLinearRegions = sizeof(nAmplLevels) / sizeof(nAmplLevels[0]);
    int nTotalLevels = 0;

    // Calculate total levels
    for (iQuant = 0; iQuant < nAmplLinearRegions; iQuant++) {
        nTotalLevels += nAmplLevels[iQuant];
    }

    // Get size
    TotalNoOfBins = descriptor->GetSize();

    // Loop through bins
    for (iBin = 0; iBin < TotalNoOfBins; iBin++) {
        // Get bin amplitude
        double val = descriptor->GetElement(iBin);

        // Normalize
        val /= Norm;
        /* (KK) replace  'assert(val >= 0.0); assert(val <= 1.0);' with exception: */
        if (val < 0.0 || val > 1.0) {
            // Nothing to clean
            throw COL_STRUCT_QUANTAMPL_NOLINEAR_NORMALIZE_ERROR;
        }

        // Find quantization boundary and base value
        int quantValue = 0;
        for (iQuant = 0; iQuant + 1 < nAmplLinearRegions; iQuant++) {
            if (val < amplThresh[iQuant + 1]) {
                break;
            }
            quantValue += nAmplLevels[iQuant];
        }

        // Quantize
        const double nextThresh = (iQuant + 1 < nAmplLinearRegions) ? amplThresh[iQuant + 1] : 1.0;

        val = floor(quantValue +(val - amplThresh[iQuant]) * (nAmplLevels[iQuant] / (nextThresh - amplThresh[iQuant])));

        // Limit (and alert), one bin contains all of histogram
        if (val == nTotalLevels) {
            /*
            (XM)
            Possible (though rare) case
            std::cerr << "Degenerate case, histogram bin " << iBin << " has value " << 
            GetHistogramDescriptorInterface()->GetElement(iBin) << " of " << Norm << " Norm" << std::endl; */
            val = nTotalLevels - 1;
        }
        /* (KK) replace 'assert(val >= 0.0); assert(val < nTotalLevels);' with exception */
        if (val < 0.0 || val >= nTotalLevels) {
            // Nothing to clean
            throw COL_STRUCT_QUANTAMPL_NONLINEAR_TOTAL_LEVELS;
        }

        // Set value into histogram
        descriptor->SetElement(iBin, static_cast<int>(val));
    }
    return 0;
}

int ColorStructureExtractor::GetColorQuantSpace(const int size) {
    if (size == 256) {
        return 3;
    }
    else if (size == 128) {
        return 2;
    }
    else if (size == 64) {
        return 1;
    }
    else if (size == 32) {
        return 0;
    }
    else {
        throw COL_STUCT_GETCOLORQSPACE_ERROR;
    }
}

int ColorStructureExtractor::GetBinSize(const int iColorQuantSpace) {
    if (iColorQuantSpace <= 3 && iColorQuantSpace >= 0) {
        return 32 << iColorQuantSpace;
    }
    else {
        // (KK) replaced 'assert("Out of bounds GetBinSize")' with exception
        throw COL_STRUCT_GETBINSIZE_ERROR;
    }
}

ColorStructureExtractor::~ColorStructureExtractor() {
    delete descriptor;
}

const unsigned char ColorStructureExtractor::cqt256_128[256] = {
    0,   1,   2,   3,   0,   1,   2,   3,   4,   5,   6,   7,   4,   5,   6,   7,
    8,   9,   10,  11,   8,   9,  10,  11,  12,  13,  14,  15,  12,  13,  14,  15,
    16,  17,  18,  19,  16,  17,  18,  19,  20,  21,  22,  23,  20,  21,  22,  23,
    24,  25,  26,  27,  24,  25,  26,  27,  28,  29,  30,  31,  28,  29,  30,  31,
    32,  33,  34,  35,  32,  33,  34,  35,  36,  37,  38,  39,  36,  37,  38,  39,
    40,  41,  42,  43,  40,  41,  42,  43,  44,  45,  46,  47,  44,  45,  46,  47,
    48,  49,  50,  51,  48,  49,  50,  51,  52,  53,  54,  55,  52,  53,  54,  55,
    56,  57,  58,  59,  56,  57,  58,  59,  60,  61,  62,  63,  60,  61,  62,  63,
    64,  65,  66,  67,  64,  65,  66,  67,  68,  69,  70,  71,  68,  69,  70,  71,
    72,  73,  74,  75,  72,  73,  74,  75,  76,  77,  78,  79,  76,  77,  78,  79,
    80,  81,  82,  83,  80,  81,  82,  83,  84,  85,  86,  87,  84,  85,  86,  87,
    88,  89,  90,  91,  88,  89,  90,  91,  92,  93,  94,  95,  92,  93,  94,  95,
    96,  96,  97,  97,  98,  98,  99,  99,  100, 100, 101, 101, 102, 102, 103, 103,
    104, 104, 105, 105, 106, 106, 107, 107, 108, 108, 109, 109, 110, 110, 111, 111,
    112, 112, 113, 113, 114, 114, 115, 115, 116, 116, 117, 117, 118, 118, 119, 119,
    120, 120, 121, 121, 122, 122, 123, 123, 124, 124, 125, 125, 126, 126, 127, 127 };

const unsigned char ColorStructureExtractor::cqt256_064[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,
    2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,
    4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,  5,
    6,  6,  6,  6,  6,  6,  6,  6,  7,  7,  7,  7,  7,  7,  7,  7,
    8,  8,  9,  9,  8,  8,  9,  9,  10, 10, 11, 11, 10, 10, 11, 11,
    12, 12, 13, 13, 12, 12, 13, 13, 14, 14, 15, 15, 14, 14, 15, 15,
    16, 16, 17, 17, 16, 16, 17, 17, 18, 18, 19, 19, 18, 18, 19, 19,
    20, 20, 21, 21, 20, 20, 21, 21, 22, 22, 23, 23, 22, 22, 23, 23,
    24, 25, 26, 27, 24, 25, 26, 27, 24, 25, 26, 27, 24, 25, 26, 27,
    28, 29, 30, 31, 28, 29, 30, 31, 28, 29, 30, 31, 28, 29, 30, 31,
    32, 33, 34, 35, 32, 33, 34, 35, 32, 33, 34, 35, 32, 33, 34, 35,
    36, 37, 38, 39, 36, 37, 38, 39, 36, 37, 38, 39, 36, 37, 38, 39,
    40, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47,
    48, 48, 49, 49, 50, 50, 51, 51, 52, 52, 53, 53, 54, 54, 55, 55,
    56, 56, 56, 56, 57, 57, 57, 57, 58, 58, 58, 58, 59, 59, 59, 59,
    60, 60, 60, 60, 61, 61, 61, 61, 62, 62, 62, 62, 63, 63, 63, 63 };

const unsigned char ColorStructureExtractor::cqt256_032[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
    8,  9, 10, 11,  8,  9,  10, 11, 8,  9,  10, 11, 8,  9,  10, 11,
    12, 13, 14, 15, 12, 13, 14, 15, 12, 13, 14, 15, 12, 13, 14, 15,
    16, 17, 18, 19, 16, 17, 18, 19, 16, 17, 18, 19, 16, 17, 18, 19,
    20, 21, 22, 23, 20, 21, 22, 23, 20, 21, 22, 23, 20, 21, 22, 23,
    8,  8,  9,  9,  10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15,
    16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23,
    24, 24, 24, 24, 25, 25, 25, 25, 26, 26, 26, 26, 27, 27, 27, 27,
    28, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 31, 31, 31, 31 };

const unsigned char ColorStructureExtractor::cqt128_064[128] = {
    0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3,
    4,  4,  4,  4,  5,  5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,
    8,  8,  9,  9,  10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15,
    16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23,
    24, 25, 26, 27, 24, 25, 26, 27, 28, 29, 30, 31, 28, 29, 30, 31,
    32, 33, 34, 35, 32, 33, 34, 35, 36, 37, 38, 39, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
    56, 56, 57, 57, 58, 58, 59, 59, 60, 60, 61, 61, 62, 62, 63, 63 };

const unsigned char ColorStructureExtractor::cqt128_032[128] = {
    0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,
    2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,
    4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,  5,
    6,  6,  6,  6,  6,  6,  6,  6,  7,  7,  7,  7,  7,  7,  7,  7,
    8,  9,  10, 11, 8,  9,  10, 11, 12, 13, 14, 15, 12, 13, 14, 15,
    16, 17, 18, 19, 16, 17, 18, 19, 20, 21, 22, 23, 20, 21, 22, 23,
    8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31 };

const unsigned char ColorStructureExtractor::cqt064_032[64] = {
    0,  0,  1,  1,  2,  2,  3,  3,  4,  4,  4,  4,  5,  5,  5,  5,
    6,  6,  6,  6,  7,  7,  7,  7,  8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 };

const int ColorStructureExtractor::diffThresh[NUM_COLOR_QUANT_SPACE][MAX_SUB_SPACE + 1] = {
    { 0, 6, 60, 110, 256, -1 },
    { 0, 6, 20, 60,  110, 256 },
    { 0, 6, 20, 60,  110, 256 },
    { 0, 6 ,20, 60,  110, 256 } };

const int ColorStructureExtractor::nHueLevels[NUM_COLOR_QUANT_SPACE][MAX_SUB_SPACE] = {
    { 1, 4, 4,  4,  0 },
    { 1, 4, 4,  8,  8 },
    { 1, 4, 8,  8,  8 },
    { 1, 4, 16, 16, 16 } };

const int ColorStructureExtractor::nSumLevels[NUM_COLOR_QUANT_SPACE][MAX_SUB_SPACE] = {
    { 8,  4, 1, 1, 0 },
    { 8,  4, 4, 2, 1 },
    { 16, 4, 4, 4, 4 },
    { 32, 8, 4, 4, 4 } };

/* (XM)
The following could be derived implicitly from nHue and nSum
Reverse these to order quantization from inside to outside */
const int ColorStructureExtractor::nCumLevels[NUM_COLOR_QUANT_SPACE][MAX_SUB_SPACE] = {
    { 24,  8,   4,   0,  0 },
    { 56,  40,  24,  8,  0 },
    { 112, 96,  64,  32, 0 },
    { 224, 192, 128, 64, 0 } };

const unsigned char * ColorStructureExtractor::colorQuantTable[NUM_COLOR_QUANT_SPACE] = { nullptr,nullptr,nullptr,nullptr };

const unsigned char * ColorStructureExtractor::colorQuantTransform[NUM_COLOR_QUANT_SPACE][NUM_COLOR_QUANT_SPACE] = {
    { nullptr,		  nullptr,		  nullptr,		  nullptr },
    { cqt064_032, nullptr,		  nullptr,		  nullptr },
    { cqt128_032, cqt128_064, nullptr,		  nullptr },
    { cqt256_032, cqt256_064, cqt256_128, nullptr } };


const double ColorStructureExtractor::amplThresh[6] = { 0.0, 0.000000000001, 0.037, 0.08, 0.195, 0.32 };

const int ColorStructureExtractor::nAmplLevels[6] = { 1, 25, 20, 35, 35, 140 };
