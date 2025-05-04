/** @file  RegionShapeExtractor.h
 *  @brief  Region Shape class for extraction.
 *  @author Krzysztof Lech Kucharski
 *  @bug    No bugs detected. */

#ifndef _REGION_SHAPE_EXTRACTOR_H 
#define _REGION_SHAPE_EXTRACTOR_H 

#ifndef M_PI
#define M_PI 3.141592653
#endif

#define HYPOT(x,y) sqrt((x) * (x) + (y) * (y))

#define ART_LUT_RADIUS 50 // Zernike basis function radius
#define ART_LUT_SIZE (ART_LUT_RADIUS * 2 + 1)

#include "../../DescriptorExtractor.h"
#include "../RegionShape/RegionShape.h"

class RegionShapeExtractor : public DescriptorExtractor {
    private:
        RegionShape * descriptor = nullptr;

        bool m_bLUTInit = false;

        int m_mass;
        double m_centerX, m_centerY;
        double m_radius;

        // Real value of RegionShape basis function
        double m_pBasisR[ART_ANGULAR][ART_RADIAL][ART_LUT_SIZE][ART_LUT_SIZE]; 
        // Imaginary value of RegionShape basis function
        double m_pBasisI[ART_ANGULAR][ART_RADIAL][ART_LUT_SIZE][ART_LUT_SIZE]; 

        double m_pCoeffR[ART_ANGULAR][ART_RADIAL];
        double m_pCoeffI[ART_ANGULAR][ART_RADIAL];

        double GetReal(int p, int r, double dx, double dy);
        double GetImg(int p, int r, double dx, double dy);
    public:
	    RegionShapeExtractor();
	    Descriptor * extract(Image & image, const char ** params);
        ~RegionShapeExtractor();
};
#endif