#include "EdgeHistogramExtractor.h"

EdgeHistogramExtractor::EdgeHistogramExtractor() {
    descriptor = new EdgeHistogram();
}

Descriptor * EdgeHistogramExtractor::extract(Image & image, const char ** params) {
    descriptor->loadParameters(params);

    // Get image information
    const int imageWidth     = image.getWidth();
    const int imageHeight    = image.getHeight();
    const bool isTransparent = image.getTransparencyPresent();

    // Get image data
    const unsigned char * R = image.getChannel_R();
    const unsigned char * G = image.getChannel_G();
    const unsigned char * B = image.getChannel_B();

    const unsigned char * A = nullptr;

    if (isTransparent) {
        A = image.getChannel_A();
    }


    unsigned long desired_num_of_blocks;
    unsigned long block_size;

    int	Te_Value;
    EHD	* pLocal_Edge;

    pLocal_Edge = nullptr;
    pLocal_Edge = new EHD[1];

    Te_Value = Te_Define;
    desired_num_of_blocks = Desired_Num_of_Blocks;

    // Create gray image
    int i, j, xsize, ysize;
    unsigned char * pGrayImage;

    // Arbitrary shape (Modified by Dongguk)
    unsigned char * pResampleImage = nullptr;

    int max_x = 0;
    int max_y = 0;

    int min_x = imageWidth - 1;
    int min_y = imageHeight - 1;

    double scale, EWweight, NSweight, EWtop, EWbottom;
    unsigned char NW, NE, SW, SE;
    int min_size, re_xsize, re_ysize;

    if (isTransparent) {
        for (j = 0; j < imageHeight; j++) {
            for (i = 0; i < imageWidth; i++) {
                if (A[j * imageWidth + i]) {

                    if (max_x < i) {
                        max_x = i;
                    }

                    if (max_y < j) {
                        max_y = j;
                    }
                    if (min_x > i) {
                        min_x = i;
                    }
                    if (min_y > j) {
                        min_y = j;
                    }
                }
            }
        }
        xsize = max_x - min_x + 1;
        ysize = max_y - min_y + 1;
    }
    else {
        xsize = imageWidth;
        ysize = imageHeight;
        min_x = min_y = 0;
    }

    pGrayImage = new unsigned char[xsize * ysize];

    for (j = 0; j < ysize; j++) {
        for (i = 0; i < xsize; i++) {
            if (isTransparent) {
                if (A[(j + min_y) * imageWidth + (i + min_x)]) {

                    pGrayImage[j * xsize + i] = static_cast<unsigned char>((float)
                                                                           (R[(j + min_y) * imageWidth + (i + min_x)] +
                                                                            G[(j + min_y) * imageWidth + (i + min_x)] +
                                                                            B[(j + min_y) * imageWidth + (i + min_x)]) / 3.0f);
                }
                else {
                    pGrayImage[j * xsize + i] = 0;
                }
            }
            else {
                pGrayImage[j * xsize + i] = static_cast<unsigned char>((float)
                                                                       (R[j * xsize + i] + G[j * xsize + i] + B[j * xsize + i]) / 3.0f);
            }
        }
    }

    delete[] R;
    delete[] G;
    delete[] B;

    if (A) {
        delete[] A;
    }



    min_size = (xsize>ysize) ? ysize : xsize;

    if (min_size < 70) {
        // Upsampling
        scale = 70.0 / min_size;

        re_xsize = static_cast<int>(xsize * scale + 0.5);
        re_ysize = static_cast<int>(ysize * scale + 0.5);

        pResampleImage = new unsigned char[re_xsize * re_ysize];

        for (j = 0; j < re_ysize; j++) {
            for (i = 0; i < re_xsize; i++) {
                EWweight = i / scale - floor(i / scale);
                NSweight = j / scale - floor(j / scale);

                NW = pGrayImage[static_cast<int>(floor(i / scale))     + static_cast<int>(floor(j / scale)) * xsize];
                NE = pGrayImage[static_cast<int>(floor(i / scale)) + 1 + static_cast<int>(floor(j / scale)) * xsize];

                SW = pGrayImage[static_cast<int>(floor(i / scale))     + static_cast<int>(floor(j / scale) + 1) * xsize];
                SE = pGrayImage[static_cast<int>(floor(i / scale)) + 1 + static_cast<int>(floor(j / scale) + 1) * xsize];

                EWtop    = NW + EWweight * (NE - NW);
                EWbottom = SW + EWweight * (SE - SW);

                pResampleImage[i + j * re_xsize] = static_cast<unsigned char>(EWtop + NSweight * (EWbottom - EWtop) + 0.5);
            }
        }
        block_size = GetBlockSize(re_xsize, re_ysize, desired_num_of_blocks);

        if (block_size < 2) {
            block_size = 2;
        }

        EdgeHistogramGeneration(pResampleImage, re_xsize, re_ysize, block_size, pLocal_Edge, Te_Value);

        delete[] pResampleImage;
    }
    else {
        block_size = GetBlockSize(xsize, ysize, desired_num_of_blocks);

        if (block_size < 2) {
            block_size = 2;
        }

        EdgeHistogramGeneration(pGrayImage, xsize, ysize, block_size, pLocal_Edge, Te_Value);
    }

    // Set descriptor data
    SetEdgeHistogram(pLocal_Edge);

    // Free memory
    delete[] pLocal_Edge;
    delete[] pGrayImage;

    return descriptor;
}

