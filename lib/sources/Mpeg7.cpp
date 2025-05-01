#include "Mpeg7.h"

/* ~~~~~~~~~~~~~~~~ Message creation  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/** @brief
* Returns char * created from error
* integer code passed as argument
*
* @param error   - Error code
* @return char * - created char from error code */
const char * message(int error);
/** @brief
* Returns char * created from xml string
* passed as argument
*
* @param xml     - XML string
* @return char * - created char from xml */
const char * message(std::string xml);
/** @brief
* Returns char * created from distance string
* passed as argument.
*
* @param xml     - XML string
* @return char * - created char from double converted earlier to string */
const char * message(double distance);

/* ~~~~~~~~~~~~~~~~~ General method for extraction ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/** @brief
* Performs any descriptor extraction on internal image object, created
* from image path or image file data and with additional parameters. 
* Made to reduce code size (because extractDescriptor and 
* extractDescriptorFromData use almost same code.
* 
* @param descriptorType - descriptor type listed in DescriptorType
* @param image - reference to Image object
* @param params - user parameters
* @return const char * - result descriptor xml or error code converted to literal */
const char * mainExtraction(DescriptorType & descriptorType, Image & image, const char ** params);

/* ~~~~~~~~~~~~~~~~~ Method detecting descriptor type from XML ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/** @brief
* Detects descriptor type listed in DescriptorType, based on literal
* vaue of <Descriptor> node 'xsi:type' attribute in descirptor XML.
*
* @param xmlDescriptorType - attribute value
* @return DescriptorType   - detected descriptor type */
DescriptorType detectType(const char * xmlDescriptorType);

/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// MAIN LIBRARY FUNCTIONS /////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
const char * extractDescriptor(DescriptorType descriptorType, const char * imgURL, const char ** params) {
    // 1. Check parameters if exist
    if (params == NULL) {
        return message(PARAMS_NULL);
    }

    // 2. Load image, based on descriptor type and get loading result
    Image image;

    try {
        image.load(imgURL, IMAGE_UNCHANGED);
    }
    catch (ErrorCode exception) {
        return message(exception);
    }
    
    // 3. Extract
    return mainExtraction(descriptorType, image, params);
}

const char * extractDescriptorFromData(DescriptorType descriptorType, unsigned char * buffer, int size, const char ** params) {
    // 1. Check parameters if exist
    if (params == NULL) {
        return message(PARAMS_NULL);
    }

    // 2. Load image
    Image image;

    try {
        image.load(buffer, size, IMAGE_UNCHANGED);
    }
    catch (ErrorCode exception) {
        return message(exception);
    }

    // 3. Extract
    return mainExtraction(descriptorType, image, params);
}

