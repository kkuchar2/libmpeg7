#include "DominantColor.h"

DominantColor::DominantColor() {
}

void DominantColor::loadParameters(const char ** params) {
    if (params == nullptr) {
        return; // default value remains default
    }

    /* SIZE | PARAM COMBINATION
    ------------------------------------------------------------------
    0 |  [NULL]
    2 |  [VariancePresent, value, NULL]
    2 |  [SpatialCoherency, value, NULL]
    4 |  [VariancePresent, value, SpatialCoherency, value, NULL]
    4 |  [SpatialCoherency, value, VariancePresent, value, NULL]
    ------------------------------------------------------------------ */

    // Count parameters size
    int size = 0;
    for (int i = 0; params[i] != nullptr; i++) {
        size++;
    }

    // Check size
    if (size != 0 && size != 2 && size != 4) {
        throw DOM_COL_PARAMS_NUMBER_ERROR;
    }

    for (int i = 0; params[i] != nullptr; i += 2) {
        if (params[i] == nullptr || params[i + 1] == nullptr) {
            break; // default value remains default
        }

        std::string p1(params[i]);
        std::string p2(params[i + 1]);

        if (!p1.compare("VariancePresent")) {
            if (!p2.compare("0")) {
                variancePresent = false;
            }
            else if (!p2.compare("1")) {
                variancePresent = true;
            }
            else {
                throw DOM_COL_PARAM_VALUE_ERROR;
            }
        }
        else if (!p1.compare("SpatialCoherency")) {
            if (!p2.compare("0")) {
                spatialCoherencyPresent = false;
            }
            else if (!p2.compare("1")) {
                spatialCoherencyPresent = true;
            }
            else {
                throw DOM_COL_PARAM_VALUE_ERROR;
            }
        }
        else {
            throw DOM_COL_PARAM_NAME_ERROR;
        }
    }
}

