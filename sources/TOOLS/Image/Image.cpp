#include "Image.h"
#include "../ErrorCode.h"

Image::Image() {
}

void Image::load(unsigned char * data, int size, LoadMode decode_mode) {
    std::vector<unsigned char> imageData;

    std::cout << "Loading image of size: " << size << std::endl;

    imageData.assign(data, data + size);

    std::cout << "Image data size: " << imageData.size() << std::endl;

    switch (decode_mode) {
        case IMAGE_UNCHANGED:
            // Load image as it is
            mat = cv::imdecode(imageData, CV_LOAD_IMAGE_UNCHANGED);
            break;
        case IMAGE_COLOR:
            // Load image as RGB discarding its original channel count
            mat = cv::imdecode(imageData, CV_LOAD_IMAGE_COLOR);
            break;
        case IMAGE_GRAYSCALE:
            // Load image as Grayscale (luminosity method) discarding its channel count
            mat = cv::imdecode(imageData, CV_LOAD_IMAGE_GRAYSCALE);
            break;
        default:
            // As default, load image as it is
            mat = cv::imdecode(imageData, CV_LOAD_IMAGE_UNCHANGED);
            break;
    }

    // Check, if image was opened properly
    if (mat.empty()) {
        std::cout << "Mat is empty" << std::endl;
        throw CANNOT_OPEN_IMAGE; 
    }

    // If data type is not 8-bit, convert to 8-bit
    if (mat.depth() != CV_8U) {
        std::string dpth;
        switch (mat.depth()) {
            case CV_8U:  dpth = "8U"; break;
            case CV_8S:  dpth = "8S"; break;
            case CV_16U: dpth = "16U"; break;
            case CV_16S: dpth = "16S"; break;
            case CV_32S: dpth = "32S"; break;
            case CV_32F: dpth = "32F"; break;
            case CV_64F: dpth = "64F"; break;
        }
        std::cout << "Warning: image depth: " << dpth << ", converting to 8U..." << std::endl;
        mat.convertTo(mat, CV_8U);
    }

    // Save image information
    imageWidth = mat.cols;
    imageHeight = mat.rows;
    imageSize = imageWidth * imageHeight;
    depth = mat.depth();

    // Check channel count support

    // Assuming, that 4 channel image is RGBA
    if (mat.channels() == 4) {
        transparencyPresent = true;
        totalImageSize = 4 * imageSize;
    }
    // Assuming, that 3 channel image is RGB
    else if (mat.channels() == 3) {
        transparencyPresent = false;
        totalImageSize = 3 * imageSize;
    }
    // Assuming, that 3 channel image is GrayAlpha
    else if (mat.channels() == 2) {
        transparencyPresent = true;
        totalImageSize = 2 * imageSize;
    }
    // Assuming, that 1 channel image is Grayscale
    else if (mat.channels() == 1) {
        transparencyPresent = false;
        totalImageSize = imageSize;
    }
    else {
        throw IMAGE_CHANNELS_NOT_SUPPORTED;
    }

    channels = mat.channels();
}

void Image::load(const char * filename, LoadMode load_mode) {
	switch (load_mode) {
		case IMAGE_UNCHANGED:
            // Load image as it is
			mat = cv::imread(filename, CV_LOAD_IMAGE_UNCHANGED);
			break;
		case IMAGE_COLOR:
            // Load image as RGB discarding its original channel count
			mat = cv::imread(filename, CV_LOAD_IMAGE_COLOR);
			break;
		case IMAGE_GRAYSCALE:
            // Load image as Grayscale (luminosity method) discarding its channel count
			mat = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
			break;
		default:
            // As default, load image as it is
			mat = cv::imread(filename, CV_LOAD_IMAGE_UNCHANGED);
			break;
	}

    // Check, if image was opened properly
    if (mat.empty()) {
        throw CANNOT_OPEN_IMAGE;
    }

    // If data type is not 8-bit, convert to 8-bit
    if (mat.depth() != CV_8U) {
        std::string dpth;
        switch (mat.depth()) {
            case CV_8U:  dpth = "8U"; break;
            case CV_8S:  dpth = "8S"; break;
            case CV_16U: dpth = "16U"; break;
            case CV_16S: dpth = "16S"; break;
            case CV_32S: dpth = "32S"; break;
            case CV_32F: dpth = "32F"; break;
            case CV_64F: dpth = "64F"; break;
        }
        std::cout << "Warning: image depth: " << dpth << ", converting to 8U..." << std::endl;
        mat.convertTo(mat, CV_8U);
    }

    // Save image information
    imageWidth = mat.cols;
    imageHeight = mat.rows;
    imageSize = imageWidth * imageHeight;
    depth = mat.depth();

    // Check channel count support

    // Assuming, that 4 channel image is RGBA
    if (mat.channels() == 4) {
        transparencyPresent = true;
        totalImageSize = 4 * imageSize;
    }
    // Assuming, that 3 channel image is RGB
    else if (mat.channels() == 3) {
        transparencyPresent = false;
        totalImageSize = 3 * imageSize;
    }
    // Assuming, that 3 channel image is GrayAlpha
    else if (mat.channels() == 2) {
        transparencyPresent = true;
        totalImageSize = 2 * imageSize;
    }
    // Assuming, that 1 channel image is Grayscale
    else if (mat.channels() == 1) {
        transparencyPresent = false;
        totalImageSize = imageSize;
    }
    else {
        throw IMAGE_CHANNELS_NOT_SUPPORTED;
    }

    channels = mat.channels();
}

