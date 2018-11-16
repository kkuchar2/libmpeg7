/** @file   ContourShape.h
 *  @brief  Contour Shape descriptor data,
 *          method for loading parameters, read and wirte to XML,
 *          modify and access data.
 *
 *  @author Krzysztof Lech Kucharski
 *  @bug No bugs detected. */

#ifndef _CONTOURSHAPE_H 
#define _CONTOURSHAPE_H

#include "../../Descriptor.h"

#define BITS_TO_MASK(a)            ((2 << ((a) - 1)) - 1)

#define CONTOURSHAPE_YP            0.05

#define CONTOURSHAPE_AP           0.09
#define CONTOURSHAPE_MAXCSS       10
#define CONTOURSHAPE_T            0.000001
#define CONTOURSHAPE_TXA0         3.8
#define CONTOURSHAPE_TXA1         0.6

#define CONTOURSHAPE_CSSPEAKBITS  6
#define CONTOURSHAPE_XBITS        6
#define CONTOURSHAPE_YBITS        7
#define CONTOURSHAPE_YnBITS       3
#define CONTOURSHAPE_CBITS        6
#define CONTOURSHAPE_EBITS        6

#define CONTOURSHAPE_ETHR         0.6
#define CONTOURSHAPE_CTHR         1.0
#define CONTOURSHAPE_ECOST        0.4
#define CONTOURSHAPE_CCOST        0.3

#define CONTOURSHAPE_NMATCHPEAKS  2
#define CONTOURSHAPE_TMATCHPEAKS  0.9

#define CONTOURSHAPE_XMAX         1.0
#define CONTOURSHAPE_YMAX         1.7
#define CONTOURSHAPE_CMIN         12.0
#define CONTOURSHAPE_CMAX         110.0
#define CONTOURSHAPE_EMIN         1.0
#define CONTOURSHAPE_EMAX         10.0

#define CONTOURSHAPE_CSSPEAKMASK   BITS_TO_MASK(CONTOURSHAPE_CSSPEAKBITS)
#define CONTOURSHAPE_XMASK         BITS_TO_MASK(CONTOURSHAPE_XBITS)
#define CONTOURSHAPE_YMASK         BITS_TO_MASK(CONTOURSHAPE_YBITS)
#define CONTOURSHAPE_YnMASK        BITS_TO_MASK(CONTOURSHAPE_YnBITS)
#define CONTOURSHAPE_CMASK         BITS_TO_MASK(CONTOURSHAPE_CBITS)
#define CONTOURSHAPE_EMASK         BITS_TO_MASK(CONTOURSHAPE_EBITS)

class Point2 {
public:
    double x, y;
};

typedef struct {
    int   i;
    double x, y;
} IndexCoords;

typedef struct {
    int i;
    double x, dx;
} Edge;

class ContourShape : public Descriptor {
	private:
        // Descriptor data
        unsigned char  descriptorPeaksCount;
        unsigned long  descriptorGlobalCurvatureVector[2];
        unsigned long  descriptorPrototypeCurvatureVector[2];
        unsigned short descriptorHighestPeakY;
        unsigned short * descriptorPeaks = NULL;
	public:
        ContourShape();

        void loadParameters(const char ** params);
		void readFromXML(XMLElement * descriptorElement);
        std::string generateXML();

        void SetNumberOfPeaks(unsigned char cPeaks);
        void SetHighestPeakY(unsigned short iHigh);
        void SetPeak(unsigned char cIndex, unsigned short iX, unsigned short iY);
        void SetGlobalCurvature(unsigned long lC, unsigned long lE);
        void SetPrototypeCurvature(unsigned long lC, unsigned long lE);

        void GetGlobalCurvature(unsigned long &lC, unsigned long &lE) const;
        unsigned char GetNumberOfPeaks() const;
        void GetPeak(unsigned char cIndex, unsigned short &iX, unsigned  short &iY);

		~ContourShape();
};

#endif /* _CONTOURSHAPE_H */
