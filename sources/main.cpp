#include "Mpeg7.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

void printUsage(const char* programName) {
    std::cout << "Usage:" << std::endl;
    std::cout << "  Extract descriptor: " << programName << " extract <descriptor_type> <image_path> [param_name param_value ...]" << std::endl;
    std::cout << "  Calculate distance: " << programName << " distance <xml_file1> <xml_file2> [param_name param_value ...]" << std::endl;
    std::cout << "Descriptor types:" << std::endl;
    std::cout << "  1 - Dominant Color" << std::endl;
    std::cout << "  2 - Scalable Color" << std::endl;
    std::cout << "  3 - Color Layout" << std::endl;
    std::cout << "  4 - Color Structure" << std::endl;
    std::cout << "  5 - CT Browsing" << std::endl;
    std::cout << "  6 - Homogeneous Texture" << std::endl;
    std::cout << "  7 - Texture Browsing" << std::endl;
    std::cout << "  8 - Edge Histogram" << std::endl;
    std::cout << "  9 - Region Shape" << std::endl;
    std::cout << "  10 - Contour Shape" << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  Extract: " << programName << " extract 3 image.jpg NumberOfYCoeff 64 NumberOfCCoeff 64" << std::endl;
    std::cout << "  Distance: " << programName << " distance descriptor1.xml descriptor2.xml" << std::endl;
}

int main(const int argc, char* argv[]) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];
    
    if (command == "extract") {
        // Extract descriptor mode
        if (argc < 4) {
            std::cerr << "Error: Not enough arguments for extract command." << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        
        // Parse descriptor type
        int descriptorTypeInt = std::stoi(argv[2]);
        if (descriptorTypeInt < 1 || descriptorTypeInt > 10) {
            std::cerr << "Error: Invalid descriptor type. Must be between 1 and 10." << std::endl;
            printUsage(argv[0]);
            return 1;
        }

        const auto descriptorType = static_cast<DescriptorType>(descriptorTypeInt);
        const char* imagePath = argv[3];
        
        // Parse optional parameters
        std::vector<const char*> params;
        for (int i = 4; i < argc; i++) {
            params.push_back(argv[i]);
        }
        
        // Ensure params vector has an even number of elements (name-value pairs)
        if (params.size() % 2 != 0) {
            std::cerr << "Error: Parameters must be provided as name-value pairs." << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        
        // Null-terminate the params array
        params.push_back(nullptr);
        
        // Extract descriptor
        const char* result = extractDescriptor(descriptorType, imagePath, params.data());
        
        if (result) {
            std::cout << "Descriptor XML:" << std::endl;
            std::cout << result << std::endl;
            
            // Free the allocated memory
            freeResultPointer(const_cast<char*>(result));
        } else {
            std::cerr << "Error: Failed to extract descriptor." << std::endl;
        }
    }
    else if (command == "distance") {
        // Distance calculation mode
        if (argc < 4) {
            std::cerr << "Error: Not enough arguments for distance command." << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        
        // Get XML file paths
        const char* xmlFile1 = argv[2];
        const char* xmlFile2 = argv[3];
        
        // Read XML files
        std::ifstream file1(xmlFile1);
        std::ifstream file2(xmlFile2);
        
        if (!file1.is_open()) {
            std::cerr << "Error: Could not open file " << xmlFile1 << std::endl;
            return 1;
        }
        
        if (!file2.is_open()) {
            std::cerr << "Error: Could not open file " << xmlFile2 << std::endl;
            return 1;
        }
        
        std::string xml1Content((std::istreambuf_iterator<char>(file1)), std::istreambuf_iterator<char>());
        std::string xml2Content((std::istreambuf_iterator<char>(file2)), std::istreambuf_iterator<char>());
        
        file1.close();
        file2.close();
        
        // Parse optional parameters
        std::vector<const char*> params;
        for (int i = 4; i < argc; i++) {
            params.push_back(argv[i]);
        }
        
        // Ensure params vector has an even number of elements (name-value pairs)
        if (params.size() % 2 != 0) {
            std::cerr << "Error: Parameters must be provided as name-value pairs." << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        
        // Null-terminate the params array
        params.push_back(nullptr);
        
        // Calculate distance between descriptors
        const char* result = getDistance(xml1Content.c_str(), xml2Content.c_str(), params.data());
        
        if (result) {
            std::cout << "Distance: " << result << std::endl;
            
            // Free the allocated memory
            freeResultPointer(const_cast<char*>(result));
        } else {
            std::cerr << "Error: Failed to calculate distance." << std::endl;
        }
    }
    else {
        std::cerr << "Error: Unknown command '" << command << "'. Use 'extract' or 'distance'." << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    return 0;
}
