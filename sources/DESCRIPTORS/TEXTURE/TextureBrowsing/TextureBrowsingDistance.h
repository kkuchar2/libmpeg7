/** @file   TextureBrowsingDistance.h
*   @brief  Texture Browsing class for distance calculation.
*   @author Krzysztof Lech Kucharski
*   @bug    No bugs detected.                            */

#ifndef _TEXTURE_BROWSING_DISTANCE_H 
#define _TEXTURE_BROWSING_DISTANCE_H 

#include "../../DescriptorDistance.h"
#include "../TextureBrowsing/TextureBrowsing.h"

class TextureBrowsingDistance : public DescriptorDistance {
    private:
        TextureBrowsing * descriptor = nullptr;
        double distance_PBC(int flag, int * Ref, int * Query);
    public:
        TextureBrowsingDistance();
        double getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params);
        ~TextureBrowsingDistance();
};
#endif