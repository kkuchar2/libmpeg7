#include "DominantColorDistance.h"

#define Td2 255
#define sqr(x) ((x) * (x))

static double twopi = 2.0 * 3.14159265358979323846;
static float VAR_RECL = 60.0;
static float VAR_RECH = 90.0;

DominantColorDistance::DominantColorDistance() {
}

void DominantColorDistance::loadParameters(const char ** params) {
    if (params == nullptr) {
        return; // default value remains default
    }

    /* SIZE | PARAM COMBINATION
    ------------------------------------------------------------------
    0 |  [NULL]
    2 |  [VariancePresent, value, NULL]
    2 |  [SpatialCoherency, value, NULL]
    4 |  [VariancePresent, value, SpatialCoherency, value, NULL]
    4 |  [SpatialCoherency, value, VariancePresent, value, NULL]
    ------------------------------------------------------------------ */

    // Count parameters size
    int size = 0;
    for (int i = 0; params[i] != nullptr; i++) {
        size++;
    }

    // Check size
    if (size != 0 && size != 2 && size != 4) {
        throw DOM_COL_PARAMS_NUMBER_ERROR;
    }

    for (int i = 0; params[i] != nullptr; i += 2) {
        if (params[i] == nullptr || params[i + 1] == nullptr) {
            break; // default value remains default
        }

        std::string p1(params[i]);
        std::string p2(params[i + 1]);

        if (!p1.compare("VariancePresent")) {
            if (!p2.compare("0")) {
                variancePresent = false;
            }
            else if (!p2.compare("1")) {
                variancePresent = true;
            }
            else {
                throw DOM_COL_PARAM_VALUE_ERROR;
            }
        }
        else if (!p1.compare("SpatialCoherency")) {
            if (!p2.compare("0")) {
                spatialCoherencyPresent = false;
            }
            else if (!p2.compare("1")) {
                spatialCoherencyPresent = true;
            }
            else {
                throw DOM_COL_PARAM_VALUE_ERROR;
            }
        }
        else {
            throw DOM_COL_PARAM_NAME_ERROR;
        }
    }
}

