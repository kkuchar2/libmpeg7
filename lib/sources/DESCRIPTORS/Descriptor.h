/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/** @file Descriptor.h
* @brief Pure abstract class representing general Descriptor object.
* Each descriptor of 10 supported by the library descriptor type inherits methods from 
* that class, so it is possible to use polymorphism in main extraction 
* and distance calculation methods.
*  
*  @author Krzysztof Lech Kucharski */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef _DESCRIPTOR_H 
#define _DESCRIPTOR_H 

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "../TOOLS/ErrorCode.h"
#include "../TOOLS/Image/Image.h"
#include "../TOOLS/XML/tinyxml2.h"

using namespace tinyxml2;

class Descriptor { 
    public:
        /** @brief
        * Reads descriptor data from its extraction XML
        * @param descriptorElement - <Descriptor> element of XML data */
        virtual void readFromXML(XMLElement * descriptorElement) = 0;
        /** @brief
        * Generates extraction XML from descriptor data
        * @return std::string - created XML string */
        virtual std::string generateXML() = 0;
        /** @brief
        * General destructor */
        virtual ~ Descriptor() = 0;
};

inline Descriptor::~Descriptor () {
}

#endif