#include "Image.h"
#include "../ErrorCode.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Image::Image() : imageData(nullptr), imageWidth(0), imageHeight(0), imageSize(0), totalImageSize(0), channels(0), depth(0) {
}

void Image::load(unsigned char* data, const int size, const LoadMode decode_mode) {
    std::cout << "Loading image of size: " << size << std::endl;

    int desiredChannels = 0; // 0 means keep original format

    switch (decode_mode) {
        case IMAGE_UNCHANGED:
            desiredChannels = 0; // Keep original format
            break;
        case IMAGE_COLOR:
            desiredChannels = 3; // Force 3 channels (RGB)
            break;
        case IMAGE_GRAYSCALE:
            desiredChannels = 1; // Force 1 channel (Grayscale)
            break;
        default:
            desiredChannels = 0; // Keep original format
            break;
    }

    // Free previous image if any
    if (imageData != nullptr) {
        stbi_image_free(imageData);
        imageData = nullptr;
    }

    // Load image from memory
    int width, height, channels_in_file;
    imageData = stbi_load_from_memory(data, size, &width, &height, &channels_in_file, desiredChannels);
    
    // Check if image was loaded properly
    if (imageData == nullptr) {
        std::cout << "Failed to load image: " << stbi_failure_reason() << std::endl;
        throw CANNOT_OPEN_IMAGE;
    }

    // Save image information
    imageWidth = width;
    imageHeight = height;
    imageSize = imageWidth * imageHeight;
    
    // If we specified channels, use that value, otherwise use what was in the file
    channels = (desiredChannels != 0) ? desiredChannels : channels_in_file;
    
    // Set transparency and total size based on channels
    if (channels == 4) { // RGBA
        transparencyPresent = true;
        totalImageSize = 4 * imageSize;
    }
    else if (channels == 3) { // RGB
        transparencyPresent = false;
        totalImageSize = 3 * imageSize;
    }
    else if (channels == 2) { // GrayAlpha
        transparencyPresent = true;
        totalImageSize = 2 * imageSize;
    }
    else if (channels == 1) { // Gray
        transparencyPresent = false;
        totalImageSize = imageSize;
    }
    else {
        stbi_image_free(imageData);
        imageData = nullptr;
        throw IMAGE_CHANNELS_NOT_SUPPORTED;
    }

    depth = 8; // stb_image always returns 8-bit channels
}

void Image::load(const char* filename, const LoadMode load_mode) {
    int desiredChannels = 0; // 0 means keep original format
    
    switch (load_mode) {
        // Load image as it is
        case IMAGE_UNCHANGED:
            desiredChannels = 0; // Keep original format
            break;
        case IMAGE_COLOR:
            desiredChannels = 3; // Force 3 channels (RGB)
            break;
        case IMAGE_GRAYSCALE:
            desiredChannels = 1; // Force 1 channel (Grayscale)
            break;
        default:
            desiredChannels = 0; // Keep original format
            break;
    }

    // Free previous image if any
    if (imageData != nullptr) {
        stbi_image_free(imageData);
        imageData = nullptr;
    }

    // Load image from file
    int width, height, channels_in_file;
    imageData = stbi_load(filename, &width, &height, &channels_in_file, desiredChannels);
    
    // Check if image was loaded properly
    if (imageData == nullptr) {
        std::cout << "Failed to load image: " << stbi_failure_reason() << std::endl;
        throw CANNOT_OPEN_IMAGE;
    }

    // Save image information
    imageWidth = width;
    imageHeight = height;
    imageSize = imageWidth * imageHeight;
    
    // If we specified channels, use that value, otherwise use what was in the file
    channels = (desiredChannels != 0) ? desiredChannels : channels_in_file;
    
    // Set transparency and total size based on channels
    if (channels == 4) { // RGBA
        transparencyPresent = true;
        totalImageSize = 4 * imageSize;
    }
    else if (channels == 3) { // RGB
        transparencyPresent = false;
        totalImageSize = 3 * imageSize;
    }
    else if (channels == 2) { // GrayAlpha
        transparencyPresent = true;
        totalImageSize = 2 * imageSize;
    }
    else if (channels == 1) { // Gray
        transparencyPresent = false;
        totalImageSize = imageSize;
    }
    else {
        stbi_image_free(imageData);
        imageData = nullptr;
        throw IMAGE_CHANNELS_NOT_SUPPORTED;
    }

    depth = 8; // stb_image always returns 8-bit channels
}