double DominantColorDistance::getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params) {
    DominantColor * dominantColorDescriptor1 = static_cast<DominantColor *>(descriptor1);
    DominantColor * dominantColorDescriptor2 = static_cast<DominantColor *>(descriptor2);

    // ******************* LOAD PARAMETERS **********************************

    /* Quantization and color space support was not implemented (project requirements) 
       For distance calculation only VariancePresent and SpatialCoherency parameters are used. */  
    try {
        loadParameters(params);
    }
    catch (ErrorCode exception) {
        throw exception;
    }

    // ******************* GET DATA **********************************

    // Descriptor sizes
    int N1 = dominantColorDescriptor1->getResultDescriptorSize();
    int N2 = dominantColorDescriptor2->getResultDescriptorSize();

    // Dominant colors
    int ** dominantColors1 = dominantColorDescriptor1->getResultDominantColors();
    int ** dominantColors2 = dominantColorDescriptor2->getResultDominantColors();

    // Percentage values
    int * percentages1 = dominantColorDescriptor1->getResultPercentages();
    int * percentages2 = dominantColorDescriptor2->getResultPercentages();

    // ***************************** DISANCE CALCULATION **********************************

    double d, dist = 0.0, dmax;

    // ******************* COLOR CALCULATION **********************************
    double total;
    // N1 and N2 are color counts in each descriptor   
    float * per1_float = new float[N1];
    float * per2_float = new float[N2];

    float ** color1_float = new float * [N1];
    float ** color2_float = new float * [N2];

    for (int i = 0; i < N1; ++i) {
        color1_float[i] = new float[3];
    }

    for (int i = 0; i < N2; ++i) {
        color2_float[i] = new float[3];
    }

    total = 0.0;
    for (int i = 0; i < N1; i++) {
        rgb2luv(dominantColors1[i], color1_float[i], 3);
        per1_float[i] = static_cast<float>(((float) percentages1[i] + 0.5) / 31.9999);
        total += static_cast<double>(per1_float[i]);
    }

    for (int i = 0; i < N1; i++) {
        per1_float[i] /= static_cast<float>(total);
    }

    total = 0.0;
    for (int i = 0; i < N2; i++) {
        rgb2luv(dominantColors2[i], color2_float[i], 3);
        per2_float[i] = static_cast<float>(((float) percentages2[i] + 0.5) / 31.9999);
        total += static_cast<double>(per2_float[i]);
    }

    for (int i = 0; i < N2; i++) {
        per2_float[i] /= static_cast<float>(total);
    }

    /* If user specifed VariancePresent true for distance calculation and variances are present 
       in both descriptors. */
    if (variancePresent) {

        // ******************* OPTIONAL VARIANCE CALCULATION **********************************
        float ** vars1 = nullptr;
        float ** vars2 = nullptr;

        // Get descriptors variances
        int ** colorVariances1 = dominantColorDescriptor1->getResultColorVariances();
        int ** colorVariances2 = dominantColorDescriptor2->getResultColorVariances();

        vars1 = new float * [N1];

        if (colorVariances1 != nullptr) {
            for (int i = 0; i < N1; ++i) {
                vars1[i] = new float[3];

                vars1[i][0] = (colorVariances1[i][0] > 0) ? VAR_RECH : VAR_RECL;
                vars1[i][1] = (colorVariances1[i][1] > 0) ? VAR_RECH : VAR_RECL;
                vars1[i][2] = (colorVariances1[i][2] > 0) ? VAR_RECH : VAR_RECL;
            }
        }
        else { // (KK) This type of check for variance was added because XM was not initializing variances in descriptors with NULL
            for (int i = 0; i < N1; ++i) {
                vars1[i] = new float[3];

                vars1[i][0] = VAR_RECL;
                vars1[i][1] = VAR_RECL;
                vars1[i][2] = VAR_RECL;
            }
        }

        vars2 = new float * [N2];

        if (colorVariances2 != nullptr) {
            for (int i = 0; i < N2; ++i) {
                vars2[i] = new float[3];

                vars2[i][0] = (colorVariances2[i][0] > 0) ? VAR_RECH : VAR_RECL;
                vars2[i][1] = (colorVariances2[i][1] > 0) ? VAR_RECH : VAR_RECL;
                vars2[i][2] = (colorVariances2[i][2] > 0) ? VAR_RECH : VAR_RECL;
            }
        }
        else {
            for (int i = 0; i < N2; ++i) {
                vars2[i] = new float[3];

                vars2[i][0] = VAR_RECL;
                vars2[i][1] = VAR_RECL;
                vars2[i][2] = VAR_RECL;
            }
        }

        // Get Variance distance
        dist = GetDistanceVariance(per1_float, color1_float, vars1, N1, per2_float, color2_float, vars2, N2);

        for (int i = 0; i < N1; i++) {
            delete[] vars1[i];
        }
        delete[] vars1;

        for (int i = 0; i < N2; i++) {
            delete[] vars2[i];
        }
        delete[] vars2;
    }
    else {
        double Td = sqrt((double) Td2);

        dmax = 1.2 * Td;

        for (int i = 0; i < N1; i++) {
            dist += sqr(per1_float[i]);
        }

        for (int i = 0; i < N2; i++) {
            dist += sqr(per2_float[i]);
        }

        for (int i = 0; i < N1; i++) { 
            for (int j = 0; j < N2; j++) {
                d = sqrt(sqr(color1_float[i][0] - color2_float[j][0]) + sqr(color1_float[i][1] - color2_float[j][1]) + sqr(color1_float[i][2] - color2_float[j][2]));

                if (d < Td) {
                    dist -= 2 * (1 - d / dmax)*per1_float[i] * per2_float[j];
                }
            }
        }
        /* (KK) fabs function might be sensitive to very small numbers,
        like 1e-8, which might cause big difference with XM
        because of double precision. For example in XM result can be 0.0 
        and mpeg7libpw 1.003e-8.
        dist in XM will be 0 and dist in mpeg7libpw 1.003e-4.
        Final distance in that situation in XM will be 0 and 
        libmpeg7pw 0.0001003*100000000 = 10300.0, which is a big difference */
        
        // (KK) Fix:
        double FABS_EPS = 1.0e-7; // seems to be reasonable

        if (dist > FABS_EPS) {
            dist = sqrt(fabs(dist));
        }
        else {
            dist = 0.0;
        }
    }

    delete[] per1_float;

    for (int i = 0; i < N1; i++) {
        delete[] color1_float[i];
    }

    delete[] color1_float;
    delete[] per2_float;

    for (int i = 0; i < N2; i++) {
        delete[] color2_float[i];
    }

    delete[] color2_float;

    // Added by LG CIT
    int sc1 = static_cast<int>(dominantColorDescriptor1->getSpatialCoherencyValue());
    int sc2 = static_cast<int>(dominantColorDescriptor2->getSpatialCoherencyValue());

    /* If user not specified SpatialCoherency usage as parameter in distance calculation */
    if (!spatialCoherencyPresent) {
        sc1 = sc2 = 0;
    }

    /* otherwise */
    // ******************* OPTIONAL SPATIAL COHERENCY CALCULATION **********************************
    if ((sc1 != 0) && (sc2 != 0)) {
        float fsc1 = static_cast<float>(sc1) / static_cast<float>(31.0);
        float fsc2 = static_cast<float>(sc2) / static_cast<float>(31.0);
        dist = 0.3 * dist * fabs(fsc1 - fsc2) + 0.7 * dist;
    }

    double distance = 100000000.0 * dist;

    return distance;
}

