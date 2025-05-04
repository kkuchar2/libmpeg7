/** @file   ColorLayoutDistance.h
 *  @brief  Color Layout class for distance calculation.
 *  @author Krzysztof Lech Kucharski
 *  @bug    No bugs detected.                            */

#ifndef _COLORLAYOUTDISTANCE_H 
#define _COLORLAYOUTDISTANCE_H

#include "../ColorLayout/ColorLayout.h"
#include "../../DescriptorDistance.h"

class ColorLayoutDistance : public DescriptorDistance {
    public:
        ColorLayoutDistance();
        double getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params);
        ~ColorLayoutDistance();
};
#endif