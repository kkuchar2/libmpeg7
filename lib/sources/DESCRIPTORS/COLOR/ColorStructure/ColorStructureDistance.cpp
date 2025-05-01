#include "ColorStructureDistance.h"

ColorStructureDistance::ColorStructureDistance() {
}

double ColorStructureDistance::getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params) {
    ColorStructure * colorStructureDescriptor1 = static_cast<ColorStructure *>(descriptor1);
    ColorStructure * colorStructureDescriptor2 = static_cast<ColorStructure *>(descriptor2);

    // Check size equality
    if (colorStructureDescriptor1->GetSize() != colorStructureDescriptor2->GetSize()) {
        throw COL_STRUCT_DISTANCE_SIZE_ERROR;
    }

    // Calculate the distance
    double distance = 0.0;
    double L1error = 0.0;

    unsigned int i, size = colorStructureDescriptor1->GetSize();

    for (i = 0; i < size; i++) {
        distance = colorStructureDescriptor1->GetElement(i) / 255.0 - colorStructureDescriptor2->GetElement(i) / 255.0;
        L1error += fabs(distance);
    }

    if (distance == DBL_MAX) {
        throw COL_STRUCT_DISTANCE_INCORRECT;
    }

    return L1error;
}

ColorStructureDistance::~ColorStructureDistance() {
}