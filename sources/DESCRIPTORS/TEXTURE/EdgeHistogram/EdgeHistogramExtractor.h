/** @file   EdgeHistogramExtractor.h
 *  @brief  Edge Histogram class for extraction.
 *  @author Krzysztof Lech Kucharski
 *  @bug    No bugs detected. */

#ifndef _EDGEHISTOGRAM_EXTRACTOR_H
#define _EDGEHISTOGRAM_EXTRACTOR_H

#include "../../DescriptorExtractor.h"
#include "../EdgeHistogram/EdgeHistogram.h"

typedef	struct Edge_Histogram_Descriptor {
    double Local_Edge[80];
} EHD;

class EdgeHistogramExtractor : public DescriptorExtractor {
    private:
        EdgeHistogram * descriptor = nullptr;

        EHD	 * m_pEdge_Histogram = new EHD[1];

        void EdgeHistogramGeneration(unsigned char * pImage_Y, unsigned long image_width, unsigned long image_height, unsigned long block_size, EHD * pLocal_Edge, int Te_Value);
        int GetEdgeFeature(unsigned char * pImage_Y, int image_width, int block_size, int Te_Value);
        unsigned long GetBlockSize(unsigned long image_width, unsigned long image_height, unsigned long desired_num_of_blocks);
        void SetEdgeHistogram(EHD * pEdge_Histogram);

    public:
	    EdgeHistogramExtractor();
	    Descriptor * extract(Image & image, const char ** params);
        ~EdgeHistogramExtractor();
};
#endif