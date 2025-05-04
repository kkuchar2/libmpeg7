/** @file   CTBrowsingExtractor.h
*   @brief  Color Temperature Browsing class for extraction.
*   @author Krzysztof Lech Kucharski
*   @bug    No bugs detected.                            */

#ifndef _CTBROWSING_EXTRACTOR_H 
#define _CTBROWSING_EXTRACTOR_H

#include "../../DescriptorExtractor.h"
#include "../CTBrowsing/CTBrowsing.h"

#define ABSVALUE(a)	((a < 0) ? (a * -1) : a)

/* Temperature ranges */
#define HotMinTempK	     1667
#define HotMaxTempK      2251
#define WarmMinTempK     2251
#define WarmMaxTempK     4171
#define ModerateMinTempK 4171
#define ModerateMaxTempK 8061
#define CoolMinTempK     8061
#define CoolMaxTempK     25001

#define rctHotMin      double (1000000) / double (HotMinTempK)
#define rctHotMax      double (1000000) / double (HotMaxTempK)
#define rctWarmMin     double (1000000) / double (WarmMinTempK)
#define rctWarmMax     double (1000000) / double (WarmMaxTempK)
#define rctModerateMin double (1000000) / double (ModerateMinTempK)
#define rctModerateMax double (1000000) / double (ModerateMaxTempK)
#define rctCoolMin     double (1000000) / double (CoolMinTempK)
#define rctCoolMax     double (1000000) / double (CoolMaxTempK)

#define IncrRCTHotDiff      (rctHotMin - rctHotMax) 	      / 64.0
#define IncrRCTWarmDiff     (rctWarmMin - rctWarmMax) 	      / 64.0
#define IncrRCTModerateDiff (rctModerateMin - rctModerateMax) / 64.0
#define IncrRCTCoolDiff     (rctCoolMin - rctCoolMax)         / 64.0

class CTBrowsingExtractor : public DescriptorExtractor {
    private:
        CTBrowsing * descriptor = nullptr;

        unsigned char ** allocateDiscardingArrayMemory(int height, int width);
        double ** allocateXYZ(int height, int width);

        // CIE coords temperature estimation
        int  uv2ColorTemperature(double iu, double iv);

        // RGB -> XYZ
        void rgb2xyz(unsigned char * imagebuff, double** XYZ, unsigned char ** p_mask, int imageWidth, int imageHeight);

        // RGB -> sRGB
        void rgb2srgb(int r, int g, int b, double * r_srgb, double * g_srgb, double* b_srgb);

        // xs, ys to (u,v)
        void xy2uv(double x, double y, double * u, double * v);

        void perc_ill(double ** XYZ, unsigned char ** p_mask, int imageWidth, int imageHeight, double * pix, double * piy);

        void convert_xy2temp(double pix, double piy, int * ctemperature);

        void PCTBC_Extraction(int * ctemperature, int *pctbc_out);

        // LUT
        static const double RGB2XYZ_M[3][3];
        static const double isoTemp[27];
        static const double isouv[27][3];
        static const double rgb_pow_table[256];
    public:
	    CTBrowsingExtractor();
	    Descriptor * extract(Image & image, const char ** params);
        ~CTBrowsingExtractor();
};
#endif