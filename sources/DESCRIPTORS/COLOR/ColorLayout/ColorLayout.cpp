#include "ColorLayout.h"

ColorLayout::ColorLayout() {
}

void ColorLayout::loadParameters(const char ** params) {

    if (params == nullptr) {
        return; // default value remains default
    }

    /* SIZE | PARAM COMBINATION                           
    -----------------------------------------------------------------------
     0 |  [NULL]
     2 |  [NumberOfYCoeff, value, NULL]
     2 |  [NumberOfCCoeff, value, NULL]
     4 |  [NumberOfYCoeff, value, NumberOfCCoeff, value, NULL]
     4 |  [NumberOfCCoeff, value, NumberOfYCoeff, value, NULL]
    ----------------------------------------------------------------------- */

    // Count parameters size
    int size = 0;
    for (int i = 0; params[i] != nullptr; i++) {
        size++;
    }

    // Check size
    if (size != 0 && size != 2 && size != 4) {
        throw COL_LAY_PARAMS_NUMBER_ERROR;
    }
    
    if (size == 0) {
        return; // default value remains default
    }

    // At this point parameters are size 2 or 4
    for (int i = 0; params[i] != nullptr; i += 2) {
        if (params[i] == nullptr || params[i + 1] == nullptr) {
            break; // default value remains default
        }
        
        std::string p1(params[i]);
        std::string p2(params[i + 1]);

        //----- Color layout parameters ------------------------------------
        // NumberOfYCoeff - 3,6,10,15,21,28,64
        // NumberOfCCoeff - 3,6,10,15,21,28,64
        //------------------------------------------------------------------

        if (!p1.compare("NumberOfYCoeff")) {
            numberOfYCoefficients = !p2.compare("3")  || !p2.compare("6")  || 
                                    !p2.compare("10") || !p2.compare("15") || 
                                    !p2.compare("21") || !p2.compare("28") || 
                                    !p2.compare("64") ? std::stoi(p2) : throw COL_LAY_PARAMS_VALUE_ERROR;         
        }
        else if (!p1.compare("NumberOfCCoeff")) {       
            numberOfCCoefficients = !p2.compare("3")  || !p2.compare("6")  || 
                                    !p2.compare("10") || !p2.compare("15") || 
                                    !p2.compare("21") || !p2.compare("28") || 
                                    !p2.compare("64") ? std::stoi(p2) : throw COL_LAY_PARAMS_VALUE_ERROR;          
        }
        else {
            throw COL_LAY_PARAMS_NAME_ERROR;
        }
    }
}

std::string ColorLayout::generateXML() {
    XMLDocument xmlDoc;

    XMLDeclaration * p_Declaration = xmlDoc.NewDeclaration("xml version='1.0' encoding='ISO-8859-1' ");

    XMLElement * p_Root = xmlDoc.NewElement("Mpeg7");
    XMLElement * p_DescriptionUnit = xmlDoc.NewElement("DescriptionUnit");
    XMLElement * p_Descriptor = xmlDoc.NewElement("Descriptor");

    XMLElement * p_YDCCoeff = xmlDoc.NewElement("YDCCoeff");
    XMLElement * p_CbDCCoeff = xmlDoc.NewElement("CbDCCoeff");
    XMLElement * p_CrDCCoeff = xmlDoc.NewElement("CrDCCoeff");

    std::string y_AC_name  = "YACCoeff"  + std::to_string(numberOfYCoefficients - 1);
    std::string cb_AC_name = "CbACCoeff" + std::to_string(numberOfCCoefficients - 1);
    std::string cr_AC_name = "CrACCoeff" + std::to_string(numberOfCCoefficients - 1);

    XMLElement * p_YACCoeff = xmlDoc.NewElement(y_AC_name.c_str());
    XMLElement * p_CbACCoeff = xmlDoc.NewElement(cb_AC_name.c_str());
    XMLElement * p_CrACCoeff = xmlDoc.NewElement(cr_AC_name.c_str());

    p_Root->SetAttribute("xmlns", "urn:mpeg:mpeg7:schema:2001");
    p_Root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    p_DescriptionUnit->SetAttribute("xsi:type", "DescriptorCollectionType");
    p_Descriptor->SetAttribute("xsi:type", "ColorLayoutType");

    p_YDCCoeff->SetText(yCoefficients[0]);   // YDCCoeff
    p_CbDCCoeff->SetText(cbCoefficients[0]); // CbDCCoeff
    p_CrDCCoeff->SetText(crCoefficients[0]); // CrDCCoeff

    std::string yACstring = "";
    std::string cbACstring = "";
    std::string crACstring = "";

    for (int i = 1; i < numberOfYCoefficients; i++) {
        yACstring.append(std::to_string(yCoefficients[i]) + " ");
    }

    for (int i = 1; i < numberOfCCoefficients; i++) {
        cbACstring.append(std::to_string(cbCoefficients[i]) + " ");
        crACstring.append(std::to_string(crCoefficients[i]) + " ");
    }

    p_YACCoeff->SetText(yACstring.c_str());   // YACCoeff
    p_CbACCoeff->SetText(cbACstring.c_str()); // CbACCoeff
    p_CrACCoeff->SetText(crACstring.c_str()); // CrACCoeff

    xmlDoc.InsertFirstChild(p_Declaration);
    xmlDoc.InsertEndChild(p_Root);

    p_Root->InsertEndChild(p_DescriptionUnit);
    p_DescriptionUnit->InsertEndChild(p_Descriptor);

    p_Descriptor->InsertEndChild(p_YDCCoeff);
    p_Descriptor->InsertEndChild(p_CbDCCoeff);
    p_Descriptor->InsertEndChild(p_CrDCCoeff);

    p_Descriptor->InsertEndChild(p_YACCoeff);
    p_Descriptor->InsertEndChild(p_CbACCoeff);
    p_Descriptor->InsertEndChild(p_CrACCoeff);

    XMLPrinter xmlPrinter;
    xmlDoc.Accept(&xmlPrinter);
    return static_cast<const char *>(xmlPrinter.CStr());
}

