#include "ColorLayoutExtractor.h"

ColorLayoutExtractor::ColorLayoutExtractor() {
	descriptor = new ColorLayout();
}

Descriptor * ColorLayoutExtractor::extract(Image & image, const char ** params) {  
    /* Load parameters */
    try {
        descriptor->loadParameters(params);
    }
    catch (ErrorCode exception) {
        throw exception;
    }

    /* Create small image */
	short small_img[3][64];
	CreateSmallImage(image, small_img);

    /* Apply DCT */
	FastDiscreteCosineTransform(small_img[0]);
    FastDiscreteCosineTransform(small_img[1]);
    FastDiscreteCosineTransform(small_img[2]);

    /* Quantize and zig-zag */
	int YCoeff[64], CbCoeff[64], CrCoeff[64];

    // DC
	YCoeff[0]  = YDCQuantization(small_img[0][0] / 8) >> 1;
	CbCoeff[0] = CDCQuantization(small_img[1][0] / 8);
	CrCoeff[0] = CDCQuantization(small_img[2][0] / 8);

    // AC
	for (int i = 1; i < 64; i++) {
		YCoeff[i]  = ACQuantization((small_img[0][(ZigZagScanCoefficients[i])]) / 2) >> 3;
		CbCoeff[i] = ACQuantization(small_img[1][(ZigZagScanCoefficients[i])]) >> 3;
		CrCoeff[i] = ACQuantization(small_img[2][(ZigZagScanCoefficients[i])]) >> 3;
	}

    /* Set results */
	descriptor->allocateYCoefficients();
	descriptor->allocateCbCoefficients();
	descriptor->allocateCrCoefficients();

    for (int i = 0; i < descriptor->getNumberOfYCoeffcients(); i++) {
        descriptor->setYCoefficient(i, YCoeff[i]); // Y coefficient 
	}
	for (int i = 0; i < descriptor->getNumberOfCCoefficients(); i++) {
        descriptor->setCbCoefficient(i, CbCoeff[i]); // Cb coefficient
        descriptor->setCrCoefficient(i, CrCoeff[i]); // Cr coefficient
	}

    return descriptor;
}

