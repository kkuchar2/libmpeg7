/** @file   RegionShapeDistance.h
*   @brief  Region Shape class for distance calculation.
*   @author Krzysztof Lech Kucharski
*   @bug    No bugs detected.                            */

#ifndef _REGION_SHAPE_DISTANCE_H 
#define _REGION_SHAPE_DISTANCE_H 

#include "../../DescriptorDistance.h"
#include "../RegionShape/RegionShape.h"

class RegionShapeDistance : public DescriptorDistance {
    private:
        RegionShape * descriptor = NULL;
    public:
        RegionShapeDistance();
        double getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params);
        ~RegionShapeDistance();
};
#endif