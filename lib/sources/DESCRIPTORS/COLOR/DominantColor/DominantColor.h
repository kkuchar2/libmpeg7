/** @file   DominantColor.h
 *  @brief  Dominant Color descriptor data,
 *          method for loading parameters, read and wirte to XML,
 *          modify and access data.
 *
 *  @author Krzysztof Lech Kucharski
 *  @bug No bugs detected. */

#ifndef _DOMINANTCOLOR_H 
#define _DOMINANTCOLOR_H

#define DESCRIPTOR_SIZE 8

#include "../../Descriptor.h"

class DominantColor : public Descriptor {
	private:
        bool variancePresent         = false; // Variance Present
        bool spatialCoherencyPresent = false; // Spatial Coherency required

		unsigned char resultDescriptorSize;	 // Number of dominant colors after extracting
		
        int ** resultDominantColors = NULL;  // dominant colors values array
        int ** resultColorVariances = NULL;  // dominant colors variances array
        int * resultPercentages     = NULL;	 // dominant colors percents array
		float spatialCoherencyValue;		 // spatial coherency value
	public:
        DominantColor();

        void loadParameters(const char ** params);
		void readFromXML(XMLElement * descriptorElement);
        std::string generateXML();

        bool getVariancePresent();
        bool getSpatialCoherencyPresent();

        void allocateResultArrays(int size);

        int ** getResultDominantColors();
        int ** getResultColorVariances();
        int *  getResultPercentages();
        float getSpatialCoherencyValue();

        void setSpatialCoherencyValue(float value);

        unsigned char getResultDescriptorSize();
        void setResultDescriptorSize(unsigned char size);

		~DominantColor();
};
#endif