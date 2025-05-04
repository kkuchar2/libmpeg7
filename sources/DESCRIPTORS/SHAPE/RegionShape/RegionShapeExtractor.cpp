#include "RegionShapeExtractor.h"

RegionShapeExtractor::RegionShapeExtractor(): m_mass(0), m_centerX(0), m_centerY(0), m_radius(0), m_pBasisR{}, m_pBasisI{}, m_pCoeffR{}, m_pCoeffI{} {
    descriptor = new RegionShape();
}

Descriptor * RegionShapeExtractor::extract(Image & image, const char ** params) {
    // Load parameters
    try {
        descriptor->loadParameters(params);
    }
    catch (ErrorCode exception) {
        throw exception;
    }

    /* Generate basis LUT */
    double angle, temp, radius;
    int p, r;
    int x, y;
    int maxradius;

    maxradius = ART_LUT_SIZE / 2;

    for (y = 0; y < ART_LUT_SIZE; y++) {
        for (x = 0; x < ART_LUT_SIZE; x++) {
            radius = HYPOT((double) (x - maxradius), y - maxradius);
            if (radius < maxradius) {
                angle = atan2(y - maxradius, x - maxradius);

                for (p = 0; p < ART_ANGULAR; p++) {
                    for (r = 0; r < ART_RADIAL; r++) {

                        temp = cos(radius * M_PI * r / maxradius);

                        m_pBasisR[p][r][x][y] = temp * cos(angle * p);
                        m_pBasisI[p][r][x][y] = temp * sin(angle * p);
                    }
                }
            }
            else {
                for (p = 0; p < ART_ANGULAR; p++) {
                    for (r = 0; r < ART_RADIAL; r++) {
                        m_pBasisR[p][r][x][y] = 0;
                        m_pBasisI[p][r][x][y] = 0;
                    }
                }
            }
        }
    }

    /* Reset */
    for (int p = 0; p < ART_ANGULAR; p++) {
        for (int r = 0; r < ART_RADIAL; r++) {
            m_pCoeffR[p][r] = 0;
            m_pCoeffI[p][r] = 0;
        }
    }
    m_radius = 0;

    /* Find center of mass */
    unsigned int m10 = 0;
    unsigned int m01 = 0;


    /*  
    While creating image grayscale buffer average method as in original is used:
        
    gray = (R + G + B) / 3
        
    However results are different, because in original method gray pixel value is assigned
    in wrong way:

    -----------------------------------------------------------------------------------
    in method initVopGray from d_tools.cpp :
    -----------------------------------------------------------------------------------

    y[i] = (unsigned char)((imgBufUC[j++] + imgBufUC[j++] + imgBufUC[j++])/3);
    -----------------------------------------------------------------------------------

    For example if j = 720 and:

    imgBufUC[720] = 253
    imgBufUC[721] = 254
    imgBufUC[722] = 255

    expected value is 254.

   Because of increment operator and assigment order in all 3 segments of sum,
   only imgBufUC[720] is considered (so this is value 253),

   and then j is incremented after assigment to y[i]. 

   y[i] in that case has value 253, which is wrong result.

   Moreover, it should be considered to use luminosity method instead of simple average method.

   Pros:
      - it is better for human perception of color luminosity
      - there is no need to use redundant gray conversion method, because OpenCV does it already.  */

    const unsigned char * pImage = image.getGray(GRAYSCALE_AVERAGE);
    unsigned char size = 1;

    const int imageWidth  = image.getWidth();
    const int imageHeight = image.getHeight();

    m_mass = 0;

    int i = 0;
    for (y = 0; y < imageHeight; y++) {
        for (x = 0; x < imageWidth; x++) {
            if (pImage[i] < 128) {
                m_mass++;
                m10 += x;
                m01 += y;
            }
            i++;
        }
    }

    m_centerX = static_cast<double>(m10) / static_cast<double>(m_mass);
    m_centerY = static_cast<double>(m01) / static_cast<double>(m_mass);

    // Find radius:
    double temp_radius;

    i = 0;
    for (y = 0; y < imageHeight; y++) {
        for (x = 0; x < imageWidth; x++) {
            if (pImage[i] < 128) {
                temp_radius = HYPOT(x - m_centerX, y - m_centerY);
                if (temp_radius > m_radius) {
                    m_radius = temp_radius;
                }
            }
            i++;
        }
    }

    // Extract coefficients:
    double dx, dy, tx, ty;

    i = 0;
    for (y = 0; y < imageHeight; y++) {
        for (x = 0; x < imageWidth; x++) {
            if (pImage[i] < 128) {
                // Map image coordinate (x, y) to basis function coordinate (tx, ty)
                dx = x - m_centerX;
                dy = y - m_centerY;
                tx = ((dx * ART_LUT_RADIUS) / m_radius) + ART_LUT_RADIUS;
                ty = ((dy * ART_LUT_RADIUS) / m_radius) + ART_LUT_RADIUS;

                // Summation of basis function
                if (tx >= 0 && tx < ART_LUT_SIZE && ty >= 0 && ty < ART_LUT_SIZE) {
                    for (p = 0; p < ART_ANGULAR; p++) {
                        for (r = 0; r < ART_RADIAL; r++) {
                            m_pCoeffR[p][r] += GetReal(p, r, tx, ty);
                            m_pCoeffI[p][r] -= GetImg(p, r, tx, ty);
                        }
                    }
                }
            }
            i++;
        }
    }

    delete[] pImage;

    // Set descriptor data:
    for (int r = 0; r < ART_RADIAL; r++) {
        for (int p = 0; p < ART_ANGULAR; p++) {
            descriptor->SetElement(p, r, HYPOT(m_pCoeffR[p][r] / m_mass, m_pCoeffI[p][r] / m_mass));
        }
    }

    return descriptor;
}

double RegionShapeExtractor::GetReal(const int p, const int r, const double dx, const double dy) {
    const int x = static_cast<int>(dx);
    const int y = static_cast<int>(dy);

    const double ix = dx - x;
    const double iy = dy - y;

    const double x1 = m_pBasisR[p][r][x][y]     + (m_pBasisR[p][r][x + 1][y]     - m_pBasisR[p][r][x][y])     * ix;
    const double x2 = m_pBasisR[p][r][x][y + 1] + (m_pBasisR[p][r][x + 1][y + 1] - m_pBasisR[p][r][x][y + 1]) * ix;

    return (x1 + (x2 - x1) * iy);
}

double RegionShapeExtractor::GetImg(const int p, const int r, const double dx, const double dy) {
    const int x = static_cast<int>(dx);
    const int y = static_cast<int>(dy);

    const double ix = dx - x;
    const double iy = dy - y;

    const double x1 = m_pBasisI[p][r][x][y]     + (m_pBasisI[p][r][x + 1][y]     - m_pBasisI[p][r][x][y])     * ix;
    const double x2 = m_pBasisI[p][r][x][y + 1] + (m_pBasisI[p][r][x + 1][y + 1] - m_pBasisI[p][r][x][y + 1]) * ix;

    return (x1 + (x2 - x1) * iy);
}

RegionShapeExtractor::~RegionShapeExtractor() {
    delete descriptor;
}