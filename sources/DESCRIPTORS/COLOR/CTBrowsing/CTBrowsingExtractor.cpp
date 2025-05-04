#include "CTBrowsingExtractor.h"

CTBrowsingExtractor::CTBrowsingExtractor() {
    descriptor = new CTBrowsing();
}

Descriptor * CTBrowsingExtractor::extract(Image & image, const char ** params) {
    // Load parameters
    try {
        descriptor->loadParameters(params);
    }
    catch (ErrorCode exception) {
        throw exception;
    }
    
    // Get image inforamtion
    int imageWidth  = image.getWidth();
    int imageHeight = image.getHeight();

    // Discarding array
    unsigned char ** p_mask = allocateDiscardingArrayMemory(imageHeight, imageWidth);

    // Allocate memory for XYZ color space image's pixel values:
    double ** XYZ = allocateXYZ(imageHeight, imageWidth * 3);

    // Get image RGB values
    unsigned char * RGB = image.getRGB();

    // Convert RGB to sRGB and then sRGB to XYZ (1) (2)
    rgb2xyz(RGB, XYZ, p_mask, imageWidth, imageHeight);

    delete[] RGB;

    // Discard near black colors, average pixels (3) (4) and get chromacity of average color of the image (6)
    double pix, piy;
    perc_ill(XYZ, p_mask, imageWidth, imageHeight, &pix, &piy);

    // Convert (xs, ys) to (us vs) according to CIE (7)
    int * ctemperature = new(int);

    convert_xy2temp(pix, piy, ctemperature);

    // PCTBC Extraction
    int PCTBC[2];
    PCTBC_Extraction(ctemperature, PCTBC);

    // Set descriptor data
    descriptor->SetCTBrowsing_Component(PCTBC);

    // Free memory
    for (int i = 0; i < image.getHeight(); i++) {
        free(XYZ[i]);
        free(p_mask[i]);
    }

    free(XYZ);
    free(p_mask);
    delete ctemperature;

    return descriptor;
}

unsigned char ** CTBrowsingExtractor::allocateDiscardingArrayMemory(int height, int width) {
    // Allocate memory for rows:
    unsigned char ** mem = (unsigned char **) malloc(height * sizeof(unsigned char *));

    // Allocate memory for cols:
    for (int i = 0; i < height; i++) {
        mem[i] = (unsigned char *) malloc(width * sizeof(unsigned char));
    }
    return	mem;
}

double ** CTBrowsingExtractor::allocateXYZ(int height, int width) {
    // Allocate memory for rows
    double	** mem = (double **) malloc(height * sizeof(double *));

    // Allocate memory for cols
    for (int i = 0; i < height; i++) {
        mem[i] = (double *) malloc(width * sizeof(double));
    }
    return mem;
}

void CTBrowsingExtractor::rgb2xyz(unsigned char * imagebuff, double ** XYZ, unsigned char ** p_mask, int imageWidth, int imageHeight) {
    int i, j;
    double sRGBr, sRGBg, sRGBb;

    for (i = 0; i < imageHeight; i++) {
        for (j = 0; j < imageWidth; j++) {
            // Convert RGB to sRGB (1)
            rgb2srgb((int) imagebuff[(i * (imageWidth * 3)) + j * 3 + 0],
                     (int) imagebuff[(i * (imageWidth * 3)) + j * 3 + 1],
                     (int) imagebuff[(i * (imageWidth * 3)) + j * 3 + 2], &sRGBr, &sRGBg, &sRGBb);

            // Convert sRGB to XYZ with conversion matrix (2)
            XYZ[i][j * 3 + 0] = RGB2XYZ_M[0][0] * sRGBr + RGB2XYZ_M[0][1] * sRGBg + RGB2XYZ_M[0][2] * sRGBb;
            XYZ[i][j * 3 + 1] = RGB2XYZ_M[1][0] * sRGBr + RGB2XYZ_M[1][1] * sRGBg + RGB2XYZ_M[1][2] * sRGBb;
            XYZ[i][j * 3 + 2] = RGB2XYZ_M[2][0] * sRGBr + RGB2XYZ_M[2][1] * sRGBg + RGB2XYZ_M[2][2] * sRGBb;

            if ((XYZ[i][j * 3 + 0] < 0) || (XYZ[i][j * 3 + 1] < 0) || (XYZ[i][j * 3 + 2] < 0)) {
                XYZ[i][j * 3 + 0] = 0.0;
                XYZ[i][j * 3 + 1] = 0.0;
                XYZ[i][j * 3 + 2] = 0.0;
            }

            // This made no sense, because next method "perc_ill" overrides initially p_mask array values to 255 (duh!):
            /*  (XM continuation)
            if ((XYZ[i][j * 3 + 0] > 1.0) || (XYZ[i][j * 3 + 1] > 1.0) || (XYZ[i][j * 3 + 2] > 1.0)) {
            p_mask[i][j] = 255;
            } */
        }
    }
}