unsigned char * Image::getChannel_R() {
    // Image size is pixel count in an image (width * height)
    const auto rChannel = new unsigned char[imageSize];
    
    if (!imageData) {
        return nullptr;
    }

    // RGBA format in stb_image is R,G,B,A order
    if (channels == 4) { // RGBA
        for (int i = 0; i < imageSize; i++) {
            rChannel[i] = imageData[i * 4]; // R is first byte
        }
    }
    // RGB format in stb_image is R,G,B order
    else if (channels == 3) { 
        for (int i = 0; i < imageSize; i++) {
            rChannel[i] = imageData[i * 3]; // R is first byte
        }
    }
    // GrayAlpha
    else if (channels == 2) { 
        for (int i = 0; i < imageSize; i++) {
            rChannel[i] = imageData[i * 2]; // Gray value is first byte
        }
    }
    // Gray
    else if (channels == 1) { 
        for (int i = 0; i < imageSize; i++) {
            rChannel[i] = imageData[i]; // R | G | B get same grayscale value
        }
    }
    // Other cases do not exist - it has been checked while image load process
    return rChannel;
}

unsigned char * Image::getChannel_G() {
    const auto gChannel = new unsigned char[imageSize];

    if (!imageData) {
        return nullptr;
    }

    // RGBA format in stb_image is R,G,B,A order
    if (channels == 4) { // RGBA
        for (int i = 0; i < imageSize; i++) {
            gChannel[i] = imageData[i * 4 + 1]; // G is second byte
        }
    }
    // RGB format in stb_image is R,G,B order
    else if (channels == 3) { 
        for (int i = 0; i < imageSize; i++) {
            gChannel[i] = imageData[i * 3 + 1]; // G is second byte
        }
    }
    // GrayAlpha
    else if (channels == 2) { 
        for (int i = 0; i < imageSize; i++) {
            gChannel[i] = imageData[i * 2 + 1]; // Alpha value is second byte
        }
    }
    // Gray - return the same grayscale value
    else if (channels == 1) { 
        for (int i = 0; i < imageSize; i++) {
            gChannel[i] = imageData[i]; // R | G | B get same grayscale value
        }
    }
    // Other cases do not exist - it has been checked while image load process
    return gChannel;
}

unsigned char * Image::getChannel_B() {
    const auto bChannel = new unsigned char[imageSize];

    if (!imageData) {
        return nullptr;
    }

    // RGBA format in stb_image is R,G,B,A order
    if (channels == 4) { // RGBA
        for (int i = 0; i < imageSize; i++) {
            bChannel[i] = imageData[i * 4 + 2]; // B is third byte
        }
    }
    // RGB format in stb_image is R,G,B order
    else if (channels == 3) { 
        for (int i = 0; i < imageSize; i++) {
            bChannel[i] = imageData[i * 3 + 2]; // B is third byte
        }
    }
    // GrayAlpha - for consistency, return the grayscale value
    else if (channels == 2) { 
        for (int i = 0; i < imageSize; i++) {
            bChannel[i] = imageData[i * 2]; // Gray value is first byte
        }
    }
    // Gray - return the same grayscale value
    else if (channels == 1) { 
        for (int i = 0; i < imageSize; i++) {
            bChannel[i] = imageData[i]; // R | G | B get same grayscale value
        }
    }
    // Other cases do not exist - it has been checked while image load process
    return bChannel;
}

