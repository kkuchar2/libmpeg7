#include "ScalableColor.h"

ScalableColor::ScalableColor() {
}

void ScalableColor::loadParameters(const char ** params) {
    if (params == nullptr) {
        return; // default value remains default
    }

    /* SIZE | PARAM COMBINATION
    ------------------------------------------------------------------
    0 |  [NULL]
    2 |  [NumberOfCoefficients, value, NULL]
    2 |  [NumberOfBitplanesDiscarded, value, NULL]
    4 |  [NumberOfCoefficients, value, NumberOfBitplanesDiscarded, value, NULL]
    4 |  [NumberOfBitplanesDiscarded, value, NumberOfCoefficients, value, NULL]
    ------------------------------------------------------------------ */

    // Count parameters size
    int size = 0;
    for (int i = 0; params[i] != nullptr; i++) {
        size++;
    }

    // Check size
    if (size != 0 && size != 2 && size != 4) {
        throw SCAL_COL_PARAMS_NUMBER_ERROR;;
    }

    for (int i = 0; params[i] != nullptr; i += 2) {
        if (params[i] == nullptr || params[i + 1] == nullptr) {
            break; // default value remains default
        }

        std::string p1(params[i]);
        std::string p2(params[i + 1]);

        if (!p1.compare("NumberOfCoefficients")) {
            numberOfCoefficients = !p2.compare("16")  ||
                                   !p2.compare("32")  || 
                                   !p2.compare("64")  || 
                                   !p2.compare("128") || 
                                   !p2.compare("256") ? std::stoi(p2) : 
                                   throw SCAL_COL_PARAMS_VALUE_ERROR;    
        }
        else if (!p1.compare("NumberOfBitplanesDiscarded")) {
            numberOfBitplanesDiscarded = !p2.compare("0") ||
                                         !p2.compare("1") ||
                                         !p2.compare("2") ||
                                         !p2.compare("3") ||
                                         !p2.compare("4") ||
                                         !p2.compare("6") ||
                                         !p2.compare("8") ? std::stoi(p2) : 
                                         throw SCAL_COL_PARAMS_VALUE_ERROR;
        }
        else {
            throw SCAL_COL_PARAMS_NAME_ERROR;
        }
    }
}

void ScalableColor::readFromXML(XMLElement * descriptorElement) {
    /* Load number of coefficients and bit planes discarded  */
    std::stringstream ss_numberOfCoefficients(descriptorElement->Attribute("NumberOfCoefficients"));
    std::stringstream ss_numberOfBitplanesDiscarded(descriptorElement->Attribute("NumberOfBitplanesDiscarded"));

    int xml_coeffNum;
    int xml_bitsDisc;

    ss_numberOfCoefficients >> xml_coeffNum;

    numberOfCoefficients = 
        xml_coeffNum == 0 ? 16  :
        xml_coeffNum == 1 ? 32  :
        xml_coeffNum == 2 ? 64  :
        xml_coeffNum == 3 ? 128 :
        xml_coeffNum == 4 ? 256 : throw SCAL_COL_XML_COEFF_ERROR;

    ss_numberOfBitplanesDiscarded >> xml_bitsDisc;

    numberOfBitplanesDiscarded = 
        xml_bitsDisc == 0 ? 0 :
        xml_bitsDisc == 1 ? 1 :
        xml_bitsDisc == 2 ? 2 :
        xml_bitsDisc == 3 ? 3 :
        xml_bitsDisc == 4 ? 4 :
        xml_bitsDisc == 5 ? 6 :
        xml_bitsDisc == 6 ? 8 : throw SCAL_COL_XML_BITS_DISC_ERROR;

    /* Load coefficients */
    coefficients = new int [numberOfCoefficients];

    XMLElement * coefficientsElement = descriptorElement->FirstChildElement("Coefficients");

    if (coefficientsElement == nullptr) {
        throw SCAL_COL_XML_COEFF_NODE_MISSING;
    }

    std::stringstream ss_coefficients(coefficientsElement->GetText());
   
    std::vector<int> coefficientsVector;

    int singleCoefficient;

    /* Read coefficients from xml to temporary vector of integers */
    while (ss_coefficients >> singleCoefficient) {
        coefficientsVector.push_back(singleCoefficient);
    }
    
    /* Check, if read coefficients count is equal to read coeffcients number */
    if (coefficientsVector.size() != numberOfCoefficients) {
        throw SCAL_COL_XML_COEFF_MATCH_ERROR;
    }

    /* Fill actual coefficients */
    for (unsigned int i = 0; i < coefficientsVector.size(); i++) {
        coefficients[i] = coefficientsVector[i];
    }
}