void ColorLayoutExtractor::CreateSmallImage(Image & image, short small_img[3][64]) {
    /* Original image is being partitioned into 64 blocks */

    // Get image information:
    const bool transparencyPresent = image.getTransparencyPresent();
    const int imageWidth           = image.getWidth();
    const int imageHeight          = image.getHeight();
    const int imageSize            = image.getSize();

    int i, j, k;
    int x, y;
    long small_block_sum[3][64];
    int cnt[64];

    // Prepare small image
    for (i = 0; i < (8 * 8); i++) {
        cnt[i] = 0;
        for (j = 0; j < 3; j++) {
            small_block_sum[j][i] = 0;
            small_img[j][i] = 0;
        }
    }

    // Upsampling for small pictures (less than 8 x 8 pixels) to avoid floating point exception (XM)    
    const int rep_width  = (imageWidth  < 8) ? 7 : 0;
    const int rep_height = (imageHeight < 8) ? 7 : 0;

    const int width  = (imageWidth   < 8) ? 8 * imageWidth : imageWidth;
    const int height = (imageHeight  < 8) ? 8 * imageHeight : imageHeight;

    // Get image data
    const unsigned char * pR = image.getChannel_R();
    const unsigned char * pG = image.getChannel_G();
    const unsigned char * pB = image.getChannel_B();
    // if image has alpha channel, get it:
    const unsigned char * pA = transparencyPresent ? new unsigned char[imageSize] : nullptr;

    /* Modify the code to use new operation to alloc memory for 3D array (XM)

    Creates new buffer for image data (for 3 or 4 channels, if alpha is present) 
    In case of memory leaks, reduntant pointers to each channel in this buffer
    were removed from original code. All operations are made on this buffer now. (KK) */
    unsigned char * buffer = transparencyPresent ? new unsigned char[4 * width * height] : new unsigned char[3 * width * height];

    int idx = 0;
    for (y = 0; y < imageHeight; y++) {
        for (x = 0; x < imageWidth; x++) {
            buffer[idx + 0] = pR[idx];
            buffer[idx + width * height] = pG[idx];
            buffer[idx + width * height * 2] = pB[idx];

            if (transparencyPresent) {
                buffer[idx + width * height * 3] = pA[idx];
            }

            for (i = 0; i < rep_width; i++) {
                buffer[idx + 0] = pR[idx];
                buffer[idx + width * height] = pG[idx];
                buffer[idx + width * height * 2] = pB[idx];

                if (transparencyPresent) {
                    buffer[idx + width * height * 3] = pA[idx];
                }
            }
            idx++;
        }

        for (j = 0; j < rep_height; j++) {
            memcpy(buffer + 0 * (width * height) + (8 * y + j + 1) * width + 0,
                   buffer + 0 * (width * height) + (8 * y)         * width + 0, width);

            memcpy(buffer + 1 * (width * height) + (8 * y + j + 1) * width + 0,
                   buffer + 1 * (width * height) + (8 * y)         * width + 0, width);

            memcpy(buffer + 2 * (width * height) + (8 * y + j + 1) * width + 0,
                   buffer + 2 * (width * height) + (8 * y)         * width + 0, width);

            if (transparencyPresent) {
                memcpy(buffer + 3 * (width * height) + (8 * y + j + 1) * width + 0,
                       buffer + 3 * (width * height) + (8 * y)         * width + 0, width);
            }
        }
    }

    double yy;
    short R, G, B, A;
    int y_axis, x_axis;

    idx = 0;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            y_axis = static_cast<int>(y / (height / 8.0));
            x_axis = static_cast<int>(x / (width / 8.0));

            k = y_axis * 8 + x_axis;

            G = pG[idx];
            B = pB[idx];
            R = pR[idx];

            if (transparencyPresent) {
                A = pA[idx];
                if (A == 0) {
                    continue;
                }
            }

            idx++;

            // RGB to YCbCr conversion
            yy = (0.299 * R + 0.587 * G + 0.114 * B) / 256.0;

            small_block_sum[0][k] += static_cast<int>(219.0 * yy + 16.5);                              // Y
            small_block_sum[1][k] += static_cast<int>(224.0 * 0.564 * (B / 256.0 * 1.0 - yy) + 128.5); // Cb
            small_block_sum[2][k] += static_cast<int>(224.0 * 0.713 * (R / 256.0 * 1.0 - yy) + 128.5); // Cr

            cnt[k]++;
        }
    }

    double total_sum[3] = { 0.0, 0.0, 0.0 };
    int valid_cell = 0;

    // Create 8 x 8 pixels image
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            for (k = 0; k < 3; k++) {

                if (cnt[i * 8 + j]) {
                    small_img[k][i * 8 + j] = static_cast<short>((small_block_sum[k][i * 8 + j] / cnt[i * 8 + j]));
                }
                else {
                    small_img[k][i * 8 + j] = 0;
                }

                total_sum[k] += small_img[k][i * 8 + j];
            }

            if (cnt[i * 8 + j] != 0) {
                valid_cell++;
            }
        }
    }

    if (transparencyPresent) {
        for (k = 0; k < 3; k++) {
            if (valid_cell) {
                total_sum[k] = total_sum[k] / valid_cell;
            }
            else {
                total_sum[k] = 0;
            }
        }

        for (i = 0; i < 8; i++) {
            for (j = 0; j < 8; j++) {
                for (k = 0; k < 3; k++) {
                    if (small_img[k][i * 8 + j] == 0) {
                        small_img[k][i * 8 + j] = static_cast<short>(total_sum[k]);
                    }
                }
            }
        }
    }

    delete[] pR;
    delete[] pG;
    delete[] pB;

    if (pA) {
        delete[] pA;
    }

    delete[] buffer;
}