void CTBrowsingExtractor::rgb2srgb(int r, int g, int b, double * r_srgb, double * g_srgb, double * b_srgb) {
    *r_srgb = (r <= 0.03928 * 255.0) ? r / (255.0 * 12.92) : rgb_pow_table[r];
    *g_srgb = (g <= 0.03928 * 255.0) ? g / (255.0 * 12.92) : rgb_pow_table[g];
    *b_srgb = (b <= 0.03928 * 255.0) ? b / (255.0 * 12.92) : rgb_pow_table[b];
}

void CTBrowsingExtractor::perc_ill(double ** XYZ, unsigned char ** p_mask, int imageWidth, int imageHeight, double * pix, double * piy) {
    /* (KK) OMMIT DARK PIXELS FROM XYZ (3)
    If luminance component of threshold array is lower than 5 %,
    this pixel does not impact colour temperature perception,
    so make corresponding XYZ zero and mark this pixel position to 0 in mask value.

    (Mask value named "p" in (3) is equivalent to "p_mask" here) */

    double txyz[3];

    long lowlevel_pix_cnt  = 0;
    double lowlevelpercent = 0.05; // Threshold for pixel discarding (typical value: 5 %)

    // Get rid of low luminance pixels, (3)
    for (int i = 0; i < imageHeight; i++) {
        for (int j = 0; j < imageWidth; j++) {
            for (int k = 0; k < 3; k++) {
                txyz[k] = XYZ[i][j * 3 + k];  // Get current pixel xyz values to one treshold array
            }

            if (txyz[1] < lowlevelpercent) {  // Check luminance (Y) component for criteria
                XYZ[i][j * 3 + 0] = 0;
                XYZ[i][j * 3 + 1] = 0;
                XYZ[i][j * 3 + 2] = 0;
                lowlevel_pix_cnt++;	// count discarded pixels
                p_mask[i][j] = 0;
            }
            else {
                p_mask[i][j] = 255;	// Otherwise mask gets maximum value (in (3) is specified as "1", here it is 255)
            }
        }
    }

    /* Averaging remaining pixels (4) */
    int loop_cnt = 0;   // Counting averaging loop iterations
    long pix_cnt;	    // Number of pixels, which are not discarded (in documentation equal to "rows x cols" in equation
    int flag = 1;	    // Flag for indicating, whenever current threshold is equal to previous one, what ends averaging loop
    double f = 3.0;	    // Multiplier for the colour component average value to obtain threshold above which pixels are discarded (typical value is 3 - subjective experiments)   
    int iterations = 5; // Number of averaging loop iterations (typical number is in range from 4 to 8, maximum happened to be 20)
    double	p_th[3] = { 0.0, 0.0, 0.0 }; // Previous threshold for threshold comparison
    
                                         
    // (4):
    double	xyz_a[3];	// Averaged xyz value of pixels in image

    /* a) - threshold in current loop iteration,
    above which the pixels are discarded */
    double	xyz_Ts[3] = { 0.0, 0.0, 0.0 };			

    while (loop_cnt < iterations && flag == 1) {
        xyz_a[0] = 0.0;
        xyz_a[1] = 0.0;
        xyz_a[2] = 0.0;

        // b) - average of not discarded pixels:
        pix_cnt = 0;
        for (int i = 0; i < imageHeight; i++) {
            for (int j = 0; j < imageWidth; j++) {
                if (p_mask[i][j] != 0) {
                    xyz_a[0] += XYZ[i][j * 3 + 0];
                    xyz_a[1] += XYZ[i][j * 3 + 1];
                    xyz_a[2] += XYZ[i][j * 3 + 2];
                    pix_cnt++;
                }
            }
        }
        xyz_a[0] /= (double) (pix_cnt);
        xyz_a[1] /= (double) (pix_cnt);
        xyz_a[2] /= (double) (pix_cnt);

        // end of averaging for current iteration

        // Calculate treshold:
        for (int i = 0; i < 3; i++) {
            xyz_Ts[i] = f * xyz_a[i];
        }

        // c) 
        for (int i = 0; i < imageHeight; i++) {
            for (int j = 0; j < imageWidth; j++) {
                for (int k = 0; k < 3; k++) { // get current xyz values
                    txyz[k] = XYZ[i][j * 3 + k];
                }

                // if X(i,j) > current threshold, p_mask is 0, otherwise is 1 (so 255 as in the beginning)
                if (txyz[0] > xyz_Ts[0]) {
                    p_mask[i][j] = 0;
                }
                if (txyz[1] > xyz_Ts[1]) {
                    p_mask[i][j] = 0;
                }
                if (txyz[2] > xyz_Ts[2]) {
                    p_mask[i][j] = 0;
                }
            }
        }

        // d) If current threshold is equal to previous one, return Xa (or Ya or Za) -> that is why flag is used, so it stops the loop
        // We're seeing, that in the end of method Xa is used for calulating. We're good.

        if ((xyz_Ts[0] == p_th[0]) && (xyz_Ts[1] == p_th[1]) && (xyz_Ts[2] == p_th[2])) {
            flag = 0;
        }

        // Otherwise override "previous" threshold with current one
        for (int i = 0; i < 3; i++) {
            p_th[i] = xyz_Ts[i];
        }
        loop_cnt++;
    }

    // Use Xa (and Ya and Za) at the end and convert to chromacity (xs, ys),    (6)
    *pix = xyz_a[0] / (xyz_a[0] + xyz_a[1] + xyz_a[2]);
    *piy = xyz_a[1] / (xyz_a[0] + xyz_a[1] + xyz_a[2]);
}