const char * getDistance(const char * xml1, const char * xml2, const char ** params) {
	// 1. Check, if one of passed pointers is NULL
    if (xml1 == NULL || xml2 == NULL) {
        return message(XML_NULL);
    }
    
    // 2. Parse xmls
    XMLDocument document1, document2;

    XMLError parseResult1 = document1.Parse(xml1);
    XMLError parseResult2 = document2.Parse(xml2);

    // Check, if parsing xmls was done with no errors
    if (parseResult1 != XML_NO_ERROR || parseResult2 != XML_NO_ERROR) {
        return message(PARSE_ERROR);
    }

    // 3. Get important elements

    // Mpeg7
    XMLElement * mpeg7Element1 = document1.FirstChildElement("Mpeg7");
    XMLElement * mpeg7Element2 = document2.FirstChildElement("Mpeg7");

    if (mpeg7Element1 == NULL || mpeg7Element2 == NULL) {
        return message(MPEG7_NODE_NOT_FOUND);
    }

    // DescriptionUnit
    XMLElement * descriptionUnitElement1 = mpeg7Element1->FirstChildElement("DescriptionUnit");
    XMLElement * descriptionUnitElement2 = mpeg7Element2->FirstChildElement("DescriptionUnit");

    if (descriptionUnitElement1 == NULL || descriptionUnitElement2 == NULL) {
        return message(DESCRIPTION_UNIT_NODE_NOT_FOUND); 
    }

    // Descriptor
    XMLElement * descriptorElement1 = descriptionUnitElement1->FirstChildElement("Descriptor");
    XMLElement * descriptorElement2 = descriptionUnitElement2->FirstChildElement("Descriptor");

    if (descriptorElement1 == NULL || descriptorElement2 == NULL) {
        return message(DESCRIPTOR_NODE_NOT_FOUND);
    }

    /* For now, Mpeg7, DescriptionUnit and Descriptor element was found in both xmls
    It's time to check types of Descriptors */

    // 4. Try to get xsi:type attribute from both xmls
    const char * xml1DescriptorType = descriptorElement1->Attribute("xsi:type");
    const char * xml2DescriptorType = descriptorElement2->Attribute("xsi:type");

    if (xml1DescriptorType == NULL || xml1DescriptorType == NULL) {
        return message(TYPE_ATTRIBUTE_NOT_FOUND);
    }

    /* 5. Create enum types for both descriptors, based on value of xsi:type
    If value is not recognized, type will be NONE */
    DescriptorType type1 = detectType(xml1DescriptorType);
    DescriptorType type2 = detectType(xml2DescriptorType);

    if (type1 == NONE || type2 == NONE) {
        return message(XML_TYPE_NOT_RECOGNIZED);
    }

    // 6. Check type equality
    if (type1 != type2) {
        return message(XML_TYPES_NOT_EQUAL);
    }

    // 7. Types are equal, create objects
    Descriptor * descriptor1 = NULL;
    Descriptor * descriptor2 = NULL;
    DescriptorDistance * descriptorDistanceInterface = NULL;

    /* Switch by first type (for now type1 and type2 are equal)
    There is no need to check 'default' - types were already checked */
    try {
        switch (type1) {
            case COLOR_LAYOUT_D:
                descriptorDistanceInterface = new ColorLayoutDistance(); 
                descriptor1 = new ColorLayout(); 
                descriptor2 = new ColorLayout();
                break;
            case COLOR_STRUCTURE_D:
                descriptorDistanceInterface = new ColorStructureDistance(); 
                descriptor1 = new ColorStructure(); 
                descriptor2 = new ColorStructure();
                break;
            case CT_BROWSING_D:
                descriptorDistanceInterface = new CTBrowsingDistance(); 
                descriptor1 = new CTBrowsing(); 
                descriptor2 = new CTBrowsing();
                break;
            case DOMINANT_COLOR_D:
                descriptorDistanceInterface = new DominantColorDistance(); 
                descriptor1 = new DominantColor();
                descriptor2 = new DominantColor();
                break;
            case SCALABLE_COLOR_D:
                descriptorDistanceInterface = new ScalableColorDistance(); 
                descriptor1 = new ScalableColor(); 
                descriptor2 = new ScalableColor();
                break;
            case HOMOGENEOUS_TEXTURE_D:
                descriptorDistanceInterface = new HomogeneousTextureDistance(); 
                descriptor1 = new HomogeneousTexture(); 
                descriptor2 = new HomogeneousTexture();
                break;
            case TEXTURE_BROWSING_D:
                descriptorDistanceInterface = new TextureBrowsingDistance(); 
                descriptor1 = new TextureBrowsing(); 
                descriptor2 = new TextureBrowsing();
                break;
            case EDGE_HISTOGRAM_D:
                descriptorDistanceInterface = new EdgeHistogramDistance();
                descriptor1 = new EdgeHistogram(); 
                descriptor2 = new EdgeHistogram();
                break;
            case REGION_SHAPE_D:
                descriptorDistanceInterface = new RegionShapeDistance(); 
                descriptor1 = new RegionShape(); 
                descriptor2 = new RegionShape();
                break;
            case CONTOUR_SHAPE_D:
                descriptorDistanceInterface = new ContourShapeDistance(); 
                descriptor1 = new ContourShape(); 
                descriptor2 = new ContourShape();
                break;
            default:
                throw XML_TYPE_NOT_RECOGNIZED;
        }
    }
    catch (ErrorCode exception) {
        return message(exception);
    }

    // 8. Fill descriptor objects with xml data
    try {
        descriptor1->readFromXML(descriptorElement1);
        descriptor2->readFromXML(descriptorElement2);
    }
    catch (ErrorCode exception) {
        delete descriptor1;
        delete descriptor2;
        delete descriptorDistanceInterface;
        return message(exception);
    }
    // 9. Reading went good, it's time to calculate distance to distance result
    double distance = DBL_MAX;

    try {
        distance = descriptorDistanceInterface->getDistance(descriptor1, descriptor2, params);
    }
    catch (ErrorCode exception) {
        delete descriptor1;
        delete descriptor2;
        delete descriptorDistanceInterface;
        return message(exception);
    }

    delete descriptor1;
    delete descriptor2;
    delete descriptorDistanceInterface;

    // 10. Prepare final message to send
    const char * distanceMessage = message(distance);
   
    if (distanceMessage == NULL) {
        return message(DISTANCE_MESSAGE_NULL);
    }

    // 11. Return final message
    return distanceMessage; 
}

