#include "CTBrowsing.h"

CTBrowsing::CTBrowsing() = default;

void CTBrowsing::loadParameters(const char ** params) {
}

std::string CTBrowsing::generateXML() {
    XMLDocument xmlDoc;
    XMLDeclaration * p_Declaration = xmlDoc.NewDeclaration("xml version='1.0' encoding='ISO-8859-1' ");
    XMLElement * p_Root = xmlDoc.NewElement("Mpeg7");
    XMLElement * p_DescriptionUnit = xmlDoc.NewElement("DescriptionUnit");
    XMLElement * p_Descriptor = xmlDoc.NewElement("Descriptor");
    XMLElement * p_BrowsingCategory = xmlDoc.NewElement("BrowsingCategory");
    XMLElement * p_SubRanngeIndex = xmlDoc.NewElement("SubRangeIndex");

    p_Root->SetAttribute("xmlns", "urn:mpeg:mpeg7:schema:2001");
    p_Root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    p_DescriptionUnit->SetAttribute("xsi:type", "DescriptorCollectionType");
    p_Descriptor->SetAttribute("xsi:type", "ColorTemperatureBrowsingType");

    // Set BrowsingCategory
    int temp = ctBrowsingComponent[0];

    std::string category_string = temp == 0 ? "hot"      :
                                  temp == 1 ? "warm"     :
                                  temp == 2 ? "moderate" :
                                  temp == 3 ? "cool"     : throw CT_BROWSING_GEN_CATEGORY_ERROR;

    p_BrowsingCategory->SetText(category_string.c_str());

    // Set SubRangeIndex
    p_SubRanngeIndex->SetText(ctBrowsingComponent[1]);

    xmlDoc.InsertFirstChild(p_Declaration);
    xmlDoc.InsertEndChild(p_Root);

    p_Root->InsertEndChild(p_DescriptionUnit);
    p_DescriptionUnit->InsertEndChild(p_Descriptor);
    p_Descriptor->InsertEndChild(p_BrowsingCategory);
    p_Descriptor->InsertEndChild(p_SubRanngeIndex);

    XMLPrinter xmlPrinter;
    xmlDoc.Accept(&xmlPrinter);
    return xmlPrinter.CStr();
}

void CTBrowsing::readFromXML(XMLElement * descriptorElement) {
    // Load Browsing Category
    const XMLElement * xmlBrowsingCategoryElement = descriptorElement->FirstChildElement("BrowsingCategory");

    if (xmlBrowsingCategoryElement == nullptr) {
        throw CT_BROWSING_BROWSING_CATEGORY_NOT_FOUND;
    }

    const std::string xmlBrowsingCategory = xmlBrowsingCategoryElement->GetText();

    ctBrowsingComponent = new int[2];

    ctBrowsingComponent[0] = xmlBrowsingCategory.compare("hot")      == 0 ? 0 :
                             xmlBrowsingCategory.compare("warm")     == 0 ? 1 :
                             xmlBrowsingCategory.compare("moderate") == 0 ? 2 :
                             xmlBrowsingCategory.compare("cool")     == 0 ? 3 : throw CT_BROWSING_UNRECOGNIZED_CATEGORY;

    // Load SubRangeIndex 
    const XMLElement * xmlSubRangeIndexElement = descriptorElement->FirstChildElement("SubRangeIndex");

    if (xmlSubRangeIndexElement == nullptr) {
        throw CT_BROWSING_SUBRANGE_IDX_NOT_FOUND;
    }

    std::stringstream ss_SubRangeIdx(xmlSubRangeIndexElement->GetText());
    ss_SubRangeIdx >> ctBrowsingComponent[1];
}

void CTBrowsing::SetCTBrowsing_Component(int * PBC) {
    if (ctBrowsingComponent == nullptr) { 
        ctBrowsingComponent = new int[2];
    }

	for (int i = 0; i < 2; i++) {
		ctBrowsingComponent[i] = PBC[i];
	}
}

int * CTBrowsing::getCTBrowsingComponent() {
    return ctBrowsingComponent;
}

CTBrowsing::~CTBrowsing() {
	delete[] ctBrowsingComponent;
}