void ColorLayout::readFromXML(XMLElement * descriptorElement) {
    XMLElement * p_Y_DC_Coeff  = descriptorElement->FirstChildElement("YDCCoeff");
    XMLElement * p_Cb_DC_Coeff = descriptorElement->FirstChildElement("CbDCCoeff");
    XMLElement * p_Cr_DC_Coeff = descriptorElement->FirstChildElement("CrDCCoeff");

    if (p_Y_DC_Coeff == nullptr || p_Cb_DC_Coeff == nullptr || p_Cr_DC_Coeff == nullptr) {
        throw COL_LAY_DC_NODE_ERROR;
    }

    XMLElement * p_Y_AC_Coeff  = nullptr;
    XMLElement * p_Cb_AC_Coeff = nullptr;
    XMLElement * p_Cr_AC_Coeff = nullptr;

    XMLElement * p_Y_AC_Coeff2   = descriptorElement->FirstChildElement("YACCoeff2");
    XMLElement * p_Y_AC_Coeff5   = descriptorElement->FirstChildElement("YACCoeff5");
    XMLElement * p_Y_AC_Coeff9   = descriptorElement->FirstChildElement("YACCoeff9");
    XMLElement * p_Y_AC_Coeff14  = descriptorElement->FirstChildElement("YACCoeff14");
    XMLElement * p_Y_AC_Coeff20  = descriptorElement->FirstChildElement("YACCoeff20");
    XMLElement * p_Y_AC_Coeff27  = descriptorElement->FirstChildElement("YACCoeff27");
    XMLElement * p_Y_AC_Coeff63  = descriptorElement->FirstChildElement("YACCoeff63");

    XMLElement * p_Cb_AC_Coeff2  = descriptorElement->FirstChildElement("CbACCoeff2");
    XMLElement * p_Cb_AC_Coeff5  = descriptorElement->FirstChildElement("CbACCoeff5");
    XMLElement * p_Cb_AC_Coeff9  = descriptorElement->FirstChildElement("CbACCoeff9");
    XMLElement * p_Cb_AC_Coeff14 = descriptorElement->FirstChildElement("CbACCoeff14");
    XMLElement * p_Cb_AC_Coeff20 = descriptorElement->FirstChildElement("CbACCoeff20");
    XMLElement * p_Cb_AC_Coeff27 = descriptorElement->FirstChildElement("CbACCoeff27");
    XMLElement * p_Cb_AC_Coeff63 = descriptorElement->FirstChildElement("CbACCoeff63");

    XMLElement * p_Cr_AC_Coeff2  = descriptorElement->FirstChildElement("CrACCoeff2");
    XMLElement * p_Cr_AC_Coeff5  = descriptorElement->FirstChildElement("CrACCoeff5");
    XMLElement * p_Cr_AC_Coeff9  = descriptorElement->FirstChildElement("CrACCoeff9");
    XMLElement * p_Cr_AC_Coeff14 = descriptorElement->FirstChildElement("CrACCoeff14");
    XMLElement * p_Cr_AC_Coeff20 = descriptorElement->FirstChildElement("CrACCoeff20");
    XMLElement * p_Cr_AC_Coeff27 = descriptorElement->FirstChildElement("CrACCoeff27");
    XMLElement * p_Cr_AC_Coeff63 = descriptorElement->FirstChildElement("CrACCoeff63");

    p_Y_AC_Coeff  = p_Y_AC_Coeff2   != nullptr
                        ? p_Y_AC_Coeff2   : 
                    p_Y_AC_Coeff5   != nullptr
                        ? p_Y_AC_Coeff5   :
                    p_Y_AC_Coeff9   != nullptr
                        ? p_Y_AC_Coeff9   :
                    p_Y_AC_Coeff14  != nullptr
                        ? p_Y_AC_Coeff14  :
                    p_Y_AC_Coeff20  != nullptr
                        ? p_Y_AC_Coeff20  :
                    p_Y_AC_Coeff27  != nullptr
                        ? p_Y_AC_Coeff27  :
                    p_Y_AC_Coeff63  != nullptr
                        ? p_Y_AC_Coeff63  : throw COL_LAY_AC_NODE_ERROR;

    p_Cb_AC_Coeff = p_Cb_AC_Coeff2  != nullptr
                        ? p_Cb_AC_Coeff2  : 
                    p_Cb_AC_Coeff5  != nullptr
                        ? p_Cb_AC_Coeff5  :
                    p_Cb_AC_Coeff9  != nullptr
                        ? p_Cb_AC_Coeff9  :
                    p_Cb_AC_Coeff14 != nullptr
                        ? p_Cb_AC_Coeff14 :
                    p_Cb_AC_Coeff20 != nullptr
                        ? p_Cb_AC_Coeff20 :
                    p_Cb_AC_Coeff27 != nullptr
                        ? p_Cb_AC_Coeff27 :
                    p_Cb_AC_Coeff63 != nullptr
                        ? p_Cb_AC_Coeff63 : throw COL_LAY_AC_NODE_ERROR;

    p_Cr_AC_Coeff = p_Cr_AC_Coeff2  != nullptr
                        ? p_Cr_AC_Coeff2  : 
                    p_Cr_AC_Coeff5  != nullptr
                        ? p_Cr_AC_Coeff5  :
                    p_Cr_AC_Coeff9  != nullptr
                        ? p_Cr_AC_Coeff9  :
                    p_Cr_AC_Coeff14 != nullptr
                        ? p_Cr_AC_Coeff14 :
                    p_Cr_AC_Coeff20 != nullptr
                        ? p_Cr_AC_Coeff20 :
                    p_Cr_AC_Coeff27 != nullptr
                        ? p_Cr_AC_Coeff27 :
                    p_Cr_AC_Coeff63 != nullptr
                        ? p_Cr_AC_Coeff63 : throw COL_LAY_AC_NODE_ERROR;

    std::stringstream ss_Y_DC_Coeff (p_Y_DC_Coeff ->GetText());
    std::stringstream ss_Cb_DC_Coeff(p_Cb_DC_Coeff->GetText());
    std::stringstream ss_Cr_DC_Coeff(p_Cr_DC_Coeff->GetText());

    std::stringstream ss_Y_AC_Coeff (p_Y_AC_Coeff ->GetText());
    std::stringstream ss_Cb_AC_Coeff(p_Cb_AC_Coeff->GetText());
    std::stringstream ss_Cr_AC_Coeff(p_Cr_AC_Coeff->GetText());
    
    std::vector<int> y_DC_Coeff_Values;
    std::vector<int> cb_DC_Coeff_Values;
    std::vector<int> cr_DC_Coeff_Values;

    std::vector<int> y_AC_Coeff_Values;
    std::vector<int> cb_AC_Coeff_Values;
    std::vector<int> cr_AC_Coeff_Values;

    int i;

    //  Load DC Coefficients
    while (ss_Y_DC_Coeff >> i) {            // Load Y DC coefficients
        y_DC_Coeff_Values.push_back(i);

        if (ss_Y_DC_Coeff.peek() == ' ') {
            ss_Y_DC_Coeff.ignore();
        }
    }

    ss_Y_DC_Coeff.clear();

    while (ss_Cb_DC_Coeff >> i) {           // Load Cb DC coefficients
        cb_DC_Coeff_Values.push_back(i);

        if (ss_Cb_DC_Coeff.peek() == ' ') {
            ss_Cb_DC_Coeff.ignore();
        }
    }

    ss_Cb_DC_Coeff.clear();

    while (ss_Cr_DC_Coeff >> i) {           // Load Cr DC coefficients
        cr_DC_Coeff_Values.push_back(i);

        if (ss_Cr_DC_Coeff.peek() == ' ') {
            ss_Cr_DC_Coeff.ignore();
        }
    }

    ss_Cr_DC_Coeff.clear();

    //  Load AC Coefficients
    while (ss_Y_AC_Coeff >> i) {            // Load Y AC coefficients
        y_AC_Coeff_Values.push_back(i);

        if (ss_Y_AC_Coeff.peek() == ' ') {
            ss_Y_AC_Coeff.ignore();
        }
    }

    ss_Y_AC_Coeff.clear();

    while (ss_Cb_AC_Coeff >> i) {           // Load Cb DC coefficients
        cb_AC_Coeff_Values.push_back(i);

        if (ss_Cb_AC_Coeff.peek() == ' ') {
            ss_Cb_AC_Coeff.ignore();
        }
    }

    ss_Cb_AC_Coeff.clear();

    while (ss_Cr_AC_Coeff >> i) {           // Load Cr DC coefficients
        cr_AC_Coeff_Values.push_back(i);

        if (ss_Cr_DC_Coeff.peek() == ' ') {
            ss_Cr_DC_Coeff.ignore();
        }
    }

    ss_Cr_AC_Coeff.clear();

    // Count Y Coefficients
    int xmlNumberOfYCoefficients = static_cast<int>(y_DC_Coeff_Values.size() + y_AC_Coeff_Values.size());

    numberOfYCoefficients = xmlNumberOfYCoefficients == 3  || xmlNumberOfYCoefficients == 6  ||
                            xmlNumberOfYCoefficients == 10 || xmlNumberOfYCoefficients == 15 ||
                            xmlNumberOfYCoefficients == 21 || xmlNumberOfYCoefficients == 28 || 
                            xmlNumberOfYCoefficients == 64 ? xmlNumberOfYCoefficients : throw COL_LAY_WRONG_COEFF_NUMBER;

    //  Count Cb and Cr Coefficients
    int xmlNumberOfCbCoefficients = static_cast<int>(cb_DC_Coeff_Values.size() + cb_AC_Coeff_Values.size());
    int xmlNumberOfCrCoefficients = static_cast<int>(cr_DC_Coeff_Values.size() + cr_AC_Coeff_Values.size());

    if (xmlNumberOfCbCoefficients == xmlNumberOfCrCoefficients) {
        numberOfCCoefficients = xmlNumberOfCbCoefficients == 3  || xmlNumberOfCbCoefficients == 6  ||
                                xmlNumberOfCbCoefficients == 10 || xmlNumberOfCbCoefficients == 15 ||
                                xmlNumberOfCbCoefficients == 21 || xmlNumberOfCbCoefficients == 28 ||
                                xmlNumberOfCbCoefficients == 64 ? xmlNumberOfCbCoefficients :  throw COL_LAY_WRONG_COEFF_NUMBER;
    }
    else {
        throw COL_LAY_COEFF_NUMBER_DIFFERENT;
    }
    
    // Fill Descriptor with data
    allocateYCoefficients();
    allocateCbCoefficients();
    allocateCrCoefficients();

    yCoefficients[0]  = y_DC_Coeff_Values[0];
    cbCoefficients[0] = cb_DC_Coeff_Values[0];
    crCoefficients[0] = cr_DC_Coeff_Values[0];

    for (unsigned int k = 0; k < y_AC_Coeff_Values.size(); k++) {
        yCoefficients[k + 1] = y_AC_Coeff_Values[k];
    }

    for (unsigned int k = 0; k < cb_AC_Coeff_Values.size(); k++) {
        cbCoefficients[k + 1] = cb_AC_Coeff_Values[k];
        crCoefficients[k + 1] = cr_AC_Coeff_Values[k];
    }
}

