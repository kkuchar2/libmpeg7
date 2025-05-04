#include "RegionShape.h"

RegionShape::RegionShape() {
}

void RegionShape::loadParameters(const char ** params) {
}

void RegionShape::readFromXML(XMLElement * descriptorElement) {   
    /* Load MagnitudeOfART element */
    XMLElement * p_MagnitudeOfArt = descriptorElement->FirstChildElement("MagnitudeOfART");

    if (p_MagnitudeOfArt == nullptr) {
        throw REG_SHAPE_MAGNITUDE_ART_MISSING;
    }

    /* Load MagnitudeOfART values */
    std::stringstream ss_MagnitudeOfArtData(p_MagnitudeOfArt->GetText());

    int single_value;

    std::vector<int> xmlMagnitudeOfARTValues;

    while (ss_MagnitudeOfArtData >> single_value) {
        xmlMagnitudeOfARTValues.push_back(single_value);
    }

    if (xmlMagnitudeOfARTValues.size() == ART_ANGULAR * ART_RADIAL - 1) { // check, if listed values count is good
        int index = 0;
        for (int i = 0; i < ART_ANGULAR; i++) {
            for (int j = 0; j < ART_RADIAL; j++) {
                if (i != 0 || j != 0) {
                    m_ArtDE[i][j] = static_cast<char>(xmlMagnitudeOfARTValues[index]);
                    index++;
                }
                else {
                    m_ArtDE[i][j] = 0;
                }
            }
        }
    }
    else {
        throw REG_SHAPE_COEFF_COUNT_ERROR;
    }
}

bool RegionShape::SetElement(char p, char r, double value) {
	if (p < 0 || p >= ART_ANGULAR || r < 0 || r >= ART_RADIAL || value > 1.0 || value < 0.0) {
		return false;
	}

	// Quantization:
	int high = sizeof(QuantTable) / sizeof(double);
	int low = 0;
	int middle;

	while (high - low > 1) {
		middle = (high + low) / 2;

		if (QuantTable[middle] < value) {
			low = middle;
		}
		else {
			high = middle;
		}
	}
	m_ArtDE[p][r] = low;
	return true;
}

char RegionShape::GetElement(char p, char r) {
	if (p < 0 || p >= ART_ANGULAR || r < 0 || r >= ART_RADIAL) {
        throw REG_SHAPE_WRONG_ELEMENT_ACCESS;
	}
	return m_ArtDE[p][r]; // Always positive value
}

double RegionShape::GetRealValue(char p, char r) {
    return IQuantTable[m_ArtDE[p][r]];
}

std::string RegionShape::generateXML() {
	XMLDocument xmlDoc;

	XMLDeclaration * p_Declaration = xmlDoc.NewDeclaration("xml version='1.0' encoding='ISO-8859-1' ");
	XMLElement * p_Root = xmlDoc.NewElement("Mpeg7");
	XMLElement * p_DescriptionUnit = xmlDoc.NewElement("DescriptionUnit");
	XMLElement * p_Descriptor = xmlDoc.NewElement("Descriptor");
	XMLElement * p_MagnitudeOfART = xmlDoc.NewElement("MagnitudeOfART");

	p_Root->SetAttribute("xmlns", "urn:mpeg:mpeg7:schema:2001");
	p_Root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	p_DescriptionUnit->SetAttribute("xsi:type", "DescriptorCollectionType");
	p_Descriptor->SetAttribute("xsi:type", "RegionShapeType");

	p_Root->InsertEndChild(p_DescriptionUnit);
	p_DescriptionUnit->InsertEndChild(p_Descriptor);
	p_Descriptor->InsertEndChild(p_MagnitudeOfART);

	std::vector<int> V(35);

	int i, j;
	int n = 0;
	for (i = 0; i < ART_ANGULAR; i++) {
		for (j = 0; j < ART_RADIAL; j++) {
			if (i != 0 || j != 0) {
                try {
                    V[n++] = GetElement(i, j);
                }
                // Wrong element access
                catch (ErrorCode exception) { 
                    throw exception;
                }
			}
		}
	}

	std::string binCounts_string;

	for (unsigned int i = 0; i < V.size(); i++) {
		binCounts_string.append(std::to_string(V[i]) + " ");
	}

	p_MagnitudeOfART->SetText(binCounts_string.c_str());

	xmlDoc.InsertFirstChild(p_Declaration);
	xmlDoc.InsertEndChild(p_Root);

	XMLPrinter xmlPrinter;
	xmlDoc.Accept(&xmlPrinter);
	return xmlPrinter.CStr();
}

RegionShape::~RegionShape() {
}

const double RegionShape::QuantTable[17] = {
    0.000000000, 0.003585473, 0.007418411, 0.011535520, 0.015982337,
    0.020816302, 0.026111312, 0.031964674, 0.038508176, 0.045926586,
    0.054490513, 0.064619488, 0.077016351, 0.092998687, 0.115524524,
    0.154032694, 1.000000000 };

const double RegionShape::IQuantTable[16] = {
    0.001763817, 0.005468893, 0.009438835, 0.013714449, 0.018346760,
    0.023400748, 0.028960940, 0.035140141, 0.042093649, 0.050043696,
    0.059324478, 0.070472849, 0.084434761, 0.103127662, 0.131506859,
    0.192540857 };