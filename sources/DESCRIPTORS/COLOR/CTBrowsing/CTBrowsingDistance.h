/** @file   CTBrowsingDistance.h
 *  @brief  Color Temperature Browsing class for distance calculation.
 *  @author Krzysztof Lech Kucharski
 *  @bug    No bugs detected.                                         */

#ifndef _CTBROWSINGDISTANCE_H 
#define _CTBROWSINGDISTANCE_H

#include "../../DescriptorDistance.h"
#include "../CTBrowsing/CTBrowsing.h"

class CTBrowsingDistance : public DescriptorDistance {
    public:
        CTBrowsingDistance();
        double getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params);
        ~CTBrowsingDistance();
};
#endif