/** @file  ColorStructure.h
 *  @brief Color Structure descriptor data,
 *         method for loading parameters, read and wirte to XML,
 *         modify and access data.
 *
 *    Descriptor data:
 *        - targetSize:
 *
 *          User definied final number of coefficients, that appear in xml.
 *
 *        - descriptorSize:
 *          
 *          Real descriptor size, as default in extraction gets value 256,                         \n
 *          because in extraction first result is full array of 256 coefficients.                  \n
 *          Later that number is reduced to targetSize and only that number of coefficients is     \n
 *          written to xml (starting from index 0)                          
 *
 *        - descriptorData:
 *          
 *           Array of descriptor coefficients, that describe descritptor data
 *          Appear after extraction in 256 size, which is later cut to targetSize.
 *
 *          These are integer arrays that hold a series of zigzag scanned DCT coefficients values, \n
 *          which are used later for xml creation.
 *
 *  @author Krzysztof Lech Kucharski
 *  @bug No bugs detected.                                                                          */

#pragma once

#include "../../Descriptor.h"

class ColorStructure : public Descriptor {
	private:
        unsigned long   targetSize = 256;      // default
        unsigned long   descriptorSize;        // final descriptor size
		unsigned long * descriptorData = nullptr; // final result coefficients

	public:
        ColorStructure();

        void loadParameters(const char ** params);
		void readFromXML(XMLElement * descriptorElement);
        std::string generateXML();

        unsigned long SetSize(unsigned long size);
        unsigned long GetSize();
        unsigned long SetElement(unsigned long index, int value);
        unsigned long GetElement(unsigned long index);
        int GetTargetSize();

		~ColorStructure();
};
