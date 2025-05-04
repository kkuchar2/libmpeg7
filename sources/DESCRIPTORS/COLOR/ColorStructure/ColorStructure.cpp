#include "ColorStructure.h"

ColorStructure::ColorStructure() {
}

void ColorStructure::loadParameters(const char ** params) {
    if (params == nullptr) {
        return; // default value remains default
    }

    /* SIZE | PARAM COMBINATION
    -------------------------------------------
          0 |  [NULL]
          2 |  [ColorQuantSize, value, NULL]
    ------------------------------------------- */

    // Count parameters size
    int size = 0;
    for (int i = 0; params[i] != nullptr; i++) {
        size++;
    }

    // Check size
    if (size != 0 && size != 2) {
        throw COL_STRUCT_PARAMS_NUMBER_ERROR;
    }

    // Loop through parameters
    for (int i = 0; params[i] != nullptr; i += 2) {
        if (params[i] == nullptr || params[i + 1] == nullptr) {
            break; // default value remains default
        }

        std::string p1(params[i]);
        std::string p2(params[i + 1]);           

        if (!p1.compare("ColorQuantSize")) {
            targetSize = !p2.compare("32") || 
                         !p2.compare("64") || !p2.compare("128") || 
                         !p2.compare("256") ? std::stoi(p2) : throw COL_STRUCT_PARAM_VALUE_ERROR;
        }
        else {
            throw COL_STRUCT_PARAM_NAME_ERROR;
        }
    }
}

std::string ColorStructure::generateXML() {
    XMLDocument xmlDoc;

    XMLDeclaration * p_Declaration = xmlDoc.NewDeclaration("xml version='1.0' encoding='ISO-8859-1' ");
    XMLElement * p_Root = xmlDoc.NewElement("Mpeg7");
    XMLElement * p_DescriptionUnit = xmlDoc.NewElement("DescriptionUnit");
    XMLElement * p_Descriptor = xmlDoc.NewElement("Descriptor");
    XMLElement * p_Values = xmlDoc.NewElement("Values");

    p_Root->SetAttribute("xmlns", "urn:mpeg:mpeg7:schema:2001");
    p_Root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    p_DescriptionUnit->SetAttribute("xsi:type", "DescriptorCollectionType");
    p_Descriptor->SetAttribute("xsi:type", "ColorStructureType");

    switch (descriptorSize) {
        case 32:
            p_Descriptor->SetAttribute("colorQuant", 1);
            break;
        case 64:
            p_Descriptor->SetAttribute("colorQuant", 2);
            break;
        case 128:
            p_Descriptor->SetAttribute("colorQuant", 3);
            break;
        case 256:
            p_Descriptor->SetAttribute("colorQuant", 4);
            break;
        default:
            p_Descriptor->SetAttribute("colorQuant", 0);
            break;
    }

    std::string values_string;
    
    for (unsigned int i = 0; i < descriptorSize; i++) {
        values_string.append(std::to_string(descriptorData[i]) + " ");
    }

    p_Values->SetText(values_string.c_str());

    xmlDoc.InsertFirstChild(p_Declaration);
    xmlDoc.InsertEndChild(p_Root);

    p_Root->InsertEndChild(p_DescriptionUnit);
    p_DescriptionUnit->InsertEndChild(p_Descriptor);
    p_Descriptor->InsertEndChild(p_Values);

    XMLPrinter xmlPrinter;
    xmlDoc.Accept(&xmlPrinter);

    return xmlPrinter.CStr();
}

void ColorStructure::readFromXML(XMLElement * descriptorElement) {
    /*  Load ColorQuant Parameter */
    std::stringstream ss_colorQuant(descriptorElement->Attribute("colorQuant"));

    int colorQuantValue;

    ss_colorQuant >> colorQuantValue;
    
    switch (colorQuantValue) {
        case 1:
            targetSize     = 32;
            descriptorSize = 32;
            break;
        case 2:
            descriptorSize = 64;
            targetSize     = 64;
            break;
        case 3:
            targetSize     = 128;
            descriptorSize = 128;
            break;
        case 4:
            targetSize     = 256;
            descriptorSize = 256;
            break;
        default:
            throw COL_STRUCT_XML_QUANT_VAL_ERROR;
            break;
    }

    /* Load data */
    XMLElement * p_Values = descriptorElement->FirstChildElement("Values");

    if (p_Values == nullptr) {
        throw COL_STRUCT_VALUES_NODE_NOT_FOUND;
    }

    std::stringstream ss_ColorStructureData(descriptorElement->FirstChildElement("Values")->GetText());

    descriptorData = new unsigned long[descriptorSize];

    unsigned long index = 0;
    unsigned long currentValue;

    std::vector<unsigned long> valuesVector;

    while (ss_ColorStructureData >> currentValue) {
        valuesVector.push_back(currentValue);

        if (ss_ColorStructureData.fail()) {
            ss_ColorStructureData.clear();
            throw COL_STRUCT_XML_VALUE_NOT_INTEGER;
        }
    }

    if (valuesVector.size() != descriptorSize) {
        throw COL_STRUCT_VALUES_COUNT_ERROR;
    }

    for (unsigned long i = 0; i < descriptorSize; i++) {
        descriptorData[i] = valuesVector[i];
    }
}

unsigned long ColorStructure::SetSize(unsigned long size) {
	if (size == 32 || size == 64 || size == 128 || size == 256) {
		if (descriptorSize == size) {
			return 0;
		}
		if (descriptorData) {
			delete[] descriptorData;
		}

        descriptorData = new unsigned long[size];

		if (!descriptorData) {
            throw COL_STRUCT_SETSIZE_ALLOCATION_ERROR;
		}

		descriptorSize = size;
		return 0;
	}
	else {
        throw COL_STRUCT_SETSIZE_SIZE_ERROR;
	}
	return 0;
}

unsigned long ColorStructure::SetElement(unsigned long index, int value) {
    if (index < descriptorSize && descriptorData) {
        descriptorData[index] = value;
        return 0;
    }

    else {
        throw COL_STRUCT_SETELEMENT_OUT_OF_BOUNDS;
    }
    return 0;
}

int ColorStructure::GetTargetSize() {
    return targetSize;
}

unsigned long ColorStructure::GetSize() {
    return descriptorSize;
}

unsigned long ColorStructure::GetElement(unsigned long index) {
	if (index < descriptorSize && descriptorData) {
		return descriptorData[index];
	}
	else {
        throw COL_STRUCT_GETELEMENT_WRONG_ACCESS;
	}
	return 0;
}

ColorStructure::~ColorStructure() {
    if (descriptorData) {
        delete[] descriptorData;
    }
}