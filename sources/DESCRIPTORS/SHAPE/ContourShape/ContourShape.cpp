#include "ContourShape.h"

ContourShape::ContourShape(): descriptorPeaksCount(0), descriptorGlobalCurvatureVector{}, descriptorPrototypeCurvatureVector{}, descriptorHighestPeakY(0) {
}

void ContourShape::loadParameters(const char ** params) {
}

void ContourShape::readFromXML(XMLElement * descriptorElement) {
    XMLElement * p_GlobalCurvature    = nullptr;
    XMLElement * p_PrototypeCurvature = nullptr;
    XMLElement * p_HighestPeakY       = nullptr;

    std::vector<XMLElement *> p_Peaks;

    /* Iterate over children */
    for (XMLElement * child = descriptorElement->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {

        std::string childName = child->Name();

        if (!childName.compare("GlobalCurvature")) {
            p_GlobalCurvature = child;
        }
        else if (!childName.compare("PrototypeCurvature")) {
            p_PrototypeCurvature = child;
        }
        else if (!childName.compare("HighestPeakY")) {
            p_HighestPeakY = child;
        }
        else if (!childName.compare("Peak")) {
            p_Peaks.push_back(child);
        }
    }

    if (p_GlobalCurvature == nullptr) {
        throw CONT_SHAPE_XML_GLOBAL_CURVATURE_MISSING;
    }

    if (p_HighestPeakY == nullptr) {
        throw CONT_SHAPE_XML_HIGHEST_PEAK_MISSING;
    }

    /* Read GlobalCurvature */

    std::stringstream ss_xmlGlobalCurvature(p_GlobalCurvature->GetText());

    std::vector<unsigned long> xmlGlobalCurvatureValues;

    unsigned long singleGlobalCurvatureElement;

    while (ss_xmlGlobalCurvature >> singleGlobalCurvatureElement) {
        xmlGlobalCurvatureValues.push_back(singleGlobalCurvatureElement);
    }

    if (xmlGlobalCurvatureValues.size() != 2) {
        throw CONT_SHAPE_XML_GLOBAL_CURVARURE_SIZE_ERROR;
    }

    for (int i = 0; i < 2; i++) {
        descriptorGlobalCurvatureVector[i] = xmlGlobalCurvatureValues[i];
    }

    /* Read PrototypeCurvature */
    if (p_PrototypeCurvature != nullptr) {
        std::stringstream ss_xmlPrototypeCurvature(p_PrototypeCurvature->GetText());

        std::vector<unsigned long> xmlPrototypeCurvatureValues;

        unsigned long singlePrototypeCurvatureValue;

        while (ss_xmlPrototypeCurvature >> singlePrototypeCurvatureValue) {
            xmlPrototypeCurvatureValues.push_back(singlePrototypeCurvatureValue);
        }

        if (xmlGlobalCurvatureValues.size() != 2) {
            throw CONT_SHAPE_XML_PROTOTYPE_CURVARURE_SIZE_ERROR;
        }
       
        for (int i = 0; i < 2; i++) {
            descriptorPrototypeCurvatureVector[i] = xmlPrototypeCurvatureValues[i];
        }
    }
    else {
        descriptorPrototypeCurvatureVector[0] = 0;
        descriptorPrototypeCurvatureVector[1] = 0;
    }

    /* Read HighestPeakY */
    std::stringstream xmlHighestPeakYValue(p_HighestPeakY->GetText());

    xmlHighestPeakYValue >> descriptorHighestPeakY;
    
    if (descriptorHighestPeakY > 0) {

        // Read all peaks
        if (p_Peaks.size() > 0) {
            SetNumberOfPeaks(static_cast<unsigned char>(p_Peaks.size()) + 1); // Number of peaks = highest peak + rest of peaks

            unsigned short currentPeakX_val;
            unsigned short currentPeakY_val;

            for (unsigned int i = 0; i < p_Peaks.size(); i++) {
                std::stringstream ss_currentPeakX(p_Peaks[i]->Attribute("peakX"));
                std::stringstream ss_currentPeakY(p_Peaks[i]->Attribute("peakY"));

                ss_currentPeakX >> currentPeakX_val;
                ss_currentPeakY >> currentPeakY_val;

                SetPeak(i + 1, currentPeakX_val, currentPeakY_val);
            }
        }
        else {
            SetNumberOfPeaks(1); // Number of peaks 1 (only highest peak)
        }

        SetPeak(0, 0, descriptorHighestPeakY);
    }
    else {
        SetNumberOfPeaks(0);
    }
}

std::string ContourShape::generateXML() {
    XMLDocument xmlDoc;

    XMLDeclaration * p_Declaration = xmlDoc.NewDeclaration("xml version='1.0' encoding='ISO-8859-1' ");
    XMLElement * p_Root               = xmlDoc.NewElement("Mpeg7");
    XMLElement * p_DescriptionUnit    = xmlDoc.NewElement("DescriptionUnit");
    XMLElement * p_Descriptor         = xmlDoc.NewElement("Descriptor");
    XMLElement * p_GlobalCurvature    = xmlDoc.NewElement("GlobalCurvature");
    XMLElement * p_PrototypeCurvature = nullptr;
    XMLElement * p_HighestPeak        = xmlDoc.NewElement("HighestPeakY");

    p_Root->SetAttribute("xmlns", "urn:mpeg:mpeg7:schema:2001");
    p_Root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    p_DescriptionUnit->SetAttribute("xsi:type", "DescriptorCollectionType");
    p_Descriptor->SetAttribute("xsi:type", "ContourShapeType");

    p_Root->InsertEndChild(p_DescriptionUnit);
    p_DescriptionUnit->InsertEndChild(p_Descriptor);
    p_Descriptor->InsertEndChild(p_GlobalCurvature);


    /* Set GlobalCurvature node */
    std::string globalCurvature_string;

    for (unsigned long i : descriptorGlobalCurvatureVector) {
        globalCurvature_string.append(std::to_string(i) + " ");
    }
    p_GlobalCurvature->SetText(globalCurvature_string.c_str());

    /* Set PrototypeCurvature node if needed */
    if (descriptorPeaksCount > 0) {
        p_PrototypeCurvature = xmlDoc.NewElement("PrototypeCurvature");

        std::string prototypeCurvature_string;
        
        for (unsigned long i : descriptorPrototypeCurvatureVector) {
            prototypeCurvature_string.append(std::to_string(i) + " ");
        }

        p_Descriptor->InsertEndChild(p_PrototypeCurvature);
        p_PrototypeCurvature->SetText(prototypeCurvature_string.c_str());

        p_Descriptor->InsertEndChild(p_PrototypeCurvature);
    }

    p_Descriptor->InsertEndChild(p_HighestPeak);

    /* Set HighestPeakY node */
    std::string highestPeakY_string;
    highestPeakY_string.append(std::to_string(descriptorHighestPeakY));
    p_HighestPeak->SetText(highestPeakY_string.c_str());

    /* Set Peak nodes */
    if (descriptorPeaksCount > 1) {
        std::vector<XMLElement *> peakNodes(descriptorPeaksCount - 1, nullptr);

        for (int i = 1; i < descriptorPeaksCount; i++) {
            peakNodes[i - 1] = xmlDoc.NewElement("Peak");

            unsigned short xp, yp;
            GetPeak(i, xp, yp);

            peakNodes[i - 1]->SetAttribute("peakX", xp);
            peakNodes[i - 1]->SetAttribute("peakY", yp);

            p_Descriptor->InsertEndChild(peakNodes[i - 1]);
        }
    }

    xmlDoc.InsertFirstChild(p_Declaration);
    xmlDoc.InsertEndChild(p_Root);

    XMLPrinter xmlPrinter;
    xmlDoc.Accept(&xmlPrinter);
    return xmlPrinter.CStr();
}

void ContourShape::SetNumberOfPeaks(unsigned char cPeaks) {
    unsigned char cOldPeaks = descriptorPeaksCount;

    // Only 5 bits used so mask out rest

    if (cPeaks > CONTOURSHAPE_CSSPEAKMASK) {
        descriptorPeaksCount = CONTOURSHAPE_CSSPEAKMASK;
    }
    else {
        descriptorPeaksCount = cPeaks;
    }

    // Manage peak memory
    if (descriptorPeaksCount != cOldPeaks) {

        if (descriptorPeaks) {
            delete[] descriptorPeaks;
        }

        descriptorPeaks = nullptr;

        if (descriptorPeaksCount > 0) {
            descriptorPeaks = (unsigned short*) new unsigned short[descriptorPeaksCount * 2];
        }
    }
    else {
        memset(descriptorPeaks, 0, descriptorPeaksCount * sizeof(unsigned short) * 2);
    }
}

void ContourShape::SetHighestPeakY(unsigned short iHigh) {
    descriptorHighestPeakY = iHigh;
}

void ContourShape::SetPeak(unsigned char cIndex, unsigned short iX, unsigned short iY) {
    unsigned char cOffset = cIndex * 2;

    if (cIndex < descriptorPeaksCount) {
        descriptorPeaks[cOffset] = iX;
        descriptorPeaks[cOffset + 1] = iY;
    }
}

void ContourShape::SetGlobalCurvature(unsigned long lC, unsigned long lE) {
    descriptorGlobalCurvatureVector[0] = lC;
    descriptorGlobalCurvatureVector[1] = lE;
}

void ContourShape::SetPrototypeCurvature(unsigned long lC, unsigned long lE) {
    if (descriptorPeaksCount > 0) {
        descriptorPrototypeCurvatureVector[0] = lC;
        descriptorPrototypeCurvatureVector[1] = lE;
    }
}

void ContourShape::GetGlobalCurvature(unsigned long & lC, unsigned long & lE) const {
    lC = descriptorGlobalCurvatureVector[0];
    lE = descriptorGlobalCurvatureVector[1];
}

unsigned char ContourShape::GetNumberOfPeaks() const {
    return descriptorPeaksCount;
}

void ContourShape::GetPeak(unsigned char cIndex, unsigned short & iX, unsigned short & iY) {
    unsigned char cOffset = cIndex * 2;

    if (cIndex < descriptorPeaksCount) {
        iX = descriptorPeaks[cOffset];
        iY = descriptorPeaks[cOffset + 1];
    }
    else {
        iX = iY = 0;
    }
}

ContourShape::~ContourShape() = default;
