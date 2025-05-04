#include "TextureBrowsingDistance.h"

TextureBrowsingDistance::TextureBrowsingDistance() = default;

double TextureBrowsingDistance::getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params) {
    TextureBrowsing * textureBrowsingDescriptor1 = static_cast<TextureBrowsing *>(descriptor1);
    TextureBrowsing * textureBrowsingDescriptor2 = static_cast<TextureBrowsing *>(descriptor2);

    int flag = textureBrowsingDescriptor1->GetComponentNumberFlag() * textureBrowsingDescriptor2->GetComponentNumberFlag();

    return distance_PBC(flag, textureBrowsingDescriptor1->getBrowsingComponent(), textureBrowsingDescriptor2->getBrowsingComponent());
}

double TextureBrowsingDistance::distance_PBC(int flag, int * Ref, int * Query) {
    double distance, temp1, temp2;

    temp1 = abs(Ref[1] - Query[1]);
    temp1 = (temp1 > 3) ? (6 - temp1) : temp1;

    distance = 0.6 * abs(Ref[0] - Query[0]) + 0.2 * (0.5 * (temp1 + abs(Ref[2] - Query[2])));

    if (flag != 0) {
        temp2 = abs(Ref[3] - Query[3]);
        temp2 = (temp2 > 3) ? (6 - temp2) : temp2;
        distance = distance + 0.2 * (0.5 * (temp2 + abs(Ref[4] - Query[4])));
    }

    return distance;
}

TextureBrowsingDistance::~TextureBrowsingDistance() = default;
