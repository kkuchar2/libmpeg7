/** @file  RegionShape.h
 *  @brief Region Shape descriptor data,
 *         method for loading parameters, read and wirte to XML,
 *         modify and access data and additional quantization tables
 *         for modifying data.
 *
 *  @author Krzysztof Lech Kucharski
 *  @bug No bugs detected. */

#pragma once

#include "../../Descriptor.h"


#define ART_ANGULAR	12
#define ART_RADIAL 	3

class RegionShape : public Descriptor {
	private:
        static const double QuantTable[17];
        static const double IQuantTable[16];

		char m_ArtDE[ART_ANGULAR][ART_RADIAL];

	public:
        RegionShape();

        void loadParameters(const char ** params);
        void readFromXML(XMLElement * descriptorElement);
        std::string generateXML();

        bool SetElement(char p, char r, double value);
        char GetElement(char p, char r);
        double GetRealValue(char p, char r);
		~RegionShape();
};
