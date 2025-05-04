/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/** @file DescriptorDistance.h
* @brief Pure abstract class representing general descriptor comparator object.
* Each descriptor comparator for 10 supported by the library descriptors inherits methods from
* that class, so it is possible to use polymorphism in main distance calculation method.
*
*  @author Krzysztof Lech Kucharski */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#pragma once

#include "Descriptor.h"

class DescriptorDistance {
    public:
        /** @brief
        * Reads distance between two descriptors of the same type as reslt of their comparison.
        * @param descriptor1 - first descriptor objects
        * @param descriptor2 - second descriptor object
        * @param params - additional distance calculation user parameters
        *
        * @return double - distance between descriptors */
        virtual double getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params) = 0;
        /** @brief
        * General destructor */
        virtual ~DescriptorDistance() = 0;
};

inline DescriptorDistance::~DescriptorDistance() {
}