unsigned char * Image::getChannel_A() {
    unsigned char * aChannel = nullptr;

    if (!imageData) {
        return nullptr;
    }

    if (channels == 4) { // RGBA
        aChannel = new unsigned char[imageSize];

        for (int i = 0; i < imageSize; i++) {
            aChannel[i] = imageData[i * 4 + 3]; // A is fourth byte
            
            if (minAlpha == -1) {
                minAlpha = aChannel[i];
            }
            else if (aChannel[i] < minAlpha) {
                minAlpha = aChannel[i];
            }

            if (maxAlpha == -1) {
                maxAlpha = aChannel[i];
            }
            else if (aChannel[i] > maxAlpha) {
                maxAlpha = aChannel[i];
            }
        }
        return aChannel;
    }
    else if (channels == 2) { // GrayAlpha
        aChannel = new unsigned char[imageSize];

        for (int i = 0; i < imageSize; i++) {
            aChannel[i] = imageData[i * 2 + 1]; // Alpha is second byte
            
            if (minAlpha == -1) {
                minAlpha = aChannel[i];
            }
            else if (aChannel[i] < minAlpha) {
                minAlpha = aChannel[i];
            }

            if (maxAlpha == -1) {
                maxAlpha = aChannel[i];
            }
            else if (aChannel[i] > maxAlpha) {
                maxAlpha = aChannel[i];
            }
        }
        return aChannel;
    }
    else {
        return nullptr;
    }
}

unsigned char * Image::getGray() {
    return getGray(GRAYSCALE_LUMINOSITY);
}

unsigned char * Image::getGray(const GrayscaleMode mode) {
    auto grayChannel = new unsigned char[imageSize];
    
    if (!imageData) {
        return nullptr;
    }
    
    if (channels == 1) { // Already grayscale
        // Just copy the data
        memcpy(grayChannel, imageData, imageSize);
        return grayChannel;
    }
    
    if (mode == GRAYSCALE_LUMINOSITY) {
        if (channels == 2) { // GrayAlpha
            for (int i = 0; i < imageSize; i++) {
                grayChannel[i] = imageData[i * 2]; // Gray is first byte
            }
        }
        else if (channels == 3) { // RGB
            for (int i = 0; i < imageSize; i++) {
                // Using luminosity method (0.299R + 0.587G + 0.114B)
                const unsigned char r = imageData[i * 3];
                const unsigned char g = imageData[i * 3 + 1];
                const unsigned char b = imageData[i * 3 + 2];
                grayChannel[i] = static_cast<unsigned char>(0.299 * r + 0.587 * g + 0.114 * b);
            }
        }
        else if (channels == 4) { // RGBA
            for (int i = 0; i < imageSize; i++) {
                // Using luminosity method (0.299R + 0.587G + 0.114B)
                const unsigned char r = imageData[i * 4];
                const unsigned char g = imageData[i * 4 + 1];
                const unsigned char b = imageData[i * 4 + 2];
                grayChannel[i] = static_cast<unsigned char>(0.299 * r + 0.587 * g + 0.114 * b);
            }
        }
    }
    else if (mode == GRAYSCALE_AVERAGE) {
        if (channels == 4) { // RGBA
            for (int i = 0; i < imageSize; i++) {
                const unsigned char r = imageData[i * 4];
                const unsigned char g = imageData[i * 4 + 1];
                const unsigned char b = imageData[i * 4 + 2];
                grayChannel[i] = static_cast<unsigned char>((r + g + b) / 3);
            }
        }
        else if (channels == 3) { // RGB
            for (int i = 0; i < imageSize; i++) {
                const unsigned char r = imageData[i * 3];
                const unsigned char g = imageData[i * 3 + 1];
                const unsigned char b = imageData[i * 3 + 2];
                grayChannel[i] = static_cast<unsigned char>((r + g + b) / 3);
            }
        }
        else if (channels == 2) { // GrayAlpha
            for (int i = 0; i < imageSize; i++) {
                // Grayscale values are being left as they are
                grayChannel[i] = imageData[i * 2];
            }
        }
    }
    else {
        delete[] grayChannel;
        grayChannel = getGray(GRAYSCALE_AVERAGE);
    }
    return grayChannel;
}

