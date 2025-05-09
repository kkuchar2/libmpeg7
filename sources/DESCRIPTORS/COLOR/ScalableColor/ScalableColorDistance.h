/** @file   ScalableColorDistance.h
 *  @brief  Scalable Color class for distance calculation.
 *  @author Krzysztof Lech Kucharski
 *  @bug    No bugs detected.                            */

#pragma once

#include "../ScalableColor/ScalableColor.h"
#include "../../DescriptorDistance.h"

class ScalableColorDistance : public DescriptorDistance {
    private:
        unsigned int numberOfCoefficients = 256;
    public:
        ScalableColorDistance();
        void loadParameters(const char ** params);
        double getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params);
        ~ScalableColorDistance();
};