std::string ScalableColor::generateXML() {
	XMLDocument xmlDoc;

	XMLDeclaration * p_Declaration = xmlDoc.NewDeclaration("xml version='1.0' encoding='ISO-8859-1' ");
	XMLElement * p_Root = xmlDoc.NewElement("Mpeg7");
	XMLElement * p_DescriptionUnit = xmlDoc.NewElement("DescriptionUnit");
	XMLElement * p_Descriptor = xmlDoc.NewElement("Descriptor");
	XMLElement * p_Coefficients = xmlDoc.NewElement("Coefficients");

	p_Root->SetAttribute("xmlns", "urn:mpeg:mpeg7:schema:2001");
	p_Root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	p_DescriptionUnit->SetAttribute("xsi:type", "DescriptorCollectionType");
	p_Descriptor->SetAttribute("xsi:type", "ScalableColorType");

	std::string numberOfCoefficients_string = numberOfCoefficients == 256   ? "4" :
											  numberOfCoefficients == 128   ? "3" :
											  numberOfCoefficients == 64    ? "2" :
											  numberOfCoefficients == 32    ? "1" :
											  numberOfCoefficients == 16    ? "0" : throw SCAL_COL_XML_COEFF_ERROR;

	p_Descriptor->SetAttribute("NumberOfCoefficients", numberOfCoefficients_string.c_str());

    std::string bitplanesDiscarded_string = numberOfBitplanesDiscarded == 0 ? "0" :
                                            numberOfBitplanesDiscarded == 1 ? "1" :
                                            numberOfBitplanesDiscarded == 2 ? "2" :
                                            numberOfBitplanesDiscarded == 3 ? "3" :
                                            numberOfBitplanesDiscarded == 4 ? "4" :
                                            numberOfBitplanesDiscarded == 6 ? "5" :
                                            numberOfBitplanesDiscarded == 8 ? "6" : throw SCAL_COL_XML_BITS_DISC_ERROR;

	p_Descriptor->SetAttribute("NumberOfBitplanesDiscarded", bitplanesDiscarded_string.c_str());

	std::string coefficients_string;

	for (unsigned int i = 0; i < numberOfCoefficients; i++) {
		coefficients_string.append(std::to_string(coefficients[i]) + " ");
	}

	p_Coefficients->SetText(coefficients_string.c_str());

	xmlDoc.InsertFirstChild(p_Declaration);
	xmlDoc.InsertEndChild(p_Root);

	p_Root->InsertEndChild(p_DescriptionUnit);
	p_DescriptionUnit->InsertEndChild(p_Descriptor);
	p_Descriptor->InsertEndChild(p_Coefficients);

	XMLPrinter xmlPrinter;
	xmlDoc.Accept(&xmlPrinter);

	return xmlPrinter.CStr();
}

unsigned int ScalableColor::getNumberOfCoefficients() {
    return numberOfCoefficients;
}

unsigned int ScalableColor::getNumberOfBitplanesDiscarded() {
    return numberOfBitplanesDiscarded;
}

int ScalableColor::getCoefficient(int index) {
    return coefficients[index];
}

void ScalableColor::setNumberOfCoefficients(int coeffNum) {
    numberOfCoefficients = coeffNum;
}

void ScalableColor::setNumberOfBitplanesDiscarded(int bitsDisc) {
    numberOfBitplanesDiscarded = bitsDisc;
}

void ScalableColor::allocateCoefficients(int size) {
    coefficients = new int[size];
}

int * ScalableColor::getCoefficients() {
    return coefficients;
}

ScalableColor::~ScalableColor() {
    if (coefficients) {
        delete[] coefficients;
    }
}