double DominantColorDistance::GetDistanceVariance(float * per1, float ** color1, float ** var1, int size1, float * per2, float ** color2, float ** var2, int size2) {
    int     i1, i2;
    double  d0, d1, d2, v0, v1, v2, arg1, arg2;
    double  tmp, val = 0.0;

    /* The overall formula is:
    Integral of ( sum_ij f_i * f_j + sum_ij g_  *g_j - 2 * sum_ij f_j * g_j ) */

    // Loop for f_i * f_j
    for (i1 = 0; i1 < size1; i1++) {
        for (i2 = 0; i2 < size1; i2++) {
            d0 = color1[i1][0] - color1[i2][0];  v0 = var1[i1][0] + var1[i2][0];
            d1 = color1[i1][1] - color1[i2][1];  v1 = var1[i1][1] + var1[i2][1];
            d2 = color1[i1][2] - color1[i2][2];  v2 = var1[i1][2] + var1[i2][2];

            arg1 = (d0 * d0 / v0 + d1 * d1 / v1 + d2 * d2 / v2) / 2.0;
            arg2 = twopi * sqrt(twopi * v0 * v1 * v2);
            tmp = per1[i1] * per1[i2] * exp(-arg1) / arg2;
            val += tmp;
        }
    }

    // Loop for g_i * g_j
    for (i1 = 0; i1 < size2; i1++) {
        for (i2 = 0; i2 < size2; i2++) {
            d0 = color2[i1][0] - color2[i2][0];  v0 = var2[i1][0] + var2[i2][0];
            d1 = color2[i1][1] - color2[i2][1];  v1 = var2[i1][1] + var2[i2][1];
            d2 = color2[i1][2] - color2[i2][2];  v2 = var2[i1][2] + var2[i2][2];

            arg1 = (d0 * d0 / v0 + d1 * d1 / v1 + d2 * d2 / v2) / 2.0;
            arg2 = twopi * sqrt(twopi * v0 * v1 * v2);
            tmp = per2[i1] * per2[i2] * exp(-arg1) / arg2;
            val += tmp;
        }
    }

    // loop for f_i * g_j
    for (i1 = 0; i1 < size1; i1++) {
        for (i2 = 0; i2 < size2; i2++) {
            d0 = color1[i1][0] - color2[i2][0];  v0 = var1[i1][0] + var2[i2][0];
            d1 = color1[i1][1] - color2[i2][1];  v1 = var1[i1][1] + var2[i2][1];
            d2 = color1[i1][2] - color2[i2][2];  v2 = var1[i1][2] + var2[i2][2];

            arg1 = (d0 * d0 / v0 + d1 * d1 / v1 + d2 * d2 / v2) / 2.0;
            arg2 = twopi * sqrt(twopi * v0 * v1 * v2);
            tmp = per1[i1] * per2[i2] * exp(-arg1) / arg2;
            val -= 2.0 * tmp;
        }
    }
    return val;
}

void DominantColorDistance::rgb2luv(int * RGB, float * LUV, int size) {
    int i;
    double x, y, X, Y, Z, den, u2, v2, X0, Z0, Y0, u20, v20, r, g, b;

    X0 = (0.607 + 0.174 + 0.201);
    Y0 = (0.299 + 0.587 + 0.114);
    Z0 = (0.066 + 1.117);

    // Y0 = 1.0
    u20 = 4 * X0 / (X0 + 15 * Y0 + 3 * Z0);
    v20 = 9 * Y0 / (X0 + 15 * Y0 + 3 * Z0);

    for (i = 0; i<size; i += 3) {
        if (RGB[i] <= 20) {
            r = (double) (8.715e-4 * RGB[i]);
        }
        else {
            r = (double) pow((RGB[i] + 25.245) / 280.245, 2.22);
        }

        if (RGB[i + 1] <= 20) {
            g = (double) (8.715e-4 * RGB[i + 1]);
        }
        else {
            g = (double) pow((RGB[i + 1] + 25.245) / 280.245, 2.22);
        }

        if (RGB[i + 2] <= 20) {
            b = (double) (8.715e-4*RGB[i + 2]);
        }
        else {
            b = (double) pow((RGB[i + 2] + 25.245) / 280.245, 2.22);
        }

        X = 0.412453 * r + 0.357580 * g + 0.180423 * b;
        Y = 0.212671 * r + 0.715160 * g + 0.072169 * b;
        Z = 0.019334 * r + 0.119193 * g + 0.950227 * b;

        if (X == 0.0 && Y == 0.0 && Z == 0.0) {
            x = 1.0 / 3.0; y = 1.0 / 3.0;
        }
        else {
            den = X + Y + Z;
            x = X / den; y = Y / den;
        }

        den = -2 * x + 12 * y + 3;
        u2 = 4 * x / den;
        v2 = 9 * y / den;

        if (Y > 0.008856) {
            LUV[i] = (float) (116 * pow(Y, 1.0 / 3.0) - 16);
        }
        else {
            LUV[i] = (float) (903.3 * Y);
        }
        LUV[i + 1] = (float) (13 * LUV[i] * (u2 - u20));
        LUV[i + 2] = (float) (13 * LUV[i] * (v2 - v20));
    }
}

DominantColorDistance::~DominantColorDistance() {
}