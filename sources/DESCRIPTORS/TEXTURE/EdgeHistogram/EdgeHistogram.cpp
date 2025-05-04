#include "EdgeHistogram.h"

EdgeHistogram::EdgeHistogram(): Local_Edge{} {
}

void EdgeHistogram::loadParameters(const char ** params) {
}

void EdgeHistogram::setEdgeHistogramElement(int index, int value) {
    m_pEdge_HistogramElement[index] = value;
}

void EdgeHistogram::setEdgeHistogramElement(char * pEdgeHistogram) {
    int i;
    for (i = 0; i < 80; i++)
        Local_Edge[i] = QuantTable[i % 5][pEdgeHistogram[i]];

    memcpy(m_pEdge_HistogramElement, pEdgeHistogram, 80);
}

char EdgeHistogram::getEdgeHistogramElement(int index) {
    return m_pEdge_HistogramElement[index];
}

void EdgeHistogram::readFromXML(XMLElement * descriptorElement) {
    /* Load BinCounts element */
    const XMLElement * p_BinCounts = descriptorElement->FirstChildElement("BinCounts");

    if (p_BinCounts == nullptr) {
        throw EDGE_HIST_XML_BINCOUNTS_MISSING;
    }

    /* Load BinCounts values */
    std::stringstream ss_BinCountsData(p_BinCounts->GetText());

    int single_value;

    std::vector<int> xmlBinCountsValues;

    while (ss_BinCountsData >> single_value) {
        xmlBinCountsValues.push_back(single_value);
    }

    if (xmlBinCountsValues.size() != 80) {
        throw EDGE_HIST_BIN_COUNTS_SIZE_ERROR;
    }

    char tempBins[80];

    for (int i = 0; i < 80; i++) {
        tempBins[i] = (char) xmlBinCountsValues[i];
    }

    setEdgeHistogramElement(tempBins);
}

std::string EdgeHistogram::generateXML() {
	XMLDocument xmlDoc;

	XMLDeclaration * p_Declaration = xmlDoc.NewDeclaration("xml version='1.0' encoding='ISO-8859-1' ");
	XMLElement * p_Root = xmlDoc.NewElement("Mpeg7");
	XMLElement * p_DescriptionUnit = xmlDoc.NewElement("DescriptionUnit");
	XMLElement * p_Descriptor = xmlDoc.NewElement("Descriptor");
	XMLElement * p_BinCounts = xmlDoc.NewElement("BinCounts");

	p_Root->SetAttribute("xmlns", "urn:mpeg:mpeg7:schema:2001");
	p_Root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	p_DescriptionUnit->SetAttribute("xsi:type", "DescriptorCollectionType");
	p_Descriptor->SetAttribute("xsi:type", "EdgeHistogramType");

	p_Root->InsertEndChild(p_DescriptionUnit);
	p_DescriptionUnit->InsertEndChild(p_Descriptor);
	p_Descriptor->InsertEndChild(p_BinCounts);

	std::string binCounts_string;

	for (unsigned int i = 0; i < 80; i++) {
		binCounts_string.append(std::to_string(m_pEdge_HistogramElement[i]) + " ");
	}

	p_BinCounts->SetText(binCounts_string.c_str());

	xmlDoc.InsertFirstChild(p_Declaration);
	xmlDoc.InsertEndChild(p_Root);

	XMLPrinter xmlPrinter;
	xmlDoc.Accept(&xmlPrinter);
	return xmlPrinter.CStr();
}

EdgeHistogram::~EdgeHistogram() {
    delete[] m_pEdge_HistogramElement;
}

const double EdgeHistogram::QuantTable[5][8] = {
    { 0.010867, 0.057915, 0.099526, 0.144849, 0.195573, 0.260504, 0.358031, 0.530128 },
    { 0.012266, 0.069934, 0.125879, 0.182307, 0.243396, 0.314563, 0.411728, 0.564319 },
    { 0.004193, 0.025852, 0.046860, 0.068519, 0.093286, 0.123490, 0.161505, 0.228960 },
    { 0.004174, 0.025924, 0.046232, 0.067163, 0.089655, 0.115391, 0.151904, 0.217745 },
    { 0.006778, 0.051667, 0.108650, 0.166257, 0.224226, 0.285691, 0.356375, 0.450972 } };