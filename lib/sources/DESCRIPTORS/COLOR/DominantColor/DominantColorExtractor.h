/** @file   DominantColorExtractor.h
*  @brief  Dominant Color class for extraction.
*  @author Krzysztof Lech Kucharski
*  @bug    No bugs detected.                            */

#ifndef _DOMINANTCOLOR_EXTRACTOR_H 
#define _DOMINANTCOLOR_EXTRACTOR_H 

#define MINIMUM_DISTORTION_CHANGE       0.01 // 1% minimum distortion condition
#define SPLIT_MINIMUM_DISTORTION_CHANGE 0.02 // 2% minimum distortion condition for cluster split 
#define SPLITTING_FACTOR				0.10 // factor variable used for splitting

#define SC_BIT	 5
#define DSTMIN   255.0

#define VARTHR   50.0

#define sqr(x) ((x)*(x))

#include "../../DescriptorExtractor.h"
#include "../DominantColor/DominantColor.h"

class DominantColorExtractor : public DescriptorExtractor {
	private:
        DominantColor * descriptor = NULL;

        float  * dominantColorWeights    = NULL;
        float ** dominantColorCentroids  = NULL;
        float ** dominantColorsVariances = NULL;

        float * LUV = NULL;

        int currentColorNumber;

        // LUT (custom for rgb->srgb)
        static const double rgb_pow_table[256];

        // Color conversion
        void rgb2xyz(Image & image, double * XYZ);
        void rgb2luv(Image & image, float  * LUV);
        void xyz2luv(double * XYZ, float * LUV, int size);
        void luv2rgb(int * RGB, float *LUV, int size);
        void rgb2yuv(int r, int g, int b, int & y, int & u, int & v);

        // Clustering
        double AssignPixelsToClusters(int * closest, float * imageData, int imageSize, unsigned char * quantImageAlpha);

        // Centroid calculation
        void RecalculateCentroids(int * closest, float * imageData, int imageSize, unsigned char * quantImageAlpha);

        // Splitting color clusters
        void Split(int * closest, float * imageData, int imageSize, unsigned char *quantImageAlpha, double factor);

        void CalculateVariances(int * closest, float * imageData, int imageSize, unsigned char * quantImageAlpha);

        // Merging  using agglomerative clustering method
        void Agglom(double distthr);

        // Spatial Coherency calculation
        int GetSpatialCoherency(float * ColorData, int dim, int N, float ** col_float, 
                                unsigned char * quantImageAlpha, int imageWidth, int imageHeight);

        double GetCoherencyWithColorAllow(float * ColorData, int dim, bool * IVisit,
                                          float l, float u, float v, float Allow,
                                          int NeighborRange, unsigned int * OUTPUT_Corres_Pixel_Count,
                                          int imageWidth, int imageHeight);
        // Quantization
        int QuantizeSC(double sc);
	public:
		DominantColorExtractor();
		Descriptor * extract(Image & image, const char ** params);
        ~DominantColorExtractor();
};
#endif