void freeResultPointer(char * ptr) {
    #ifdef DEBUG
        printf("freeing address: %p\n", ptr);
    #endif
    free(ptr);
}
/////////////////////////////////////////////////////////////////////////////////////////////////

const char * message(int error) {
    // Create string from integer code
    std::string msg_str = std::to_string(error);

    // Create char * from error string
    char * msg = new char[msg_str.size() + 1];
    strcpy(msg, msg_str.c_str());

    return msg;
}

const char * message(std::string xml) {
    // Create char * from xml string
    char * msg = new char[xml.size() + 1];
    strcpy(msg, xml.c_str());

    return msg;
}

const char * message(double distance) {
    // Stream double with good precision
    std::stringstream msg_str_stream;
    msg_str_stream << std::fixed << std::setprecision(17) << distance;

    // Create string from stream
    std::string msg_str = msg_str_stream.str();

    // Create char * from string
    char * msg = new char[msg_str.size() + 1];
    strcpy(msg, msg_str.c_str());

    return msg;
}

DescriptorType detectType(const char * xmlDescriptorType) {
    // Return DescriptorType based on descriptor type attribute fetched from xml
    return strcmp(xmlDescriptorType, "ColorLayoutType")              == 0 ? COLOR_LAYOUT_D        :
           strcmp(xmlDescriptorType, "ColorStructureType")           == 0 ? COLOR_STRUCTURE_D     :
           strcmp(xmlDescriptorType, "ColorTemperatureBrowsingType") == 0 ? CT_BROWSING_D         :
           strcmp(xmlDescriptorType, "DominantColorType")            == 0 ? DOMINANT_COLOR_D      :
           strcmp(xmlDescriptorType, "ScalableColorType")            == 0 ? SCALABLE_COLOR_D      :
           strcmp(xmlDescriptorType, "ContourShapeType")             == 0 ? CONTOUR_SHAPE_D       :
           strcmp(xmlDescriptorType, "RegionShapeType")              == 0 ? REGION_SHAPE_D        :
           strcmp(xmlDescriptorType, "EdgeHistogramType")            == 0 ? EDGE_HISTOGRAM_D      :
           strcmp(xmlDescriptorType, "HomogeneousTextureType")       == 0 ? HOMOGENEOUS_TEXTURE_D :
           strcmp(xmlDescriptorType, "TextureBrowsingType")          == 0 ? TEXTURE_BROWSING_D    : NONE;
}

const char * mainExtraction(DescriptorType & descriptorType, Image & image, const char ** params) {
    /* 1. Detect extractor type based on descriptor type.
    If type is not recognized, an exception will be thrown */
    DescriptorExtractor * extractor = NULL;

    try {
        extractor = descriptorType == DOMINANT_COLOR_D      ? (DescriptorExtractor *) new DominantColorExtractor()      :
                    descriptorType == SCALABLE_COLOR_D      ? (DescriptorExtractor *) new ScalableColorExtractor()      :
                    descriptorType == COLOR_LAYOUT_D        ? (DescriptorExtractor *) new ColorLayoutExtractor()        :
                    descriptorType == COLOR_STRUCTURE_D     ? (DescriptorExtractor *) new ColorStructureExtractor()     :
                    descriptorType == CT_BROWSING_D         ? (DescriptorExtractor *) new CTBrowsingExtractor()         :
                    descriptorType == HOMOGENEOUS_TEXTURE_D ? (DescriptorExtractor *) new HomogeneousTextureExtractor() :
                    descriptorType == TEXTURE_BROWSING_D    ? (DescriptorExtractor *) new TextureBrowsingExtractor()    :
                    descriptorType == EDGE_HISTOGRAM_D      ? (DescriptorExtractor *) new EdgeHistogramExtractor()      :
                    descriptorType == REGION_SHAPE_D        ? (DescriptorExtractor *) new RegionShapeExtractor()        :
                    descriptorType == CONTOUR_SHAPE_D       ? (DescriptorExtractor *) new ContourShapeExtractor()       :
                                                            throw UNRECOGNIZED_DESCRIPTOR_TYPE;
    }
    catch (ErrorCode exception) {
        return message(exception);
    }

    // 2. Extract
    Descriptor * descriptor = NULL;

    try {
        descriptor = extractor->extract(image, params);
    }
    catch (ErrorCode exception) {
        return message(exception);
    }

    // 3. Prepare final message to send
    const char * extractionMessage = NULL;

    try {
        extractionMessage = message(descriptor->generateXML());
    }
    catch (ErrorCode exception) {
        delete extractor;
        return message(exception);
    }

    delete extractor;

    if (extractionMessage == NULL) {
        return message(EXTRACTION_MESSAGE_NULL);
    }

    // 4. Return final message
    return extractionMessage;
}