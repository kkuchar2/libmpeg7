#include "HomogeneousTexture.h"

HomogeneousTexture::HomogeneousTexture(): outputFeature{} {
}

void HomogeneousTexture::loadParameters(const char ** params) {
    if (params[0] == nullptr) {
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
    if (size != 0 && size != 2) {
        throw HOMOG_TEXT_PARAMS_NUMBER_ERROR;
    }

    for (int i = 0; params[i] != nullptr; i += 2) {
        if (params[i] == nullptr || params[i + 1] == nullptr) {
            return; // default value remains default
        }

        std::string p1(params[i]);
        std::string p2(params[i + 1]);

        if (!p1.compare("layer")) {
            energyDeviationFlag = !p2.compare("0") || !p2.compare("1") ? 
                                  std::stoi(p2) : throw HOMOG_TEXT_PARAMS_VALUE_ERROR;
        }
        else {
            throw HOMOG_TEXT_PARAMS_NAME_ERROR;
        }
    }
}

std::string HomogeneousTexture::generateXML() {
    XMLDocument xmlDoc;

    XMLDeclaration * p_Declaration = xmlDoc.NewDeclaration("xml version='1.0' encoding='ISO-8859-1' ");

    XMLElement     * p_Root = xmlDoc.NewElement("Mpeg7");
    XMLElement     * p_DescriptionUnit = xmlDoc.NewElement("DescriptionUnit");
    XMLElement	   * p_Descriptor = xmlDoc.NewElement("Descriptor");
    XMLElement     * p_Average = xmlDoc.NewElement("Average");
    XMLElement     * p_StandardDeviation = xmlDoc.NewElement("StandardDeviation");
    XMLElement     * p_Energy = xmlDoc.NewElement("Energy");
    XMLElement     * p_EnergyDeviation = nullptr;

    p_Root->SetAttribute("xmlns", "urn:mpeg:mpeg7:schema:2001");
    p_Root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    p_DescriptionUnit->SetAttribute("xsi:type", "DescriptorCollectionType");
    p_Descriptor->SetAttribute("xsi:type", "HomogeneousTextureType");

    p_Root->InsertEndChild(p_DescriptionUnit);
    p_DescriptionUnit->InsertEndChild(p_Descriptor);
    p_Descriptor->InsertEndChild(p_Average);
    p_Descriptor->InsertEndChild(p_StandardDeviation);
    p_Descriptor->InsertEndChild(p_Energy);

    // Set Average node
    p_Average->SetText(std::to_string(outputFeature[0]).c_str());

    // Set StandardDeviation node
    p_StandardDeviation->SetText(std::to_string(outputFeature[1]).c_str());

    std::vector<int> energy(30), energydeviation(30);

    // Set Energy node
    std::string energy_string;

    for (int i = 0; i < 30; i++) {
        energy_string.append(std::to_string(outputFeature[i + 2]));
        energy_string.append(" ");
    }
    p_Energy->SetText(energy_string.c_str());

    // Set EnergyDeviation node
    std::string energyDeviation_string;

    if (energyDeviationFlag == 1) {
        p_EnergyDeviation = xmlDoc.NewElement("EnergyDeviation");

        for (int i = 0; i < 30; i++) {
            energyDeviation_string.append(std::to_string(outputFeature[i + 32]));
            energyDeviation_string.append(" ");
            p_EnergyDeviation->SetText(energyDeviation_string.c_str());
        }
        p_Descriptor->InsertEndChild(p_EnergyDeviation);
    }
    xmlDoc.InsertFirstChild(p_Declaration);
    xmlDoc.InsertEndChild(p_Root);

    XMLPrinter xmlPrinter;
    xmlDoc.Accept(&xmlPrinter);
    return xmlPrinter.CStr();
}

void HomogeneousTexture::readFromXML(XMLElement * descriptorElement) {
    XMLElement * p_Average           = descriptorElement->FirstChildElement("Average");
    XMLElement * p_StandardDeviation = descriptorElement->FirstChildElement("StandardDeviation");
    XMLElement * p_Energy            = descriptorElement->FirstChildElement("Energy");
    XMLElement * p_EnergyDeviation   = descriptorElement->FirstChildElement("EnergyDeviation");

    if (p_Average == nullptr) {
        throw HOMOG_TEXT_XML_AVERAGE_MISSING;
    }

    if (p_StandardDeviation == nullptr) {
        throw HOMOG_TEXT_XML_STANDDEV_MISSING;
    }

    if (p_Energy == nullptr) {
        throw HOMOG_TEXT_XML_ENERGY_MISSING;
    }

    /* Load values */
    std::stringstream ss_xmlAverageValue(p_Average->GetText());
    std::stringstream ss_xmlStandardDeviationValue(p_StandardDeviation->GetText());
    std::stringstream ss_xmlEnergyValues(p_Energy->GetText());

    // Load Average value
    ss_xmlAverageValue >> outputFeature[0];

    // Load StandardDeviation value
    ss_xmlStandardDeviationValue >> outputFeature[1];

    // Load Energy values    
    std::vector<int> xmlEnergyValues;

    int singleEnergyValue;

    while (ss_xmlEnergyValues >> singleEnergyValue) {
        xmlEnergyValues.push_back(singleEnergyValue);
    }

    if (xmlEnergyValues.size() != 30) {
        throw HOMOG_TEXT_XML_ENERGY_SIZE_ERROR;
    }

    for (int i = 0; i < 30; i++) {
        outputFeature[i + 2] = xmlEnergyValues[i];
    }

    if (p_EnergyDeviation != nullptr) {
        // Load EnergyDeviation values
        std::stringstream ss_xmlEnergyDeviationValues(p_EnergyDeviation->GetText());

        std::vector<int> xmlEnergyDeviationValues;

        int singleEnergyDeviationValue;

        while (ss_xmlEnergyDeviationValues >> singleEnergyDeviationValue) {
            xmlEnergyDeviationValues.push_back(singleEnergyDeviationValue);
        }

        if (xmlEnergyDeviationValues.size() != 30) {
            energyDeviationFlag = 0;
            for (int i = 0; i < 30; i++) {
                outputFeature[i + 32] = 0;
            }
        }
        else {
            energyDeviationFlag = 1;
            for (int i = 0; i < 30; i++) {
                outputFeature[i + 32] = xmlEnergyDeviationValues[i];
            }
        }    
    }
    else {
        energyDeviationFlag = 0;
        for (int i = 0; i < 30; i++) {
            outputFeature[i + 32] = 0;
        }
    }
}

void HomogeneousTexture::SetHomogeneousTextureFeature(const int * pHomogeneousTextureFeature) {
	memcpy(outputFeature, pHomogeneousTextureFeature, sizeof(outputFeature));
}

int * HomogeneousTexture::GetHomogeneousTextureFeature() {
    return outputFeature;
}

int HomogeneousTexture::GetHomogeneousTextureFeatureFlag() const {
    return energyDeviationFlag;
}

HomogeneousTexture::~HomogeneousTexture() = default;