unsigned char * Image::getChannel_R() {
    // Image size is pixel count in an image (width * height)
	unsigned char * rChannel = new unsigned char[imageSize];

    // RGBA
    if (mat.channels() == 4) { 
        int i = 0;
        for (int y = 0; y < imageHeight; y++) {
            for (int x = 0; x < imageWidth; x++) {
                cv::Vec4b pixel = mat.at<cv::Vec4b>(y, x);
                rChannel[i] = pixel.val[2]; // (A - 3, R - 2, G - 1, B - 0)
                i++;
            }
        }
    }
    // RGB
    else if (mat.channels() == 3) { 
        int i = 0;
        for (int y = 0; y < imageHeight; y++) {
            for (int x = 0; x < imageWidth; x++) {
                cv::Vec3b pixel = mat.at<cv::Vec3b>(y, x);
                rChannel[i] = pixel.val[2]; // (R - 2, G - 1, B - 0)
                i++;
            }
        }
    }
    // GrayAlpha
    else if (mat.channels() == 2) { 
        int i = 0;
        for (int y = 0; y < imageHeight; y++) {
            for (int x = 0; x < imageWidth; x++) {
                cv::Vec2b pixel = mat.at<cv::Vec2b>(y, x);
                rChannel[i] = pixel.val[0]; // (Alpha - 1, Gray - 0)
                i++;
            }
        }
    }
    // Gray
    else if (mat.channels() == 1) { 
        int i = 0;
        for (int y = 0; y < imageHeight; y++) {
            for (int x = 0; x < imageWidth; x++) {
                rChannel[i] = mat.at<uchar>(y, x); // (R | G | B get same grayscale value)
                i++;
            }
        }
    }
    // Other cases do not exist - it has been checked while image load process
	return rChannel;
}

unsigned char * Image::getChannel_G() {
	unsigned char * gChannel = new unsigned char[imageSize];

    if (mat.channels() == 4) { // RGBA
        int i = 0;
        for (int y = 0; y < imageHeight; y++) {
            for (int x = 0; x < imageWidth; x++) {
                cv::Vec4b pixel = mat.at<cv::Vec4b>(y, x);
                gChannel[i] = pixel.val[1]; // (A - 3, R - 2, G - 1, B - 0)
                i++;
            }
        }
    }
    else if (mat.channels() == 3) { // RGB
        int i = 0;
        for (int y = 0; y < imageHeight; y++) {
            for (int x = 0; x < imageWidth; x++) {
                cv::Vec3b pixel = mat.at<cv::Vec3b>(y, x);
                gChannel[i] = pixel.val[1]; // (R - 2, G - 1, B - 0)
                i++;
            }
        }
    }
    else if (mat.channels() == 2) { // GrayAlpha
        int i = 0;
        for (int y = 0; y < imageHeight; y++) {
            for (int x = 0; x < imageWidth; x++) {
                cv::Vec2b pixel = mat.at<cv::Vec2b>(y, x);
                gChannel[i] = pixel.val[1]; // (Alpha - 1, Gray - 0)
                i++;
            }
        }
    }
    else if (mat.channels() == 1) { // Gray
        int i = 0;
        for (int y = 0; y < imageHeight; y++) {
            for (int x = 0; x < imageWidth; x++) {
                gChannel[i] = mat.at<uchar>(y, x); // (R | G | B get same grayscale value)
                i++;
            }
        }
    }
    // Other cases do not exist - it has been checked while image load process
	return gChannel;
}