void CTBrowsingExtractor::convert_xy2temp(double pix, double piy, int * ctemperature) {
    double u, v;
    xy2uv(pix, piy, &u, &v); // (7)
    *ctemperature = uv2ColorTemperature(u, v);
}

void CTBrowsingExtractor::xy2uv(double x, double y, double * u, double * v) {
    //-- Reference : Color Science 2nd Edition, pp. 828 (KK)
    *u = (4 * x) / (-2 * x + 12 * y + 3);
    *v = (6 * y) / (-2 * x + 12 * y + 3);
}

int CTBrowsingExtractor::uv2ColorTemperature(double iu, double iv) {
    //-- Reference : Color Science 2nd Edition, pp. 227~228 (KK)
    int i, idx1, idx2, nCT;
    double d1, d2, sqt1, sqt2;
    double invT1, invT2, invD;
    double mindist;
    int mindistIdx;

    idx1 = nCT = 0;
    idx2 = 1000;

    mindist = 10000.0;

    //-- for 31 isotemperature lines: notice i, i + 1 considered at one step
    //for(i=0; i<30; i++) {
    //-- for 27 isotemperature lines: notice i, i + 1 considered at one step

    for (i = 0; i < 26; i++) {
        sqt1 = sqrt(1.0 + isouv[i][2] * isouv[i][2]);
        d1 = ((iv - isouv[i][1]) - isouv[i][2] * (iu - isouv[i][0])) / sqt1;

        sqt2 = sqrt(1.0 + isouv[i + 1][2] * isouv[i + 1][2]);
        d2 = ((iv - isouv[i + 1][1]) - isouv[i + 1][2] * (iu - isouv[i + 1][0])) / sqt2;

        if ((d1 * d2) <= 0) {
            idx1 = i;
            idx2 = i + 1;
            break;
        }

        /* GG: zglaszane kiedys to mpega; ponizsze wystarczy robic dla pierwszej i ostatniej linii (temperatury poza zakresem), aby poprawnie 'ustawic' dla niebieskiego/czerwonego
        if(mindist > ABSVALUE(d1)) {
        mindist = ABSVALUE(d1);
        mindistIdx = i;
        if(mindist > ABSVALUE(d2)) {
        mindist = ABSVALUE(d2);
        mindistIdx = i+1;
        }
        }
        else if(mindist > ABSVALUE(d2)) {
        mindist = ABSVALUE(d2);
        mindistIdx = i+1;
        }
        */
        //gg -->: dla temperatur poza zakresem...

        if (i == 0) {
            mindist = ABSVALUE(d1);
            mindistIdx = i;
            if (d1 < 0) { //kolor w 'obszarze niebieskim' (na lewo od pierwszej linii), zadna inna linia nie moze byc blizsza
                break;
            }
        }
        else if (i == 25) {
            /*
            wystarczy sprawdzic czy ostatnia linia jest najblizsza - 
            jesli tak, to jestesmy w 'obszarze czerwonym' 
            (tak w ogole to czy fakt ze tu program dotarl, nie oznacza juz automatycznie, ze jest w 'obszarze czerwonym'?)  */
            if (mindist > abs(d2)) {//
            
                mindist = abs(d2);
                mindistIdx = i + 1;
            }
        }
    }

    // Using reciprocal temperature
    if (idx2 == 1000) {
        nCT = (int) isoTemp[mindistIdx];
    }
    else {
        invT1 = 1.0 / isoTemp[idx1];
        invT2 = 1.0 / isoTemp[idx2];
        invD = d1 / (d1 - d2);
        nCT = (int) (1.0 / (invT1 + invD * (invT2 - invT1)));
    }

    return nCT;
}

