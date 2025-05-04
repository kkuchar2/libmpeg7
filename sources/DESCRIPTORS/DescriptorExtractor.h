/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/** @file DescriptorExtractor.h
* @brief Pure abstract class representing general descriptor extractor object.
* Each descriptor extractor for 10 supported by the library descriptors inherits methods from
* that class, so it is possible to use polymorphism in main extraction calculation methods.
*
*  @author Krzysztof Lech Kucharski */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#pragma once

#include "Descriptor.h"

class DescriptorExtractor {
    public:
        /** @brief
        * Performs descirptor extraction on loaded image object and \n
        * whith user specified parameter.
        *
        * @param image - internal image loaded from file or decoded from array data
        * @param params - additional user parameters for extraction
        *
        * @return Descriptor * - pointer to extracted descriptor object */
        virtual Descriptor * extract(Image & image, const char ** params) = 0;
        /** @brief
        * General destructor */
        virtual ~DescriptorExtractor() = 0;
};

inline DescriptorExtractor::~DescriptorExtractor() {
}