unsigned char * Image::getChannel_B() {
	unsigned char * bChannel = new unsigned char[imageSize];

    if (mat.channels() == 4) { // RGBA
        int i = 0;
        for (int y = 0; y < imageHeight; y++) {
            for (int x = 0; x < imageWidth; x++) {
                cv::Vec4b pixel = mat.at<cv::Vec4b>(y, x);
                bChannel[i] = pixel.val[0]; // (A - 3, R - 2, G - 1, B - 0)
                i++;
            }
        }
    }
    else if (mat.channels() == 3) { // RGB
        int i = 0;
        for (int y = 0; y < imageHeight; y++) {
            for (int x = 0; x < imageWidth; x++) {
                cv::Vec3b pixel = mat.at<cv::Vec3b>(y, x);
                bChannel[i] = pixel.val[0]; // (R - 2, G - 1, B - 0)
                i++;
            }
        }
    }
    else if (mat.channels() == 2) { // GrayAlpha
        int i = 0;
        for (int y = 0; y < imageHeight; y++) {
            for (int x = 0; x < imageWidth; x++) {
                cv::Vec2b pixel = mat.at<cv::Vec2b>(y, x);
                bChannel[i] = pixel.val[0]; // (Alpha - 1, Gray - 0)
                i++;
            }
        }
    }
    else if (mat.channels() == 1) { // Gray
        int i = 0;
        for (int y = 0; y < imageHeight; y++) {
            for (int x = 0; x < imageWidth; x++) {
                bChannel[i] = mat.at<uchar>(y, x); // (R | G | B get same grayscale value)
                i++;
            }
        }
    }
    // Other cases do not exist - it has been checked while image load process
	return bChannel;
}

unsigned char * Image::getChannel_A() {
	unsigned char * aChannel = NULL;

	if (mat.channels() == 4) { // RGBA
		aChannel = new unsigned char[imageSize];

		int i = 0;
		for (int y = 0; y < imageHeight; y++) {
			for (int x = 0; x < imageWidth; x++) {
				cv::Vec4b pixel = mat.at<cv::Vec4b>(y, x);
				aChannel[i] = pixel.val[3]; // (A - 3, R - 2, G - 1, B - 0)

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

				i++;
			}
		}
		return aChannel;
	}
    else if (mat.channels() == 2) { // GrayAlpha
        aChannel = new unsigned char[imageSize];

        int i = 0;
        for (int y = 0; y < imageHeight; y++) {
            for (int x = 0; x < imageWidth; x++) {
                cv::Vec2b pixel = mat.at<cv::Vec2b>(y, x);
                aChannel[i] = pixel.val[1]; // (Alpha - 1, Gray - 0)

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

                i++;
            }
        }
        return aChannel;
    }
	else {
		return NULL;
	}
}

unsigned char * Image::getGray() {
    return getGray(GRAYSCALE_LUMINOSITY);
}

