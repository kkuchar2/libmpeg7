#include "ColorLayoutDistance.h"

ColorLayoutDistance::ColorLayoutDistance() = default;

double ColorLayoutDistance::getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params) {
    const auto colorLayoutDescriptor1 = static_cast<ColorLayout *>(descriptor1);
    const auto colorLayoutDescriptor2 = static_cast<ColorLayout *>(descriptor2);

    int NY1, NY2, NC1, NC2, NY, NC;

    // Get number of coefficients
    NY1 = colorLayoutDescriptor1->getNumberOfYCoeffcients();
    NY2 = colorLayoutDescriptor2->getNumberOfYCoeffcients();

    NC1 = colorLayoutDescriptor1->getNumberOfCCoefficients();
    NC2 = colorLayoutDescriptor2->getNumberOfCCoefficients();

    // Get minimum of two coefficients count (for sum range)
    NY = (NY1 < NY2) ? NY1 : NY2;
    NC = (NC1 < NC2) ? NC1 : NC2;

    // Get coefficients values
    int * Y1, * Y2, * Cb1, * Cb2, * Cr1, * Cr2;

    Y1  = colorLayoutDescriptor1->getYCoefficients();
    Cb1 = colorLayoutDescriptor1->getCbCoefficients();
    Cr1 = colorLayoutDescriptor1->getCrCoefficients();

    Y2  = colorLayoutDescriptor2->getYCoefficients();
    Cb2 = colorLayoutDescriptor2->getCbCoefficients();
    Cr2 = colorLayoutDescriptor2->getCrCoefficients();

    // Set weight values:
    int weights[3][64];

    weights[0][0] = 3; weights[0][1] = weights[0][2] = 3;
    weights[1][0] = 2; weights[1][1] = weights[1][2] = 2;
    weights[2][0] = 4; weights[2][1] = weights[2][2] = 2;

    for (int i = 0; i < 3; i++) {
        for (int j = 3; j < 64; j++) {
            weights[i][j] = 1;
        }
    }

    /* 15938-8.doc - page 263
    Each part (under square root) is 
    sum (from i = 0 to max (CoeffNum1, CoeffNum2)) <- (accually it is minumum of two like here, because of uneven arrays size)
    of squared differences between coefficients multiplied by lambda(i), where lambda is weight */

    int j;
    int sum[3], diff;
    
    sum[0] = 0; // Y coefficients part - sum
    for (j = 0; j < NY; j++) {
        diff = (Y1[j] - Y2[j]);                  
        sum[0] += (weights[0][j] * diff * diff);
    }

    sum[1] = 0; // Cb coefficients part - sum
    for (j = 0; j < NC; j++) {
        diff = (Cb1[j] - Cb2[j]);
        sum[1] += (weights[1][j] * diff * diff);
    }

    sum[2] = 0; // Cr coefficients part - sum
    for (j = 0; j < NC; j++) {
        diff = (Cr1[j] - Cr2[j]);
        sum[2] += (weights[2][j] * diff * diff);
    }

    /* Calculate distance - sum of 3 square roots of (Y, Cb, Cr) sums
    Cast to float and back to double to have exact values. */
    return static_cast<double>(float(sqrt(sum[0] * 1.0) + sqrt(sum[1] * 1.0) + sqrt(sum[2] * 1.0)));
}

ColorLayoutDistance::~ColorLayoutDistance() {
}
