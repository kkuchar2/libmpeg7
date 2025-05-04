#include "Mpeg7.h"

const char * message(int error);
const char * message(const std::string &xml);
const char * message(double distance);

const char * mainExtraction(DescriptorType & descriptorType, Image & image, const char ** params);

DescriptorType detectType(const char * xmlDescriptorType);

const char * extractDescriptor(DescriptorType descriptorType, const char * imgURL, const char ** params) {
    if (params == nullptr) {
        return message(PARAMS_NULL);
    }
    Image image;

    try {
        image.load(imgURL, IMAGE_UNCHANGED);
    }
    catch (ErrorCode exception) {
        return message(exception);
    }
    return mainExtraction(descriptorType, image, params);
}

const char * extractDescriptorFromData(DescriptorType descriptorType, unsigned char * buffer, int size, const char ** params) {
    if (params == nullptr) {
        return message(PARAMS_NULL);
    }

    Image image;

    try {
        image.load(buffer, size, IMAGE_UNCHANGED);
    }
    catch (ErrorCode exception) {
        return message(exception);
    }

    return mainExtraction(descriptorType, image, params);
}

const char * getDistance(const char * xml1, const char * xml2, const char ** params) {
    if (xml1 == nullptr || xml2 == nullptr) {
        return message(XML_NULL);
    }
    
    XMLDocument document1, document2;

    XMLError parseResult1 = document1.Parse(xml1);
    XMLError parseResult2 = document2.Parse(xml2);

    if (parseResult1 != XML_NO_ERROR || parseResult2 != XML_NO_ERROR) {
        return message(PARSE_ERROR);
    }

    XMLElement * mpeg7Element1 = document1.FirstChildElement("Mpeg7");
    XMLElement * mpeg7Element2 = document2.FirstChildElement("Mpeg7");

    if (mpeg7Element1 == nullptr || mpeg7Element2 == nullptr) {
        return message(MPEG7_NODE_NOT_FOUND);
    }

    XMLElement * descriptionUnitElement1 = mpeg7Element1->FirstChildElement("DescriptionUnit");
    XMLElement * descriptionUnitElement2 = mpeg7Element2->FirstChildElement("DescriptionUnit");

    if (descriptionUnitElement1 == nullptr || descriptionUnitElement2 == nullptr) {
        return message(DESCRIPTION_UNIT_NODE_NOT_FOUND); 
    }

    XMLElement * descriptorElement1 = descriptionUnitElement1->FirstChildElement("Descriptor");
    XMLElement * descriptorElement2 = descriptionUnitElement2->FirstChildElement("Descriptor");

    if (descriptorElement1 == nullptr || descriptorElement2 == nullptr) {
        return message(DESCRIPTOR_NODE_NOT_FOUND);
    }

    const char * xml1DescriptorType = descriptorElement1->Attribute("xsi:type");
    const char * xml2DescriptorType = descriptorElement2->Attribute("xsi:type");

    if (xml1DescriptorType == nullptr || xml1DescriptorType == nullptr) {
        return message(TYPE_ATTRIBUTE_NOT_FOUND);
    }

    DescriptorType type1 = detectType(xml1DescriptorType);
    DescriptorType type2 = detectType(xml2DescriptorType);

    if (type1 == NONE || type2 == NONE) {
        return message(XML_TYPE_NOT_RECOGNIZED);
    }

    if (type1 != type2) {
        return message(XML_TYPES_NOT_EQUAL);
    }

    Descriptor * descriptor1 = nullptr;
    Descriptor * descriptor2 = nullptr;
    DescriptorDistance * descriptorDistanceInterface = nullptr;

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

    const char * distanceMessage = message(distance);
   
    if (distanceMessage == nullptr) {
        return message(DISTANCE_MESSAGE_NULL);
    }
    return distanceMessage; 
}

void freeResultPointer(char * ptr) {
    #ifdef DEBUG
        printf("freeing address: %p\n", ptr);
    #endif
    free(ptr);
}

const char * message(int error) {
    const std::string msg_str = std::to_string(error);

    const auto msg = new char[msg_str.size() + 1];
    strcpy(msg, msg_str.c_str());

    return msg;
}

const char * message(const std::string &xml) {
    const auto msg = new char[xml.size() + 1];
    strcpy(msg, xml.c_str());
    return msg;
}

const char * message(double distance) {
    std::stringstream msg_str_stream;
    msg_str_stream << std::fixed << std::setprecision(17) << distance;

    const std::string msg_str = msg_str_stream.str();

    const auto msg = new char[msg_str.size() + 1];
    strcpy(msg, msg_str.c_str());

    return msg;
}

DescriptorType detectType(const char * xmlDescriptorType) {
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
    DescriptorExtractor * extractor = nullptr;

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
    Descriptor * descriptor = nullptr;

    try {
        descriptor = extractor->extract(image, params);
    }
    catch (ErrorCode exception) {
        return message(exception);
    }

    const char * extractionMessage = nullptr;

    try {
        extractionMessage = message(descriptor->generateXML());
    }
    catch (ErrorCode exception) {
        delete extractor;
        return message(exception);
    }

    delete extractor;

    if (extractionMessage == nullptr) {
        return message(EXTRACTION_MESSAGE_NULL);
    }
    return extractionMessage;
}