void ColorLayout::allocateYCoefficients() {
	yCoefficients = new int[numberOfYCoefficients];
}

void ColorLayout::allocateCbCoefficients() {
	cbCoefficients = new int[numberOfCCoefficients];
}

void ColorLayout::allocateCrCoefficients() {
	crCoefficients = new int[numberOfCCoefficients];
}

void ColorLayout::setNumberOfYCoefficients(int yCoeffNumber) {
	numberOfYCoefficients = yCoeffNumber;
}

void ColorLayout::setNumberOfCCoefficients(int cCoeffNumber) {
	numberOfCCoefficients = cCoeffNumber;
}

void ColorLayout::setYCoefficient(int index, int value) {
    yCoefficients[index] = value;
}

void ColorLayout::setCbCoefficient(int index, int value) {
    cbCoefficients[index] = value;
}

void ColorLayout::setCrCoefficient(int index, int value) {
    crCoefficients[index] = value;
}

int ColorLayout::getNumberOfYCoeffcients() {
    return numberOfYCoefficients;
}

int ColorLayout::getNumberOfCCoefficients() {
    return numberOfCCoefficients;
}

int * ColorLayout::getYCoefficients() {
    return yCoefficients;
}

int * ColorLayout::getCbCoefficients() {
    return cbCoefficients;
}

int * ColorLayout::getCrCoefficients() {
    return crCoefficients;
}

ColorLayout::~ColorLayout() {
	if (yCoefficients) {
		delete[] yCoefficients;
	}
	if (cbCoefficients) {
		delete[] cbCoefficients;
	}
	if (crCoefficients) {
		delete[] crCoefficients;
	}
}