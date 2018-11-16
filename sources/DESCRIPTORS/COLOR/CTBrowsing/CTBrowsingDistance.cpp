#include "CTBrowsingDistance.h"

CTBrowsingDistance::CTBrowsingDistance() {
}

double CTBrowsingDistance::getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params) {
    CTBrowsing * cTBrowsingDescriptor1 = static_cast<CTBrowsing *>(descriptor1);
    CTBrowsing * cTBrowsingDescriptor2 = static_cast<CTBrowsing *>(descriptor2);

    int * cTbrowsingComponent1 = cTBrowsingDescriptor1->getCTBrowsingComponent();
    int * cTbrowsingComponent2 = cTBrowsingDescriptor2->getCTBrowsingComponent();

    double distance = abs((cTbrowsingComponent1[0] * 64 + cTbrowsingComponent1[1]) - (cTbrowsingComponent2[0] * 64 + cTbrowsingComponent2[1]));

    return distance;
}

CTBrowsingDistance::~CTBrowsingDistance() {
}
