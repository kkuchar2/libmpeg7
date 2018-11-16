/** @file   HomogeneousTextureDistance.h
*   @brief  Homogeneous Texture class for distance calculation.
*   @author Krzysztof Lech Kucharski
*   @bug    No bugs detected.                            */

#ifndef _HOMOGENEOUSTEXTURE_DISTANCE_H
#define _HOMOGENEOUSTEXTURE_DISTANCE_H

#include "../../DescriptorDistance.h"
#include "../HomogeneusTexture/HomogeneousTexture.h"

#define RadialDivision  5
#define RadialDivision2 3
#define AngularDivision 6

#define Nray	     128 // Number of ray
#define Nview		 180 // Number of view
#define NUMofFEATURE 62
#define Quant_level	 255

/* KK - it seems that XM algorithm is not symmetrical 
   different distances are achievied, when comparing two files
   in order: 1,2, and different, when comparing in reverse order: 2,1 */
class HomogeneousTextureDistance : public DescriptorDistance {
    private:
        HomogeneousTexture * descriptor = NULL;
        const char * option = NULL;
    public:
        HomogeneousTextureDistance();

        double getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params);

        void loadParameters(const char ** params);

        void Dequantization(int * integerFeature, float * floatFeature);
        void Normalization(float * feature);

        ~HomogeneousTextureDistance();
};
#endif