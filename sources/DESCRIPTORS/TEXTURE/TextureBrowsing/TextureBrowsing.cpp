#include "TextureBrowsing.h"

TextureBrowsing::TextureBrowsing() {
}

void TextureBrowsing::loadParameters(const char ** params) {
    if (params == nullptr) {
        return; // default value remains default
    }

    /* SIZE | PARAM COMBINATION
    -------------------------------
    0 |  [NULL]
    2 |  [layer, value, NULL]
    ------------------------------- */

    // Count parameters size
    int size = 0;
    for (int i = 0; params[i] != nullptr; i++) {
        size++;
    }

    // Check size
    if (size != 0 && size != 2 && size != 4) {
        throw TEXT_BROWS_PARAMS_NUMBER_ERROR;
    }

    for (int i = 0; params[i] != nullptr; i += 2) {
        if (params[i] == nullptr || params[i + 1] == nullptr) {
            return;
        }

        std::string p1(params[i]);
        std::string p2(params[i + 1]);

        if (!p1.compare("layer")) {
            m_ComponentNumberFlag = !p2.compare("0") || !p2.compare("1") ?
                                     std::stoi(p2) : throw TEXT_BROWS_PARAMS_VALUE_ERROR;
        }
        else {
            throw TEXT_BROWS_PARAMS_NAME_ERROR;
        }
    }
}

void TextureBrowsing::readFromXML(XMLElement * descriptorElement) {
    XMLElement * p_Regularity = nullptr;
    XMLElement * p_Direction1 = nullptr;
    XMLElement * p_Scale1     = nullptr;
    XMLElement * p_Direction2 = nullptr;
    XMLElement * p_Scale2     = nullptr;

    // Load xml elements
    for (XMLElement * child = descriptorElement->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
        std::string name = child->Name();

        if (!name.compare("Regularity")) {
            p_Regularity = child;
        }
        else if (!name.compare("Direction") && p_Direction1 == nullptr) {
            p_Direction1 = child;
        }
        else if (!name.compare("Direction") && p_Direction1 != nullptr) {
            p_Direction2 = child;
        }
        else if (!name.compare("Scale") && p_Scale1 == nullptr) {
            p_Scale1 = child;
        }
        else if (!name.compare("Scale") && p_Scale1 != nullptr) {
            p_Scale2 = child;
        }
    }

    if (p_Regularity == nullptr) {
        throw TEXT_BROWS_XML_REGULARITY_MISSING;
    }

    if (p_Direction1 == nullptr) {
        throw TEXT_BROWS_XML_DIRECTION_MISSING;
    }

    if (p_Scale1 == nullptr) {
        throw TEXT_BROWS_XML_SCALE_MISSING;
    }

    // Load values
    m_Browsing_Component = new int[5];

    std::string xmlRegularity = p_Regularity->GetText();
    std::string xmlDirection1 = p_Direction1->GetText();
    std::string xmlScale1     = p_Scale1->GetText();

    m_Browsing_Component[0] = 
        !xmlRegularity.compare("irregular")             ? 1 :
        !xmlRegularity.compare("slightly regular")      ? 2 :
        !xmlRegularity.compare("regular")               ? 3 :
        !xmlRegularity.compare("highly regular")        ? 4 : -1;

    m_Browsing_Component[1] = 
        !xmlDirection1.compare("No directionality")     ? 0 :
        !xmlDirection1.compare("0 degree")              ? 1 :
        !xmlDirection1.compare("30 degree")             ? 2 :
        !xmlDirection1.compare("60 degree")             ? 3 :
        !xmlDirection1.compare("90 degree")             ? 4 :
        !xmlDirection1.compare("120 degree")            ? 5 :
        !xmlDirection1.compare("150 degree")            ? 6 : -1;

    m_Browsing_Component[2] = 
        !xmlScale1.compare("fine")                      ? 1 :
        !xmlScale1.compare("medium")                    ? 2 :
        !xmlScale1.compare("coarse")                    ? 3 :
        !xmlScale1.compare("very coarse")               ? 4 : -1;

    if (p_Direction2 != nullptr) {
        std::string xmlDirection2 = p_Direction2->GetText();

        m_ComponentNumberFlag = 1;

        m_Browsing_Component[3] =
            !xmlDirection2.compare("No directionality") ? 0 :
            !xmlDirection2.compare("0 degree")          ? 1 :
            !xmlDirection2.compare("30 degree")         ? 2 :
            !xmlDirection2.compare("60 degree")         ? 3 :
            !xmlDirection2.compare("90 degree")         ? 4 :
            !xmlDirection2.compare("120 degree")        ? 5 :
            !xmlDirection2.compare("150 degree")        ? 6 : -1;

        std::string xmlScale2 = p_Scale2->GetText();

        m_Browsing_Component[4] =
            !xmlScale2.compare("fine")                  ? 1 :
            !xmlScale2.compare("medium")                ? 2 :
            !xmlScale2.compare("coarse")                ? 3 :
            !xmlScale2.compare("very coarse")           ? 4 : -1;
    }
    else {
        m_ComponentNumberFlag   = 0;
        m_Browsing_Component[3] = 0;
        m_Browsing_Component[4] = 0;
    }

    // Check for errors
    if (m_Browsing_Component[0] == -1 || 
        m_Browsing_Component[1] == -1 || 
        m_Browsing_Component[2] == -1 || 
        m_Browsing_Component[3] == -1 || 
        m_Browsing_Component[4] == -1) {

        throw TEXT_BROWS_XML_WRONG_VALUES;
    }
}

void TextureBrowsing::SetComponentNumberFlag(int ComponentNumber) {
	m_ComponentNumberFlag = ComponentNumber;
}