void CTBrowsingExtractor::PCTBC_Extraction(int * ctemperature, int * pctbc_out) {
    double firstRCT, secondRCT, RCTemperature;
    int exit = 0, index = 0;

    RCTemperature = double(1000000) / double(*ctemperature);

    if (*ctemperature < HotMaxTempK) {
        pctbc_out[0] = 0;
        firstRCT = rctHotMin;
        secondRCT = firstRCT - IncrRCTHotDiff;

        while ((secondRCT >= rctHotMax) && (!exit)) {
            if ((RCTemperature <= firstRCT) && (RCTemperature > secondRCT)) {
                /* gg: pctbc[1] nie jest nigdzie inicjalizowane, 
                a ze wzgledu na skonczona precyzje moze tu nie wejsc dla index=63 (bug zglaszany kiedys do mpeg-a) */
                pctbc_out[1] = index;
                exit = 1;
            }
            else {
                firstRCT = secondRCT;
                secondRCT -= IncrRCTHotDiff;
                index++;
            }
        }
        pctbc_out[1] = index; //gg: dodane, by uniknac losowej wartosci pctbc_out[1] (j.w.)
    }
    else if ((*ctemperature >= WarmMinTempK) && (*ctemperature < WarmMaxTempK)) {
        pctbc_out[0] = 1;
        firstRCT = rctWarmMin;
        secondRCT = firstRCT - IncrRCTWarmDiff;

        while ((secondRCT >= rctWarmMax) && (!exit)) {
            if ((RCTemperature <= firstRCT) && (RCTemperature > secondRCT)) {
                pctbc_out[1] = index; //gg: j.w.
                exit = 1;
            }
            else {
                firstRCT = secondRCT;
                secondRCT -= IncrRCTWarmDiff;
                index++;
            }
            pctbc_out[1] = index; //gg: dodane, by uniknac losowej wartosci pctbc_out[1] (j.w.)
        }
    }
    else if ((*ctemperature >= ModerateMinTempK) && (*ctemperature < ModerateMaxTempK)) {
        pctbc_out[0] = 2;
        firstRCT = rctModerateMin;
        secondRCT = firstRCT - IncrRCTModerateDiff;

        while ((secondRCT >= rctModerateMax) && (!exit)) {
            if ((RCTemperature <= firstRCT) && (RCTemperature > secondRCT)) {
                pctbc_out[1] = index; //gg: j.w.
                exit = 1;
            }
            else {
                firstRCT = secondRCT;
                secondRCT -= IncrRCTModerateDiff;
                index++;
            }
        }
        pctbc_out[1] = index; //gg: dodane, by uniknac losowej wartosci pctbc_out[1] (j.w.)
    }
    else {
        pctbc_out[0] = 3;
        firstRCT = rctCoolMin;
        secondRCT = firstRCT - IncrRCTCoolDiff;

        while ((secondRCT >= rctCoolMax) && (!exit)) {
            if ((RCTemperature <= firstRCT) && (RCTemperature > secondRCT)) {
                pctbc_out[1] = index; //gg: j.w.
                exit = 1;
            }
            else {
                firstRCT = secondRCT;
                secondRCT -= IncrRCTCoolDiff;
                index++;
            }
        }
        pctbc_out[1] = index; //gg: dodane, by uniknac losowej wartosci pctbc_out[1] (j.w.)
    }
}

