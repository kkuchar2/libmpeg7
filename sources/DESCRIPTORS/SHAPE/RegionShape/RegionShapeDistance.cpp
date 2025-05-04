#include "RegionShapeDistance.h"

RegionShapeDistance::RegionShapeDistance() = default;

double RegionShapeDistance::getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params) {
    /*  
        UWAGA: W oryginale wniesiono poprawki w funckji initVOPGray,
               gdzie indeksowanie pikseli obrazu bylo niewlasciwe
        
        ----------------------------------------------------------------------------
        newbuf[i] = (buf[j++] + buf[j++] + buf[j++]) / 3
        ----------------------------------------------------------------------------
    
        zamieniono na:

        ----------------------------------------------------------------------------
        newbuf[i] = (buf[3 * j + 0] + buf[3 * j + 1] + buf[3 * j + 2]) / 3
        j++
        ----------------------------------------------------------------------------  */


    RegionShape * regionShapeDescriptor1 = static_cast<RegionShape *>(descriptor1);
    RegionShape * regionShapeDescriptor2 = static_cast<RegionShape *>(descriptor2);

    double distance = 0;

    for (int i = 0; i < ART_ANGULAR; i++) {
        for (int j = 0; j < ART_RADIAL; j++) {
            if (i != 0 || j != 0) {
                distance += fabs(regionShapeDescriptor1->GetRealValue(i, j) - regionShapeDescriptor2->GetRealValue(i, j));
            }
        }
    }

    return distance;
}

RegionShapeDistance::~RegionShapeDistance() = default;