void TextureBrowsing::SetBrowsing_Component(int * PBC) {
	int i;
	m_Browsing_Component = new int[5];

	for (i = 0; i < 5; ++i) {
		this->m_Browsing_Component[i] = PBC[i];
	}
}

int TextureBrowsing::GetComponentNumberFlag() {
    return m_ComponentNumberFlag;
}

int * TextureBrowsing::getBrowsingComponent() {
    return m_Browsing_Component;
}

std::string TextureBrowsing::generateXML() {
	XMLDocument xmlDoc;

	XMLDeclaration * p_Declaration = xmlDoc.NewDeclaration("xml version='1.0' encoding='ISO-8859-1' ");
	XMLElement * p_Root = xmlDoc.NewElement("Mpeg7");
	XMLElement * p_DescriptionUnit = xmlDoc.NewElement("DescriptionUnit");
	XMLElement * p_Descriptor = xmlDoc.NewElement("Descriptor");
	XMLElement * p_Regularity = xmlDoc.NewElement("Regularity");
	XMLElement * p_Direction1 = xmlDoc.NewElement("Direction");
	XMLElement * p_Scale1 = xmlDoc.NewElement("Scale");
	XMLElement * p_Direction2 = nullptr;
	XMLElement * p_Scale2 = nullptr;

	p_Root->SetAttribute("xmlns", "urn:mpeg:mpeg7:schema:2001");
	p_Root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	p_DescriptionUnit->SetAttribute("xsi:type", "DescriptorCollectionType");
	p_Descriptor->SetAttribute("xsi:type", "TextureBrowsingType");

	p_Root->InsertEndChild(p_DescriptionUnit);
	p_DescriptionUnit->InsertEndChild(p_Descriptor);

	p_Descriptor->InsertEndChild(p_Regularity);
	p_Descriptor->InsertEndChild(p_Direction1);
	p_Descriptor->InsertEndChild(p_Scale1);

	std::string regularity_string = "";
	std::string direction1_string = "";
	std::string scale1_string	  = "";

	/*
    Set regularity node */
	switch (m_Browsing_Component[0]) {
		case 1:
			regularity_string.append("irregular");
			break;
		case 2:
			regularity_string.append("slightly regular");
			break;
		case 3:
			regularity_string.append("regular");
			break;
		case 4:
			regularity_string.append("highly regular");
			break;
		default:
            throw TEXT_BROWS_WRONG_COMPONENT_VALUE;
	}
	p_Regularity->SetText(regularity_string.c_str());

	/*
    Set first direction node */
	switch (m_Browsing_Component[1]) {
		case 0:
			direction1_string.append("No directionality");
			break;
		case 1:
			direction1_string.append("0 degree");
			break;
		case 2:
			direction1_string.append("30 degree");
			break;
		case 3:
			direction1_string.append("60 degree");
			break;
		case 4:
			direction1_string.append("90 degree");
			break;
		case 5:
			direction1_string.append("120 degree");
			break;
		case 6:
			direction1_string.append("150 degree");
			break;
		default:
            throw TEXT_BROWS_WRONG_COMPONENT_VALUE;
	}
	p_Direction1->SetText(direction1_string.c_str());

	/*
    Set first scale node */
	switch (m_Browsing_Component[2]) {
		case 1:
			scale1_string.append("fine");
			break;
		case 2:
			scale1_string.append("medium");
			break;
		case 3:
			scale1_string.append("coarse");
			break;
		case 4:
			scale1_string.append("very coarse");
			break;
		default:
            throw TEXT_BROWS_WRONG_COMPONENT_VALUE;
	}
	p_Scale1->SetText(scale1_string.c_str());

    /*
    Set second direction node if flag requires */
	if (m_ComponentNumberFlag != 0) { 
		std::string direction2_string = "";
		std::string scale2_string = "";

		p_Direction2 = xmlDoc.NewElement("Direction");
		p_Descriptor->InsertEndChild(p_Direction2);

		switch (m_Browsing_Component[3]) {
			case 0:
				direction2_string.append("No directionality");
				break;
			case 1:
				direction2_string.append("0 degree");
				break;
			case 2:
				direction2_string.append("30 degree");
				break;
			case 3:
				direction2_string.append("60 degree");
				break;
			case 4:
				direction2_string.append("90 degree");
				break;
			case 5:
				direction2_string.append("120 degree");
				break;
			case 6:
				direction2_string.append("150 degree");
				break;
			default:
                throw TEXT_BROWS_WRONG_COMPONENT_VALUE;
		}
		p_Direction2->SetText(direction2_string.c_str());

		/*
        Set second scale node: */
		p_Scale2 = xmlDoc.NewElement("Scale");
		p_Descriptor->InsertEndChild(p_Scale2);

		switch (m_Browsing_Component[4]) {
			case 1:
				scale2_string.append("fine");
				break;
			case 2:
				scale2_string.append("medium");
				break;
			case 3:
				scale2_string.append("coarse");
				break;
			case 4:
				scale2_string.append("very coarse");
				break;
			default:
                throw TEXT_BROWS_WRONG_COMPONENT_VALUE;
		}
		p_Scale2->SetText(scale2_string.c_str());
	}

	xmlDoc.InsertFirstChild(p_Declaration);
	xmlDoc.InsertEndChild(p_Root);

	XMLPrinter xmlPrinter;
	xmlDoc.Accept(&xmlPrinter);
	return xmlPrinter.CStr();
}

TextureBrowsing::~TextureBrowsing() {
    if (m_Browsing_Component) {
        delete[] m_Browsing_Component;
    }
}