void ColorLayoutExtractor::FastDiscreteCosineTransform(short * block) {
	int i, j, k;
	double s;
	double tmp[64];

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			s = 0.0;
            for (k = 0; k < 8; k++) {
                s += DCT_Coefficients[j][k] * block[8 * i + k];
            }
			tmp[8 * i + j] = s;
		}
	}

	for (j = 0; j < 8; j++) {
		for (i = 0; i < 8; i++) {
			s = 0.0;
			for (k = 0; k < 8; k++) s += DCT_Coefficients[i][k] * tmp[8 * k + j];
			block[8 * i + j] = static_cast<int>(floor(s + 0.499999));
		}
	}
}

int ColorLayoutExtractor::YDCQuantization(const int i) {
	int j;

	if (i > 191) {
		j = 112 + (i - 192) / 4;
	}
	else if (i > 159) {
		j = 96 + (i - 160) / 2;
	}
	else if (i > 95) {
		j = 32 + (i - 96);
	}
	else if (i > 63) {
		j = 16 + (i - 64) / 2;
	}
	else {
		j = i / 4;
	}

	return j;
}

int ColorLayoutExtractor::CDCQuantization(const int i) {
	int j;

	if (i > 191) {
		j = 63;
	}
	else if (i > 159) {
		j = 56 + (i - 160) / 4;
	}
	else if (i > 143) {
		j = 48 + (i - 144) / 2;
	}
	else if (i > 111) {
		j = 16 + (i - 112);
	}
	else if (i > 95) {
		j = 8 + (i - 96) / 2;
	}
	else if (i > 63) {
		j = (i - 64) / 4;
	}
	else {
		j = 0;
	}

	return j;
}

int ColorLayoutExtractor::ACQuantization(int i) {
	int j;
	/* 
    change for WD 2.0
	if(i>255) i= 255; (XM) */

	if (i > 239) {
		i = 239;
	}
	if (i < -256) {
		i = -256;
	}
	if ((abs(i)) > 127) {
		j = 64 + (abs(i)) / 4;
	}
	else if ((abs(i)) > 63) {
		j = 32 + (abs(i)) / 2;
	}
	else {
		j = abs(i);
	}

	j = (i < 0) ? -j : j;
	/* 
    change for WD 2.0
	j +=128; (XM) */ 
	j += 132;

	return j;
}

ColorLayoutExtractor::~ColorLayoutExtractor() {
    delete descriptor;
}

// DCT coefficients
const double ColorLayoutExtractor::DCT_Coefficients[8][8] = {
 { 3.535534e-01,  3.535534e-01,  3.535534e-01,  3.535534e-01,
   3.535534e-01,  3.535534e-01,  3.535534e-01,  3.535534e-01 },
 { 4.903926e-01,  4.157348e-01,  2.777851e-01,  9.754516e-02,
  -9.754516e-02, -2.777851e-01, -4.157348e-01, -4.903926e-01 },
 { 4.619398e-01,  1.913417e-01, -1.913417e-01, -4.619398e-01,
  -4.619398e-01, -1.913417e-01,  1.913417e-01,  4.619398e-01 },
 { 4.157348e-01, -9.754516e-02, -4.903926e-01, -2.777851e-01,
   2.777851e-01,  4.903926e-01,  9.754516e-02, -4.157348e-01 },
 { 3.535534e-01, -3.535534e-01, -3.535534e-01,  3.535534e-01,
   3.535534e-01, -3.535534e-01, -3.535534e-01,  3.535534e-01 },
 { 2.777851e-01, -4.903926e-01,  9.754516e-02,  4.157348e-01,
  -4.157348e-01, -9.754516e-02,  4.903926e-01, -2.777851e-01 },
 { 1.913417e-01, -4.619398e-01,  4.619398e-01, -1.913417e-01,
  -1.913417e-01,  4.619398e-01, -4.619398e-01,  1.913417e-01 },
 { 9.754516e-02, -2.777851e-01,  4.157348e-01, -4.903926e-01,
   4.903926e-01, -4.157348e-01,  2.777851e-01, -9.754516e-02 } };

// Zig-Zag scan pattern
const unsigned char ColorLayoutExtractor::ZigZagScanCoefficients[64] = {
   0,  1,  8,  16, 9,  2,  3,  10, 17, 24, 32, 25, 18, 11, 4,  5,
   12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6,  7,  14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63 };