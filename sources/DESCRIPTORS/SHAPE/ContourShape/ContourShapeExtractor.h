/** @file  ContourShapeExtractor.h
 *  @brief  Contour Shape class for extraction.
 *  @author Krzysztof Lech Kucharski
 *  @bug    No bugs detected */

#pragma once

#include "../../DescriptorExtractor.h"
#include "../ContourShape/ContourShape.h"

#define WHITE_ON_BLACK

class ContourShapeExtractor : public DescriptorExtractor {
    private:
        ContourShape * descriptor = nullptr;

    public:
	    ContourShapeExtractor();
	    Descriptor * extract(Image & image, const char ** params);

        unsigned long ExtractContour(int n, Image & image, Point2 * const & ishp);
        unsigned long ExtractPeaks(int n, const Point2 * const &ishp);
        void ExtractCurvature(int n, const Point2 * const &shp, unsigned long &qc, unsigned long &qe);

        static int compare_edges(const void * v1, const void * v2);
        static int compare_ind(const void *v1, const void *v2);

        unsigned char * getPixel(unsigned char * image, int x, int y, int imageWidth, int imageHeight);
        ~ContourShapeExtractor();
};