CTBrowsingExtractor::~CTBrowsingExtractor() {
    delete descriptor;
}

// sRGB -> XYZ conversion matrix
const double CTBrowsingExtractor::RGB2XYZ_M[3][3] = {
    { 0.4124, 0.3576, 0.1805 },
    { 0.2126, 0.7152, 0.0722 },
    { 0.0193, 0.1192, 0.9505 } };

// Temperature related to isotemperature lines
const double CTBrowsingExtractor::isoTemp[27] = {
    25000, 20000, 16667, 14286, 12500, 11111,
    10000, 8000,  6667,  5714,  5000,  4444,
    4000,  3636,  3333,  3077,  2857,  2677,
    2500,  2353,  2222,  2105,  2000,  1905,
    1818,  1739,  1667 };

// Isotemperature lines: u, v, slope for 31 color temperature ranged 1667K ~ infinity.
const double CTBrowsingExtractor::isouv[27][3] = {
    { 0.18293,	0.27407, -0.30470 },
    { 0.18388,	0.27709, -0.32675 },
    { 0.18494,	0.28021, -0.35156 },
    { 0.18611,	0.28342, -0.37915 },
    { 0.18740,	0.28668, -0.40955 },
    { 0.18880,	0.28997, -0.44278 },
    { 0.19032,	0.29326, -0.47888 },
    { 0.19462,	0.30141, -0.58204 },
    { 0.19962,	0.30921, -0.70471 },
    { 0.20525,	0.31647, -0.84901 },
    { 0.21142,	0.32312, -1.0182 },
    { 0.21807,	0.32909, -1.2168 },
    { 0.22511,	0.33439, -1.4512 },
    { 0.23247,	0.33904, -1.7298 },
    { 0.24010,	0.34308, -2.0637 },
    { 0.24702,	0.34655, -2.4681 },
    { 0.25591,	0.34951, -2.9641 },
    { 0.26400,	0.35200, -3.5841 },
    { 0.27218,	0.35407, -4.3633 },
    { 0.28039,	0.35577, -5.3762 },
    { 0.28863,	0.35714, -6.7262 },
    { 0.29685,	0.35823, -8.5955 },
    { 0.30505,	0.35907, -11.324 },
    { 0.31320,	0.35968, -15.628 },
    { 0.32129,	0.36011, -23.325 },
    { 0.32931,	0.36038, -40.770 },
    { 0.33724,	0.36051, -116.45 } };

