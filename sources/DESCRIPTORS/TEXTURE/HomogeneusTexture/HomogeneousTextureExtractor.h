/** @file  HomogeneousTextureExtractor.h
*  @brief  Homogeneous Texture class for extraction.
*  @author Krzysztof Lech Kucharski
*  @bug    No bugs detected.                            */

#pragma once

#include "../../DescriptorExtractor.h"
#include "../HomogeneusTexture/HomogeneousTexture.h"

class HomogeneousTextureExtractor : public DescriptorExtractor {
    private:
        HomogeneousTexture * descriptor = nullptr;

        // LUT
        static const double mmax[5][6];
        static const double mmin[5][6];
        static const double dmax[5][6];
        static const double dmin[5][6];
        //

        COMPLEX * timage [1024];
        COMPLEX * inimage[512];
        COMPLEX * image  [512];

        int mean2[5][6];
        int dev2 [5][6];
        int m_dc;
        int m_std;

        double Num_pixel;

        double hdata[5][128];
        double vdata[6][180];
        double dc;
        double stdev;
        double vec [5][6];
        double dvec[5][6];

        double dcmin = 0.0;
        double dcmax = 255.0; // 2001.01.31 - yjyu@samsung.com
        
        double stdmin = 1.309462;
        double stdmax = 109.476530;

        // Arbitrary shape calculation
        void ArbitraryShape(unsigned char * aChannel, unsigned char * grayImage, int imageHeight, int imageWidth);
        void mintest(int a, int & min);
        bool maxtest(int a, int & max);

        // Criterion leads design
        void vatomdesign();
        void hatomdesign();

        // Main extraction methods
        void FeatureExtraction(unsigned char * image, int image_height, int image_width);
        void SecondLevelExtraction(unsigned char * imagedata, int image_height, int image_width);
        void RadonTransform(unsigned char(*cin)[imsize], double(*fin)[Nray], int nr, int nv);
        void Feature(double(*fin)[128], double(*vec)[6], double(*dvec)[6]);
        COMPLEX GetProjectionFromFFT(CARTESIAN cart, COMPLEX **inimage, int size2);
        
        // FFTs
        void FastFourierTransform2d(COMPLEX ** inimage, COMPLEX ** timage, int size2, int x, int y, int inc, double dx, double dy);
        void four1(COMPLEX * data1, int nn, int isign);

        // Swap
        void Swap(COMPLEX * data, int size2);

        // Final quantization
        void Quantization();


    public:
        HomogeneousTextureExtractor();
        Descriptor * extract(Image & image, const char ** params);
        ~HomogeneousTextureExtractor();
};