void EdgeHistogramExtractor::EdgeHistogramGeneration(unsigned char * pImage_Y, const unsigned long image_width, const unsigned long image_height, const unsigned long block_size, EHD * pLocal_Edge, const int Te_Value) {
    int  Count_Local[16];
    long LongTyp_Local_Edge[80];

    int sub_local_index;
    int Offset, EdgeTypeOfBlock;
    unsigned int i, j;

    // Clear
    memset(Count_Local, 0, 16 * sizeof(int));
    memset(LongTyp_Local_Edge, 0, 80 * sizeof(long));

    for (j = 0; j <= image_height - block_size; j += block_size) {
        for (i = 0; i <= image_width - block_size; i += block_size) {

            sub_local_index = static_cast<int>(i * 4 / image_width) + static_cast<int>(j * 4 / image_height) * 4;

            Count_Local[sub_local_index]++;

            Offset = image_width * j + i;

            EdgeTypeOfBlock = GetEdgeFeature(pImage_Y + Offset, image_width, block_size, Te_Value);
            switch (EdgeTypeOfBlock) {
                case NoEdge:
                    break;
                case vertical_edge:
                    LongTyp_Local_Edge[sub_local_index * 5]++;
                    break;
                case horizontal_edge:
                    LongTyp_Local_Edge[sub_local_index * 5 + 1]++;
                    break;
                case diagonal_45_degree_edge:
                    LongTyp_Local_Edge[sub_local_index * 5 + 2]++;
                    break;
                case diagonal_135_degree_edge:
                    LongTyp_Local_Edge[sub_local_index * 5 + 3]++;
                    break;
                case non_directional_edge:
                    LongTyp_Local_Edge[sub_local_index * 5 + 4]++;
                    break;
            }
        } 
    }

    for (i = 0; i < 80; i++) { // Range 0.0 ~ 1.0
        sub_local_index = static_cast<int>(i / 5);
        pLocal_Edge->Local_Edge[i] = static_cast<double>(LongTyp_Local_Edge[i]) / Count_Local[sub_local_index];
    }
}

int EdgeHistogramExtractor::GetEdgeFeature(unsigned char * pImage_Y, const int image_width, const int block_size, const int Te_Value) {
    int		i, j;
    double	d1, d2, d3, d4;
    int		e_index;
    const double  dc_th = Te_Value;
    double  e_h, e_v, e_45, e_135, e_m, e_max;

    d1 = 0.0;
    d2 = 0.0;
    d3 = 0.0;
    d4 = 0.0;

    for (j = 0; j < block_size; j++) {
        for (i = 0; i < block_size; i++) {
            if (j < block_size / 2) {
                if (i < block_size / 2)
                    d1 += (pImage_Y[i + image_width * j]);
                else
                    d2 += (pImage_Y[i + image_width * j]);
            }
            else {
                if (i < block_size / 2)
                    d3 += (pImage_Y[i + image_width * j]);
                else
                    d4 += (pImage_Y[i + image_width * j]);
            }
        }
    }
    d1 = d1 / (block_size * block_size / 4.0);
    d2 = d2 / (block_size * block_size / 4.0);
    d3 = d3 / (block_size * block_size / 4.0);
    d4 = d4 / (block_size * block_size / 4.0);

    e_h = fabs(d1 + d2 - (d3 + d4));
    e_v = fabs(d1 + d3 - (d2 + d4));

    e_45  = sqrt(2.) * fabs(d1 - d4);
    e_135 = sqrt(2.) * fabs(d2 - d3);

    e_m = 2 * fabs(d1 - d2 - d3 + d4);

    e_max = e_v;
    e_index = vertical_edge;

    if (e_h > e_max) {
        e_max = e_h;
        e_index = horizontal_edge;
    }
    if (e_45 > e_max) {
        e_max = e_45;
        e_index = diagonal_45_degree_edge;
    }
    if (e_135 > e_max) {
        e_max = e_135;
        e_index = diagonal_135_degree_edge;
    }
    if (e_m > e_max) {
        e_max = e_m;
        e_index = non_directional_edge;
    }
    if (e_max < dc_th)
        e_index = NoEdge;

    return(e_index);
}

unsigned long EdgeHistogramExtractor::GetBlockSize(const unsigned long image_width, const unsigned long image_height, const unsigned long desired_num_of_blocks) {
    const double temp_size = sqrt(static_cast<double>(image_width * image_height / desired_num_of_blocks));
    const unsigned long block_size = static_cast<unsigned long>(temp_size / 2) * 2;

    return block_size;
}

void EdgeHistogramExtractor::SetEdgeHistogram(EHD * pEdge_Histogram) {
    int i, j;
    double iQuantValue;

    for (i = 0; i < 80; i++) {
        j = 0;
        while (1) {
            if (j < 7) // SIZI-1 
                iQuantValue = (descriptor->QuantTable[i % 5][j] + descriptor->QuantTable[i % 5][j + 1]) / 2.0;
            else
                iQuantValue = 1.0;
            if (pEdge_Histogram->Local_Edge[i] <= iQuantValue)
                break;
            j++;
        }
        descriptor->setEdgeHistogramElement(i, j);
    }
    for (i = 0; i < 80; i++) {
        m_pEdge_Histogram->Local_Edge[i] = descriptor->QuantTable[i % 5][descriptor->getEdgeHistogramElement(i)];
    }
}

EdgeHistogramExtractor::~EdgeHistogramExtractor() {
    delete[] m_pEdge_Histogram;
    delete descriptor;
}