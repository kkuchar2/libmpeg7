#include "Mpeg7.h"
#include <iostream>
#include <string>
#include <vector>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <descriptor_type> <image_path> [param_name param_value ...]" << std::endl;
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
    std::cout << "Example: " << programName << " 3 image.jpg NumberOfYCoeff 64 NumberOfCCoeff 64" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }

    // Parse descriptor type
    int descriptorTypeInt = std::stoi(argv[1]);
    if (descriptorTypeInt < 1 || descriptorTypeInt > 10) {
        std::cerr << "Error: Invalid descriptor type. Must be between 1 and 10." << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    DescriptorType descriptorType = static_cast<DescriptorType>(descriptorTypeInt);
    const char* imagePath = argv[2];
    
    // Parse optional parameters
    std::vector<const char*> params;
    for (int i = 3; i < argc; i++) {
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
    
    return 0;
}
