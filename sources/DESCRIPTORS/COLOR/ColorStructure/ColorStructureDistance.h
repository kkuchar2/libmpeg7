/** @file   ColorStructureDistance.h
 *  @brief  Color Structure class for distance calculation.
 *  @author Krzysztof Lech Kucharski
 *  @bug    No bugs detected.                              */

#pragma once

#include "../../DescriptorDistance.h"
#include "../ColorStructure/ColorStructure.h"

class ColorStructureDistance : public DescriptorDistance {
    public:
        ColorStructureDistance();
        double getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params);
        ~ColorStructureDistance();
};
