/** @file  CTBrowsing.h
 *  @brief Color Temperature Browsing descriptor data,
 *         method for loading parameters, read and wirte to XML,
 *         modify and access data.
 *
 *  @author Krzysztof Lech Kucharski
 *  @bug No bugs detected. */

#ifndef _CTBROWSING_H 
#define _CTBROWSING_H

#include "../../Descriptor.h"

class CTBrowsing : public Descriptor {
	private:
        // Result of temperature estimation
		int * ctBrowsingComponent = NULL;
	public:
        CTBrowsing();

        void loadParameters(const char ** params);
		void readFromXML(XMLElement * descriptorElement);
        std::string generateXML();

        void SetCTBrowsing_Component(int * PBC);
        int * getCTBrowsingComponent();

		~CTBrowsing();
};
#endif