unsigned char * Image::getGray(GrayscaleMode mode) {
	unsigned char * grayChannel = new unsigned char[imageSize];
    
    if (mode == GRAYSCALE_LUMINOSITY) {
        if (mat.channels() == 1) {
            int i = 0;
            for (int y = 0; y < imageWidth; y++) {
                for (int x = 0; x < imageHeight; x++) {
                    grayChannel[i] = mat.at<uchar>(x, y);
                    i++;
                }
            }
        }
        if (mat.channels() == 2) {
            int i = 0;
            for (int y = 0; y < imageWidth; y++) {
                for (int x = 0; x < imageHeight; x++) {
                    cv::Vec2b pix = mat.at<cv::Vec2b>(y, x);
                    grayChannel[i] = pix.val[0];
                    i++;
                }
            }
        }
        else if (mat.channels() == 3) {
            int i = 0;
            for (int y = 0; y < imageHeight; y++) {
                for (int x = 0; x < imageWidth; x++) {
                    cv::Vec3b pix = mat.at<cv::Vec3b>(y, x);
                    grayChannel[i] = static_cast<unsigned char>(0.299 * pix.val[2] + 0.587 * pix.val[1] + 0.114 * pix.val[0]);
                    i++;
                }
            }
        }
        else if (mat.channels() == 4) {
            int i = 0;
            for (int y = 0; y < imageHeight; y++) {
                for (int x = 0; x < imageWidth; x++) {
                    cv::Vec4b pix = mat.at<cv::Vec4b>(y, x);
                    grayChannel[i] = static_cast<unsigned char>(0.299 * pix.val[2] + 0.587 * pix.val[1] + 0.114 * pix.val[0]);
                    i++;
                }
            }
        }
    }
    else if (mode == GRAYSCALE_AVERAGE) {
        if (mat.channels() == 4) { // RGBA
            int i = 0;
            for (int y = 0; y < imageHeight; y++) {
                for (int x = 0; x < imageWidth; x++) {
                    cv::Vec4b pix = mat.at<cv::Vec4b>(y, x);
                    grayChannel[i] = (unsigned char) ((pix.val[2] + pix.val[1] + pix.val[0]) / 3);
                    i++;
                }
            }
        }
        else if (mat.channels() == 3) { // RGB
            int i = 0;
            for (int y = 0; y < imageHeight; y++) {
                for (int x = 0; x < imageWidth; x++) {
                    cv::Vec3b pix = mat.at<cv::Vec3b>(y, x);
                    grayChannel[i] = (unsigned char) ((pix.val[2] + pix.val[1] + pix.val[0]) / 3);
                    i++;
                }
            }
        }
        else if (mat.channels() == 2) { // GrayAlpha
            int i = 0;
            for (int y = 0; y < imageHeight; y++) {
                for (int x = 0; x < imageWidth; x++) {
                    cv::Vec2b pix = mat.at<cv::Vec2b>(y, x);
                    // grayscale values are being left as they are
                    grayChannel[i] = pix.val[0];
                    i++;
                }
            }
        }
        else if (mat.channels() == 1) { // Gray
            int i = 0;
            for (int y = 0; y < imageHeight; y++) {
                for (int x = 0; x < imageWidth; x++) {
                    // grayscale values are being left as they are
                    grayChannel[i] = mat.at<uchar>(y, x); 
                    i++;
                }
            }
        }
    }
    else {
        grayChannel = getGray(GRAYSCALE_AVERAGE);
    }
	return grayChannel;
}

unsigned char * Image::getRGB() {
    unsigned char * RGB = NULL;

	int i = 0;

	if (transparencyPresent) {
        RGB = new unsigned char[imageSize * 3];
		for (int y = 0; y < imageHeight; y++) {
			for (int x = 0; x < imageWidth; x++) {
				cv::Vec4b pixel = mat.at<cv::Vec4b>(y, x);

				RGB[3 * i] = pixel.val[2];
				RGB[3 * i + 1] = pixel.val[1];
				RGB[3 * i + 2] = pixel.val[0];

				i++;
			}
		}
	}
	else if (mat.channels() == 3) {
        RGB = new unsigned char[imageSize * 3];
		for (int y = 0; y < imageHeight; y++) {
			for (int x = 0; x < imageWidth; x++) {
				cv::Vec3b pixel = mat.at<cv::Vec3b>(y, x);

				RGB[3 * i] = pixel.val[2];
				RGB[3 * i + 1] = pixel.val[1];
				RGB[3 * i + 2] = pixel.val[0];

				i++;
			}
		}
	}
	return RGB;
}

unsigned char * Image::getRGBA() {
    unsigned char * RGBA = NULL;

	int i = 0;

	if (transparencyPresent && channels == 4) {
        RGBA = new unsigned char[imageSize * 4];
		for (int y = 0; y < imageHeight; y++) {
			for (int x = 0; x < imageWidth; x++) {
				cv::Vec4b pixel = mat.at<cv::Vec4b>(y, x);

				RGBA[3 * i] = pixel.val[3];
				RGBA[3 * i + 1] = pixel.val[2];
				RGBA[3 * i + 2] = pixel.val[1];
				RGBA[3 * i + 3] = pixel.val[0];
				i++;
			}
		}
	}
    else {
        return NULL;
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

cv::Mat & Image::getMat() {
    return mat;
}

Image::~Image() {
}