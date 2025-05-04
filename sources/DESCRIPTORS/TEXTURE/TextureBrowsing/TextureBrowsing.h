/** @file  TextureBrowsing.h
 *  @brief Texture Browsing descriptor data,
 *         method for loading parameters, read and wirte to XML,
 *         modify and access data.
 *
 *  @author Krzysztof Lech Kucharski
 *  @bug No bugs detected. */

#ifndef _TEXTUREBROWSING_H  
#define _TEXTUREBROWSING_H

#include "../../Descriptor.h"

class TextureBrowsing : public Descriptor {
	private:
        int m_ComponentNumberFlag  = 1;
        int * m_Browsing_Component = nullptr;
	public:
        TextureBrowsing();
        void loadParameters(const char ** params);

		void readFromXML(XMLElement * descriptorElement);

        void SetComponentNumberFlag(int ComponentNumber);
        void SetBrowsing_Component(int * PBC);
        int GetComponentNumberFlag();
        int * getBrowsingComponent();

        std::string generateXML();

		~TextureBrowsing();
};
#endif