unsigned char * Image::getRGB() {
    unsigned char * RGB = nullptr;

    if (!imageData) {
        return nullptr;
    }

    if (channels == 4) { // RGBA
        RGB = new unsigned char[imageSize * 3];
        for (int i = 0; i < imageSize; i++) {
            // Copy R, G, and B, discarding A
            RGB[3 * i] = imageData[4 * i];         // R
            RGB[3 * i + 1] = imageData[4 * i + 1]; // G
            RGB[3 * i + 2] = imageData[4 * i + 2]; // B
        }
    }
    else if (channels == 3) { // RGB
        // If already RGB, just copy the data
        RGB = new unsigned char[imageSize * 3];
        memcpy(RGB, imageData, imageSize * 3);
    }
    else if (channels == 2) { // GrayAlpha
        RGB = new unsigned char[imageSize * 3];
        for (int i = 0; i < imageSize; i++) {
            const unsigned char gray = imageData[2 * i];
            // Fill R, G, and B with the same gray value
            RGB[3 * i] = gray;
            RGB[3 * i + 1] = gray;
            RGB[3 * i + 2] = gray;
        }
    }
    else if (channels == 1) { // Gray
        RGB = new unsigned char[imageSize * 3];
        for (int i = 0; i < imageSize; i++) {
            const unsigned char gray = imageData[i];
            // Fill R, G, and B with the same gray value
            RGB[3 * i] = gray;
            RGB[3 * i + 1] = gray;
            RGB[3 * i + 2] = gray;
        }
    }
	return RGB;
}

unsigned char * Image::getRGBA() {
    unsigned char * RGBA = nullptr;

    if (!imageData) {
        return nullptr;
    }

    if (channels == 4) { // RGBA
        // Simply copy the data
        RGBA = new unsigned char[imageSize * 4];
        memcpy(RGBA, imageData, imageSize * 4);
    }
    else if (channels == 3) { // RGB -> RGBA
        RGBA = new unsigned char[imageSize * 4];
        for (int i = 0; i < imageSize; i++) {
            RGBA[4 * i] = imageData[3 * i];         // R
            RGBA[4 * i + 1] = imageData[3 * i + 1]; // G
            RGBA[4 * i + 2] = imageData[3 * i + 2]; // B
            RGBA[4 * i + 3] = 255;                  // A (fully opaque)
        }
    }
    else if (channels == 2) { // GrayAlpha -> RGBA
        RGBA = new unsigned char[imageSize * 4];
        for (int i = 0; i < imageSize; i++) {
            const unsigned char gray = imageData[2 * i];
            const unsigned char alpha = imageData[2 * i + 1];
            // Fill R, G, and B with the same gray value
            RGBA[4 * i] = gray;
            RGBA[4 * i + 1] = gray;
            RGBA[4 * i + 2] = gray;
            RGBA[4 * i + 3] = alpha;
        }
    }
    else if (channels == 1) { // Gray -> RGBA
        RGBA = new unsigned char[imageSize * 4];
        for (int i = 0; i < imageSize; i++) {
            const unsigned char gray = imageData[i];
            // Fill R, G, and B with the same gray value and A with 255 (fully opaque)
            RGBA[4 * i] = gray;
            RGBA[4 * i + 1] = gray;
            RGBA[4 * i + 2] = gray;
            RGBA[4 * i + 3] = 255; // A (fully opaque)
        }
    }
    else {
        return nullptr;
    }

    return RGBA;
}

bool Image::getTransparencyPresent() {
	return transparencyPresent;
}

int Image::getWidth() {
	return imageWidth;
}

int Image::getHeight() {
	return imageHeight;
}

int Image::getSize() {
	return imageSize;
}

int Image::getTotalSize() {
	return totalImageSize;
}

unsigned char* Image::getData() {
    return imageData;
}

int Image::getChannels() {
    return channels;
}

Image::~Image() {
    if (imageData != nullptr) {
        stbi_image_free(imageData);
        imageData = nullptr;
    }
}