void DominantColor::readFromXML(XMLElement * descriptorElement) {
    std::vector<std::vector<int>> dominantColors;
    std::vector<std::vector<int>> colorVariances;
    std::vector<int> colorPercentages;

    // Iterate over all 'Descriptor' element children
    for (XMLElement * child = descriptorElement->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {

        // Get current child name 
        std::string childName = child->Name();  

        /* Watch out - SpatialCoherency node is present even, if it was not
        calculated - in that case it will always have 0 value */

        // Check, if 'SpatialCoherency' element is present
        if (!childName.compare("SpatialCoherency")) {
            std::stringstream ss_spatialCoherencyValue(child->GetText());
            ss_spatialCoherencyValue >> spatialCoherencyValue;

            if (spatialCoherencyValue == 0.0) {
                spatialCoherencyPresent = false;
            }
            else {
                spatialCoherencyPresent = true;
            }
        }

        // Get current 'Value' element
        else if (!childName.compare("Value")) {
            XMLElement * percentageElement    = child->FirstChildElement("Percentage");
            XMLElement * indexElement         = child->FirstChildElement("Index");
            XMLElement * colorVarianceElement = child->FirstChildElement("ColorVariance");

            // Check presence of elements
            if (percentageElement == nullptr) {
                throw DOM_COL_XML_NO_PERCENTAGE_ELEMENT;
            }

            if (indexElement == nullptr) {
                throw DOM_COL_XML_NO_INDEX_ELEMENT;
            }

            if (colorVarianceElement == nullptr) {
                variancePresent = false;
            }
            else {
                variancePresent = true;
            }

            std::stringstream ss_PrecentageValues(percentageElement->GetText());
            std::stringstream ss_IndexValues(indexElement->GetText());

            // a) Get current percentage value 
            int percentageValue;

            ss_PrecentageValues >> percentageValue;

            if (ss_PrecentageValues.fail()) { // not integer
                ss_PrecentageValues.clear();
                throw DOM_COL_XML_PERCENTAGE_NOT_INTEGER;
            }

            colorPercentages.push_back(percentageValue);

            // b) Get current vector of color (expected 3 values)
            std::vector<int> currentIndexVector;

            int currentIndexValue;

            while (ss_IndexValues >> currentIndexValue) {

                if (ss_IndexValues.fail()) { // not integer
                    ss_IndexValues.clear();
                    throw DOM_COL_XML_INDEX_VALUE_NOT_INTEGER;
                }
                currentIndexVector.push_back(currentIndexValue);
            }

            dominantColors.push_back(currentIndexVector);

            // c) Get color variance, if present
            if (variancePresent) {
                variancePresent = true;
                std::stringstream ss_VarianceValues(colorVarianceElement->GetText());

                std::vector<int> currentVarianceVector;

                int currentVarianceValue;

                while (ss_VarianceValues >> currentVarianceValue) {
                    if (ss_VarianceValues.fail()) { // not integer
                        ss_VarianceValues.clear();
                        throw DOM_COL_XML_VARIANCE_VALUE_NOT_INTEGER;
                    }
                    currentVarianceVector.push_back(currentVarianceValue);
                }
                colorVariances.push_back(currentVarianceVector);
            }
        }
    }

    // Allocate arrays:
    resultDescriptorSize = static_cast<unsigned char>(dominantColors.size());

    resultDominantColors = new int * [resultDescriptorSize];

    for (int i = 0; i < resultDescriptorSize; i++) {
        resultDominantColors[i] = new int[3];
    }

    if (getVariancePresent()) {
        resultColorVariances = new int * [resultDescriptorSize];

        for (int i = 0; i < resultDescriptorSize; i++) {
            resultColorVariances[i] = new int[3];
        }
    }

    resultPercentages = new int[resultDescriptorSize];
    
    // Fill arrays:
    for (unsigned int i = 0; i < dominantColors.size(); i++) {
        resultDominantColors[i][0] = dominantColors[i][0];
        resultDominantColors[i][1] = dominantColors[i][1];
        resultDominantColors[i][2] = dominantColors[i][2];

        if (getVariancePresent()) {
            resultColorVariances[i][0] = colorVariances[i][0];
            resultColorVariances[i][1] = colorVariances[i][1];
            resultColorVariances[i][2] = colorVariances[i][2];
        }

        resultPercentages[i] = colorPercentages[i];
    }
}

std::string DominantColor::generateXML() {
	XMLDocument xmlDoc;

	XMLDeclaration * p_Declaration  = xmlDoc.NewDeclaration("xml version='1.0' encoding='ISO-8859-1' ");
	XMLElement * p_Root		        = xmlDoc.NewElement("Mpeg7");
	XMLElement * p_DescriptionUnit  = xmlDoc.NewElement("DescriptionUnit");
	XMLElement * p_Descriptor		= xmlDoc.NewElement("Descriptor");
	XMLElement * p_SpatialCoherency	= nullptr;

	p_Root->SetAttribute("xmlns", "urn:mpeg:mpeg7:schema:2001");
	p_Root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	p_DescriptionUnit->SetAttribute("xsi:type", "DescriptorCollectionType");
	p_Descriptor->SetAttribute("xsi:type", "DominantColorType");

	p_Root->InsertEndChild(p_DescriptionUnit);
	p_DescriptionUnit->InsertEndChild(p_Descriptor);

	p_SpatialCoherency = xmlDoc.NewElement("SpatialCoherency");

	if (spatialCoherencyPresent) {
		p_SpatialCoherency->SetText(spatialCoherencyValue);
	}
	else {
		p_SpatialCoherency->SetText(0);
	}
    
	p_Descriptor->InsertEndChild(p_SpatialCoherency);

	std::vector<XMLElement *> p_Values(resultDescriptorSize, nullptr);
	std::vector<XMLElement *> p_ValuesPercentages(resultDescriptorSize, nullptr);
	std::vector<XMLElement *> p_ValuesColorValueIndices(resultDescriptorSize, nullptr);
	std::vector<XMLElement *> p_ValuesColorVariances(resultDescriptorSize, nullptr);

	for (int i = 0; i < resultDescriptorSize; i++) {
		p_Values[i]					 = xmlDoc.NewElement("Value");
		p_ValuesPercentages[i]		 = xmlDoc.NewElement("Percentage");
		p_ValuesColorValueIndices[i] = xmlDoc.NewElement("Index");
		p_ValuesColorVariances[i]    = xmlDoc.NewElement("ColorVariance");

		p_ValuesPercentages[i]->SetText(std::to_string(resultPercentages[i]).c_str());

		std::string colorValueIndex(std::to_string(resultDominantColors[i][0]) + " " +
								    std::to_string(resultDominantColors[i][1]) + " " +
								    std::to_string(resultDominantColors[i][2]));

		p_ValuesColorValueIndices[i]->SetText(colorValueIndex.c_str());

		p_Values[i]->InsertEndChild(p_ValuesPercentages[i]);
		p_Values[i]->InsertEndChild(p_ValuesColorValueIndices[i]);

		if (variancePresent) {
			std::string colorVariance(std::to_string(resultColorVariances[i][0]) + " " +
				                      std::to_string(resultColorVariances[i][1]) + " " +
				                      std::to_string(resultColorVariances[i][2]));

			p_ValuesColorVariances[i]->SetText(colorVariance.c_str());
			p_Values[i]->InsertEndChild(p_ValuesColorVariances[i]);
		}
		p_Descriptor->InsertEndChild(p_Values[i]);
	}

	xmlDoc.InsertFirstChild(p_Declaration);
	xmlDoc.InsertEndChild(p_Root);

	XMLPrinter xmlPrinter;
	xmlDoc.Accept(&xmlPrinter);
	return xmlPrinter.CStr();
}

bool DominantColor::getVariancePresent() {
    return variancePresent;
}

bool DominantColor::getSpatialCoherencyPresent() {
    return spatialCoherencyPresent;
}

void DominantColor::setResultDescriptorSize(unsigned char size) {
    resultDescriptorSize = size;
}

void DominantColor::allocateResultArrays(int size) {
    resultDominantColors = new int * [3 * size];
    resultColorVariances = new int * [3 * size];
    resultPercentages    = new int   [size]; 

    for (int i = 0; i < size; i++) {
        resultDominantColors[i] = new int[3];
    }

    for (int i = 0; i < size; i++) {
        resultColorVariances[i] = new int[3];
    }
}

int ** DominantColor::getResultDominantColors() {
    return resultDominantColors;
}

int ** DominantColor::getResultColorVariances() {
    return resultColorVariances;
}

int * DominantColor::getResultPercentages() {
    return resultPercentages;
}

float DominantColor::getSpatialCoherencyValue() {
    return spatialCoherencyValue;
}

void DominantColor::setSpatialCoherencyValue(float value) {
    spatialCoherencyValue = value;
}

unsigned char DominantColor::getResultDescriptorSize() {
    return resultDescriptorSize;
}

DominantColor::~DominantColor() {
    if (resultPercentages) {
        delete[] resultPercentages;
    }

    if (resultDominantColors) {
        for (int i = 0; i < resultDescriptorSize; ++i) {
            delete[] resultDominantColors[i];
        }
        delete[] resultDominantColors;
    }

    if (resultColorVariances) {
        for (int i = 0; i < resultDescriptorSize; ++i) {
            delete[] resultColorVariances[i];
        }
        delete[] resultColorVariances;
    }
}