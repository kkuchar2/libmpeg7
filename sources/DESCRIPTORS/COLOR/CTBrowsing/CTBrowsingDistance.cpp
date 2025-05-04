#include "CTBrowsingDistance.h"

CTBrowsingDistance::CTBrowsingDistance() = default;

double CTBrowsingDistance::getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params) {
    const auto cTBrowsingDescriptor1 = static_cast<CTBrowsing *>(descriptor1);
    const auto cTBrowsingDescriptor2 = static_cast<CTBrowsing *>(descriptor2);

    const int * cTbrowsingComponent1 = cTBrowsingDescriptor1->getCTBrowsingComponent();
    const int * cTbrowsingComponent2 = cTBrowsingDescriptor2->getCTBrowsingComponent();

    const double distance = abs((cTbrowsingComponent1[0] * 64 + cTbrowsingComponent1[1]) - (cTbrowsingComponent2[0] * 64 + cTbrowsingComponent2[1]));

    return distance;
}

CTBrowsingDistance::~CTBrowsingDistance() = default;
