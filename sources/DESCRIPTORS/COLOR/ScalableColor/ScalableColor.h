/** @file   ScalableColor.h
 *  @brief  Scalable Color descriptor data,
 *          method for loading parameters, read and wirte to XML,
 *          modify and access data.
 *
 *  @author Krzysztof Lech Kucharski
 *  @bug No bugs detected. */

#pragma once

#include "../../Descriptor.h"

class ScalableColor : public Descriptor {
    private:
        unsigned int numberOfCoefficients       = 256;
        unsigned int numberOfBitplanesDiscarded = 0;
        int *        coefficients               = nullptr;
    public:
        ScalableColor();

        void loadParameters(const char ** params);
        void readFromXML(XMLElement * descriptorElement);
        std::string generateXML();

        unsigned int getNumberOfCoefficients();
        unsigned int getNumberOfBitplanesDiscarded();

        int getCoefficient(int index);

        void setNumberOfCoefficients(int coeffNum);
        void setNumberOfBitplanesDiscarded(int bitsDisc);

        void allocateCoefficients(int size);

        int * getCoefficients();

	    ~ScalableColor();
};