// Power function lookup table for RGB to sRGB conversion - about 200 ms profit vs standard pow function for single image,
const double CTBrowsingExtractor::rgb_pow_table[256] = {
    0.000833805, 0.000983677, 0.00114819, 0.00132772, 0.00152264, 0.00173331, 0.00196007, 0.00220325, 0.00246318, 0.00274017,
    0.00303452,  0.00334654,  0.00367651, 0.00402472, 0.00439144, 0.00477695, 0.00518152, 0.00560539, 0.00604883, 0.00651209,
    0.00699541,  0.00749903,  0.00802319, 0.00856813, 0.00913406, 0.00972122, 0.0103298,  0.0109601,  0.0116122,  0.0122865,
    0.012983,    0.0137021,   0.0144438,  0.0152085,  0.0159963,  0.0168074,  0.017642,   0.0185002,  0.0193824,  0.0202886,
    0.021219,    0.0221739,   0.0231534,  0.0241576,  0.0251869,  0.0262412,  0.0273209,  0.028426,   0.0295568,  0.0307134,
    0.031896,    0.0331048,   0.0343398,  0.0356013,  0.0368895,  0.0382044,  0.0395462,  0.0409152,  0.0423114,  0.043735,
    0.0451862,   0.0466651,   0.0481718,  0.0497066,  0.0512695,  0.0528606,  0.0544803,  0.0561285,  0.0578054,  0.0595112,
    0.0612461,   0.06301,     0.0648033,  0.0666259,  0.0684782,  0.0703601,  0.0722719,  0.0742136,  0.0761854,  0.0781874,
    0.0802198,   0.0822827,   0.0843762,  0.0865005,  0.0886556,  0.0908417,  0.093059,   0.0953075,  0.0975873,  0.0998987,
    0.102242,    0.104616,    0.107023,   0.109462,   0.111932,   0.114435,   0.116971,   0.119538,   0.122139,   0.124772,
    0.127438,    0.130136,    0.132868,   0.135633,   0.138432,   0.141263,   0.144128,   0.147027,   0.14996,    0.152926,
    0.155926,    0.158961,    0.162029,   0.165132,   0.168269,   0.171441,   0.174647,   0.177888,   0.181164,   0.184475,
    0.187821,    0.191202,    0.194618,   0.198069,   0.201556,   0.205079,   0.208637,   0.212231,   0.215861,   0.219526,
    0.223228,    0.226966,    0.23074,    0.234551,   0.238398,   0.242281,   0.246201,   0.250158,   0.254152,   0.258183,
    0.262251,    0.266356,    0.270498,   0.274677,   0.278894,   0.283149,   0.287441,   0.291771,   0.296138,   0.300544,
    0.304987,    0.309469,    0.313989,   0.318547,   0.323143,   0.327778,   0.332452,   0.337164,   0.341914,   0.346704,
    0.351533,    0.3564,      0.361307,   0.366253,   0.371238,   0.376262,   0.381326,   0.386429,   0.391572,   0.396755,
    0.401978,    0.40724,     0.412543,   0.417885,   0.423268,   0.42869,    0.434154,   0.439657,   0.445201,   0.450786,
    0.456411,    0.462077,    0.467784,   0.473531,   0.47932,    0.48515,    0.491021,   0.496933,   0.502886,   0.508881,
    0.514918,    0.520996,    0.527115,   0.533276,   0.539479,   0.545724,   0.552011,   0.55834,    0.564712,   0.571125,
    0.57758,     0.584078,    0.590619,   0.597202,   0.603827,   0.610496,   0.617207,   0.62396,    0.630757,   0.637597,
    0.64448,     0.651406,    0.658375,   0.665387,   0.672443,   0.679542,   0.686685,   0.693872,   0.701102,   0.708376,
    0.715694,    0.723055,    0.730461,   0.73791,    0.745404,   0.752942,   0.760525,   0.768151,   0.775822,   0.783538,
    0.791298,    0.799103,    0.806952,   0.814847,   0.822786,   0.83077,    0.838799,   0.846873,   0.854993,   0.863157,
    0.871367,    0.879622,    0.887923,   0.896269,   0.904661,   0.913099,   0.921582,   0.930111,   0.938686,   0.947307,
    0.955973,    0.964686,    0.973445,   0.982251,   0.991102,   1 };