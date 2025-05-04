/** @file   DominantColorDistance.h
 *  @brief  Dominant Color class for distance calculation.
 *  @author Krzysztof Lech Kucharski
 *  @bug    No bugs detected.                            */

#ifndef _DOMINANTCOLORDISTANCE_H 
#define _DOMINANTCOLORDISTANCE_H

#include "../DominantColor/DominantColor.h"
#include "../../DescriptorDistance.h"

class DominantColorDistance : public DescriptorDistance {
    private:
        bool variancePresent         = false; // Variance Present
        bool spatialCoherencyPresent = false; // Spatial Coherency required
    public:
        DominantColorDistance();

        void loadParameters(const char ** params);

        double getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params);

        void rgb2luv(int * RGB, float * LUV, int size);
        double GetDistanceVariance(float * per1, float ** color1, float ** var1, int size1, float * per2, float ** color2, float ** var2, int size2);
        
        ~DominantColorDistance();
};
#endif