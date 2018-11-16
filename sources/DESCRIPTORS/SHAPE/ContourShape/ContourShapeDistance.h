/** @file  ContourShapeDistance.h
 *  @brief  Contour Shape class for distance calculation.
 *  @author Krzysztof Lech Kucharski
 *  @bug    Some results are not equal to XM (floating point accuraccy?). */

#ifndef _CONTOUR_SHAPE_DISTANCE_H 
#define _CONTOUR_SHAPE_DISTANCE_H 

#include "../../DescriptorDistance.h"
#include "../ContourShape/ContourShape.h"

class Node {
    public:
        double cost;
        int    nr;
        int    nq;
        Point2 rPeaks[(CONTOURSHAPE_CSSPEAKMASK) + 1];
        Point2 qPeaks[(CONTOURSHAPE_CSSPEAKMASK) + 1];
};

class ContourShapeDistance : public DescriptorDistance {
    private:
        ContourShape * descriptor = NULL;

        float * m_rPeaksX = NULL;
        float * m_rPeaksY = NULL;
        float * m_qPeaksX = NULL;
        float * m_qPeaksY = NULL;
        Node * m_nodeList = NULL;

        float range(float x);
        double getDistance(ContourShape * contourShapeDescriptor1, ContourShape * contourShapeDescriptor2, const char ** params);
    public:
        ContourShapeDistance();
        double getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params);
        ~ContourShapeDistance();
};

#endif