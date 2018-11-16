/** @file   EdgeHistogramDistance.h
 *   @brief  Edge Histogram class for distance calculation.
 *   @author Krzysztof Lech Kucharski
 *   @bug    No bugs detected.                            */

#ifndef _EDGEHISTOGRAM_DISTANCE_H
#define _EDGEHISTOGRAM_DISTANCE_H

#include "../../DescriptorDistance.h"
#include "../EdgeHistogram/EdgeHistogram.h"

class EdgeHistogramDistance : public DescriptorDistance {
    private:
        EdgeHistogram * descriptor = NULL;
    public:
        EdgeHistogramDistance();

        double getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params);
        void Make_Global_SemiGlobal(double * LocalHistogramOnly, double * TotalHistogram);

        ~EdgeHistogramDistance();
};
#endif