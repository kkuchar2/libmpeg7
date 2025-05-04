#include "HomogeneousTextureExtractor.h"

HomogeneousTextureExtractor::HomogeneousTextureExtractor(): timage{}, inimage{}, image{}, mean2{}, dev2{}, m_dc(0), m_std(0), Num_pixel(0), hdata{}, vdata{}, dc(0), stdev(0), vec{}, dvec{} {
    descriptor = new HomogeneousTexture();
}

// Top level extraction
Descriptor * HomogeneousTextureExtractor::extract(Image & image, const char ** params) {
    descriptor->loadParameters(params);

    // Get image information
    const int imageWidth = image.getWidth();
    const int imageHeight = image.getHeight();
    const bool transparencyPresent = image.getTransparencyPresent();

    // Check size
    if (imageWidth < 128 || imageHeight < 128) {
        throw HOMOG_TEXT_IMAGE_TOO_SMALL_128;
    }

    // Get image data
    unsigned char * grayImg  = image.getGray(GRAYSCALE_AVERAGE);
    unsigned char * aChannel = transparencyPresent ? image.getChannel_A() : nullptr;

    if (transparencyPresent && aChannel) {
        // If alpha channel present, get only not transparent part
        try {
            ArbitraryShape(aChannel, grayImg, imageHeight, imageWidth);
        }
        catch (ErrorCode exception) {
            // Cleanup
            if (grayImg) {
                delete[] grayImg;
            }
            if (aChannel) {
                delete[] aChannel;
            }
            throw;
        }
    }
    // Extract feature
    FeatureExtraction(grayImg, imageHeight, imageWidth);

    delete[] grayImg;

    if (aChannel) {
        delete[] aChannel;
    }

    int HomogeneousTextureFeature[62];

    // Save 62 features to temporary feature array
    HomogeneousTextureFeature[0] = m_dc;
    HomogeneousTextureFeature[1] = m_std;

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 6; j++) {
            HomogeneousTextureFeature[i * 6 + j + 2] = mean2[i][j];
        }
    }

    // Get deviation layer flag (used as parameter)
    const int flag = descriptor->GetHomogeneousTextureFeatureFlag();

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 6; j++) {
            if (flag != 0) {
                HomogeneousTextureFeature[i * 6 + j + 30 + 2] = dev2[i][j];
            }
            else {
                HomogeneousTextureFeature[i * 6 + j + 30 + 2] = 0;
            }
        }
    }

    // Set descriptor data
    descriptor->SetHomogeneousTextureFeature(HomogeneousTextureFeature);

    return descriptor;
}

// Arbitrary shape computing
void HomogeneousTextureExtractor::ArbitraryShape(unsigned char * aChannel, unsigned char * grayImage, const int imageHeight, const int imageWidth) {
    int flag, a_min, a_max;
    int center_x, center_y;
    int a_size, pad_height_count, pad_width_count;

    int ** m_arbitraryShape;
    int ** m_arbitraryShape_temp;

    unsigned char ** m_arbitraryShape_patch;

    m_arbitraryShape      = static_cast<int **>(calloc(imageHeight + 2, sizeof(int *)));
    m_arbitraryShape_temp = static_cast<int **>(calloc(imageHeight + 2, sizeof(int *)));

    m_arbitraryShape_patch = static_cast<unsigned char **>(calloc(imageHeight, sizeof(unsigned char *)));

    for (int i = 0; i < imageHeight + 2; i++) {
        m_arbitraryShape[i]      = static_cast<int *>(calloc(imageWidth + 2, sizeof(int)));
        m_arbitraryShape_temp[i] = static_cast<int *>(calloc(imageWidth + 2, sizeof(int)));
    }

    for (int i = 0; i < imageHeight; i++) {
        m_arbitraryShape_patch[i] = static_cast<unsigned char *>(calloc(imageWidth, sizeof(unsigned char)));
    }

    flag = 0;

    for (int i = 1; i < imageHeight + 1; i++) {
        for (int j = 1; j < imageWidth + 1; j++) {
            if (aChannel[(i - 1) * imageWidth + (j - 1)] != static_cast<unsigned char>(0)) { // The pixel is white
                flag = 1;
                m_arbitraryShape[i][j] = 1;
                m_arbitraryShape_temp[i][j] = 1;
            }
        }
    }

    if (flag == 0) {
        //Cleanup
        for (int i = 0; i < imageHeight + 2; i++) {
            free(m_arbitraryShape[i]);
            free(m_arbitraryShape_temp[i]);
        }

        free(m_arbitraryShape);
        free(m_arbitraryShape_temp);

        for (int i = 0; i < imageHeight; i++) {
            free(m_arbitraryShape_patch[i]);
        }

        free(m_arbitraryShape_patch);

        throw HOMOG_TEXT_ARBITRARY_SHAPE_ERROR;
    }

    flag = 1;
    a_max = 0;

    while (flag) {
        flag = 0;
        for (int i = 0; i < imageHeight + 2; i++) {
            for (int j = 0; j < imageWidth + 2; j++) {
                m_arbitraryShape[i][j] = m_arbitraryShape_temp[i][j];

                if (maxtest(m_arbitraryShape[i][j], a_max)) {
                    center_y = i - 1;
                    center_x = j - 1;
                }
            }
        }

        for (int i = 1; i < imageHeight + 1; i++) {
            for (int j = 1; j < imageWidth + 1; j++) {
                if (m_arbitraryShape[i][j] != 0) {
                    a_min = m_arbitraryShape[i][j];

                    mintest(m_arbitraryShape[i - 1][j], a_min);
                    mintest(m_arbitraryShape[i + 1][j], a_min);
                    mintest(m_arbitraryShape[i][j - 1], a_min);
                    mintest(m_arbitraryShape[i + 1][j - 1], a_min);
                    mintest(m_arbitraryShape[i - 1][j - 1], a_min);
                    mintest(m_arbitraryShape[i][j + 1], a_min);
                    mintest(m_arbitraryShape[i + 1][j + 1], a_min);
                    mintest(m_arbitraryShape[i - 1][j + 1], a_min);

                    if (m_arbitraryShape[i][j] == a_min) {
                        m_arbitraryShape_temp[i][j] = m_arbitraryShape[i][j] + 1;
                        flag = 1;
                    }
                    else {
                        m_arbitraryShape_temp[i][j] = m_arbitraryShape[i][j];
                    }
                }
            }
        }

    } // while(flag == 1)

    a_size = a_max - 1;

    pad_height_count = static_cast<int>(((float) imageHeight) / (2.0 * ((float) a_size))) + 1;
    pad_width_count  = static_cast<int>(((float) imageWidth) / (2.0 * ((float) a_size))) + 1;

    for (int y = 0; y < pad_height_count; y++) {
        for (int x = 0; x < pad_width_count; x++) {
            for (int i = 0; i < 2 * a_size + 1; i++) {
                for (int j = 0; j < 2 * a_size + 1; j++) {
                    if (((i + y * (2 * a_size + 1)) < imageHeight) && ((j + x * (2 * a_size + 1)) < imageWidth)) {
                        m_arbitraryShape_patch[i + y * (2 * a_size + 1)][j + x * (2 * a_size + 1)] = grayImage[(center_y - a_size + i) * imageWidth + center_x - a_size + j];
                    }
                }
            }
        }
    }

    for (int i = 0; i < imageHeight; i++) {
        for (int j = 0; j < imageWidth; j++) {
            grayImage[i * imageWidth + j] = m_arbitraryShape_patch[i][j];
        }
    }

    // Cleanup
    for (int i = 0; i < imageHeight + 2; i++) {
        free(m_arbitraryShape[i]);
        free(m_arbitraryShape_temp[i]);
    }

    free(m_arbitraryShape);
    free(m_arbitraryShape_temp);

    for (int i = 0; i < imageHeight; i++) {
        free(m_arbitraryShape_patch[i]);
    }

    free(m_arbitraryShape_patch);
}

void HomogeneousTextureExtractor::mintest(const int a, int & min) {
    if (a < min) {
        min = a;
    }
}

bool HomogeneousTextureExtractor::maxtest(const int a, int & max) {
    if (a > max) {
        max = a;
        return true;
    }
    else {
        return false;
    }
}

// (KK) First level extraction
void HomogeneousTextureExtractor::FeatureExtraction(unsigned char * image, const int image_height, const int image_width) {
    int n, m;
    Num_pixel = 180 * 64;

    // Cridterion leads design
    vatomdesign();
    hatomdesign();

    SecondLevelExtraction(image, image_height, image_width);

    m_dc  = static_cast<int>(dc);
    m_std = static_cast<int>(stdev);

    for (n = 0; n < 5; n++) {
        for (m = 0; m < 6; m++) {
            mean2[n][m] = static_cast<int>(vec[n][m]); // mean : 30 energy features
        }
    }

    for (n = 0; n < 5; n++) {
        for (m = 0; m < 6; m++) {
            dev2[n][m] = static_cast<int>(dvec[n][m]); // dev	: 30 deviation features
        }
    }
}

// (KK) Criterion leads design
void HomogeneousTextureExtractor::vatomdesign() {
    int	i, k;
    const int vshft[6] = { 90, 60, 30, 0, -30, -60 };

    const auto buf = new double[3 * 180];

    double a, x, cnt;

    a   = 15. / sqrt(2. * log(2.));
    cnt = 89.5;

    for (i = 0; i < 180; i++) {
        x = (i - cnt) / a;

        buf[i]           = exp(-0.5 * x * x);
        buf[i + 180]     = exp(-0.5 * x * x);
        buf[i + 2 * 180] = exp(-0.5 * x * x);
    }

    for (k = 0; k < 6; k++) {
        for (i = 0; i < 180; i++) {
            vdata[k][i] = buf[i + 180 + vshft[k]] * buf[i + 180 + vshft[k]];
        }
    }

    delete[] buf;
}

void HomogeneousTextureExtractor::hatomdesign() {
    int i, k, size2;

    const int	shift2[5] = { 2, 5, 11, 23, 47 };

    const auto data = new double[5][3 * 128];

    const double BW[5] = { 2, 4, 8, 16, 32 };

    double par[5];

    double a, x, cnt;

    size2 = Nray / 2;

    for (k = 0; k < 5; k++) {
        par[k] = (BW[k] / 2.) / (sqrt(2. * log(2.)));
    }

    for (k = 0; k < 5; k++) {
        cnt = static_cast<double>(size2) + 0.5;

        for (i = 0; i < 128; i++) {
            x = (i - cnt);
            a = par[k] * par[k];

            data[k][i]           = exp(-0.5 * (x - 128) * (x - 128) / a);
            data[k][i + 128]     = exp(-0.5 *  x * x / a);
            data[k][i + 2 * 128] = exp(-0.5 * (x + 128) * (x + 128) / a);
        }
    }

    for (k = 0; k < 5; k++) {
        for (i = 0; i < 128; i++) {
            hdata[k][i] = data[k][i + Nray + shift2[k]] * data[k][i + Nray + shift2[k]];
        }
    }

    delete[] data;
}

// (KK) Second level extraction
void HomogeneousTextureExtractor::SecondLevelExtraction(unsigned char * imagedata, int image_height, const int image_width) {
    int	i;
    const auto cin = new unsigned char[imsize][imsize];

    const auto fin = new double[Nview][Nray];

    double count = 0, count2 = 0, count3 = 0, count4 = 0, count5 = 0;

    double sum = 0;
    double min = 1.e10;

    for (i = 0; i < 1024; i++) {
        timage[i]  = static_cast<COMPLEX *>(calloc(1024, sizeof(COMPLEX)));
    }
    for (i = 0; i < 512; i++) {
        inimage[i] = static_cast<COMPLEX *>(calloc(512, sizeof(COMPLEX)));
        image[i]   = static_cast<COMPLEX *>(calloc(512, sizeof(COMPLEX)));
    }

    // 2000.10.11 - yjyu@samsung.com
    for (i = 0; i < imsize; i++) {
        for (int j = 0; j < imsize; j++) {
            cin[i][j] = imagedata[i * image_width + j];
        }
    }

    // Perform Radon Transform
    RadonTransform(cin, fin, Nray, Nview);
    //dc= (dc) * (dc);	// 2001.01.31 - yjyu@samsung.com

    // Feature extraction
    Feature(fin, vec, dvec); 

    // Cleanup
    for (i = 0; i < 1024; i++) {
        free(timage[i]);
    }

    for (i = 0; i < 512; i++) {
        free(inimage[i]);
        free(image[i]);
    }

    delete[] fin;
    delete[] cin;

    // Quantization of features min max extract
    Quantization();
}

void HomogeneousTextureExtractor::RadonTransform(unsigned char(*cin)[imsize], double(*fin)[Nray], const int nr, const int nv) {
    int i, j;
    int size2, size3, nr2;

    COMPLEX out[512];

    double stepray, stepview;
    double view, ray, dt;
    double cosv, sinv;

    CARTESIAN cartesian;

    size3 = Nray / 2;

    dc = stdev = 0;

    for (i = 0; i < imsize; i++) {
        for (j = 0; j < imsize; j++) {
            inimage[i][j].r = static_cast<double>(cin[i][j]);
            inimage[i][j].i = 0;

            dc    +=  inimage[i][j].r;
            stdev +=  inimage[i][j].r * inimage[i][j].r;
        }
    }

    dc    = (dc)  / (imsize * imsize);
    stdev = stdev / (imsize * imsize);
    stdev = sqrt(stdev - dc * dc);

    FastFourierTransform2d(inimage, timage, imsize, 2, 2, 3, -2.0 / 3.0, -2.0 / 3.0);
    FastFourierTransform2d(inimage, timage, imsize, 1, 2, 3, -2.0 / 3.0, -1.0 / 3.0);
    FastFourierTransform2d(inimage, timage, imsize, 0, 2, 3, -2.0 / 3.0, -0.0 / 3.0);
    FastFourierTransform2d(inimage, timage, imsize, 2, 1, 3, -1.0 / 3.0, -2.0 / 3.0);
    FastFourierTransform2d(inimage, timage, imsize, 2, 0, 3, -0.0 / 3.0, -2.0 / 3.0);
    FastFourierTransform2d(inimage, timage, imsize, 1, 1, 3, -1.0 / 3.0, -1.0 / 3.0);
    FastFourierTransform2d(inimage, timage, imsize, 0, 1, 3, -1.0 / 3.0, -0.0 / 3.0);
    FastFourierTransform2d(inimage, timage, imsize, 1, 0, 3, -0.0 / 3.0, -1.0 / 3.0);
    FastFourierTransform2d(inimage, timage, imsize, 0, 0, 3, -0.0 / 3.0, -0.0 / 3.0);

    nr2 = nr / 2;
    size2 = imsize * 3;

    stepray  = 1.0 / nr;
    stepview = M_PI / nv;

    for (i = 0, view = 0; i < nv; i++, view = view + stepview) {
        cosv = cos(view) * nr;
        sinv = sin(view) * nr;

        for (j = 0, ray = 0; j < nr2; j++, ray = ray + stepray * 3) { // there are errors at this module.
            cartesian.x = (ray * cosv + size2 / 2); //nr * size;
            cartesian.y = (ray * sinv + size2 / 2); //nr * size;
            out[j + nr2] = GetProjectionFromFFT(cartesian, timage, size2);
        }

        for (j = 0; j < nr2; j++) {
            out[-j + nr2].r =  out[nr2 + j].r;
            out[-j + nr2].i = -out[nr2 + j].i;
        }

        out[0].r = out[0].i = 0;

        for (j = 0; j < nr; j++) {
            fin[i][j] = out[j].r * out[j].r + out[j].i * out[j].i; // power spectrum
        }
    }

    size3 = Nray / 2;

    for (j = 0; j < size3; j++) { // lam-rac filtering
        dt = (size3 - j) * (size3 - j) / 16.;
        for (i = 0; i < Nview; i++) {
            fin[i][j] = dt * fin[i][j];
        }
    }
}

void HomogeneousTextureExtractor::Feature(double(*fin)[128], double(*vec)[6], double(*dvec)[6]) {
    int i, j, n, m;
    double t;
    double deviation[5][6];

    for (n = 0; n < 5; n++) {
        for (m = 0; m < 6; m++) {
            vec[n][m] = 0;
            deviation[n][m] = 0;
        }
    }


    for (m = 0; m < 6; m++) {
        for (i = 0; i < 180; i++) { // # of angular feature channel = 6

            for (j = 0; j < 64; j++) {
                t = fin[i][j] * vdata[m][i] * hdata[0][j];

                vec[0][m] += t;
                deviation[0][m] += (t * t);
            }

            for (j = 0; j < 64; j++) {
                t = fin[i][j] * vdata[m][i] * hdata[1][j];

                vec[1][m] += t;
                deviation[1][m] += (t * t);
            }

            for (j = 0; j < 64; j++) {
                t = fin[i][j] * vdata[m][i] * hdata[2][j];

                vec[2][m] += t;
                deviation[2][m] += (t * t);
            }

            for (j = 0; j < 64; j++) {
                t = fin[i][j] * vdata[m][i] * hdata[3][j];

                vec[3][m] += t;
                deviation[3][m] += (t * t);
            }
            for (j = 0; j < 64; j++) {
                t = fin[i][j] * vdata[m][i] * hdata[4][j];

                vec[4][m] += t;
                deviation[4][m] += (t * t);
            }
        }
    }

    for (n = 0; n < 5; n++) {
        for (m = 0; m < 6; m++) {
            vec[n][m] /= Num_pixel;

            deviation[n][m] /= Num_pixel;
            deviation[n][m] = deviation[n][m] - vec[n][m] * vec[n][m];
            deviation[n][m] = sqrt(deviation[n][m]);

            vec[n][m] = log(1 + vec[n][m]);
            dvec[n][m] = log(1 + deviation[n][m]);
        }
    }
}

// FFTs
void HomogeneousTextureExtractor::FastFourierTransform2d(COMPLEX ** inimage, COMPLEX ** timage, const int size2, const int x, const int y, const int inc, const double dx, const double dy) {
    COMPLEX * ptr;
    COMPLEX  buf[2 * imsize];

    double pix, piy, theta;

    int cx, cy;
    int i, j, k;

    cx = -imsize / 2;
    cy = -imsize / 2;

    pix = 2 * M_PI * dx / size2;
    piy = 2 * M_PI * dy / size2;

    for (i = 0; i < size2; i++) {
        for (j = 0; j < size2; j++) {
            theta = (i + cy) * piy + (j + cx) * pix;
            image[i][j].r = inimage[i][j].r * cos(theta);
            image[i][j].i = inimage[i][j].r * sin(theta);
        }
        four1(image[i], size2, -1);
    }

    for (i = imsize / 2 - 1; i < size2; i++) {
        for (j = 0; j < size2; j++) {
            buf[j] = image[j][i];
        }

        four1(buf, size2, -1);
        ptr = buf;
        k = i * inc + y;

        for (j = x; j < size2 * inc; j += inc) {
            timage[k][j] = *ptr;
            ptr++;
        }
    }
}

void HomogeneousTextureExtractor::four1(COMPLEX * data1, const int nn, const int isign) {
    /* (KK) comment 
    Fast Fourier transform program, four1, from "Numerical Recipes in C"
    Replaces data[1..2 * nn] by its discrete Fourier transform, if isign is input as
    1; or replaces data[1..2*nn] by nn times its inverse discrete Fourier transform,
    if isign is input as -1.  data is a complex array of length nn or, equivalently,
    a real array of length 2*nn.  nn MUST be an integer power of 2 (this is not
    checked for!). */
    int n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;
    double * data;

    Swap(data1, nn);
    data = (double *) data1;

    n = nn << 1;
    j = 1;

    for (i = 1; i < n; i += 2) {
        if (j > i) {
            SWAP(data[j - 1], data[i - 1]);
            SWAP(data[j], data[i]);
        }

        m = n >> 1;

        while (m >= 2 && j > m) {
            j -= m;
            m >>= 1;
        }

        j += m;
    }

    mmax = 2;

    while (n > mmax) {
        istep = 2 * mmax;
        theta = 6.28318530717959 / (isign * mmax);
        wtemp = sin(0.5 * theta);

        wpr = -2.0 * wtemp * wtemp;
        wpi = sin(theta);
        wr  = 1.0;
        wi  = 0.0;

        for (m = 1; m < mmax; m += 2) {
            for (i = m; i <= n; i += istep) {
                j = i + mmax;
                tempr = wr * data[j - 1] - wi * data[j];
                tempi = wr * data[j]     + wi * data[j - 1];

                data[j - 1] = data[i - 1] - tempr;
                data[j]     = data[i]     - tempi;

                data[i - 1] += tempr;
                data[i]     += tempi;
            }
            wr = (wtemp = wr) * wpr - wi * wpi + wr;
            wi = wi * wpr + wtemp * wpi + wi;
        }

        mmax = istep;
    }
    Swap(data1, nn);
}

// Swap
void HomogeneousTextureExtractor::Swap(COMPLEX * data, const int size2) {
    int i, center = size2 / 2;
    COMPLEX tempr;

    for (i = 0; i < center; i++) {
        SWAP(data[i], data[i + center]);
    }
}

// Compute projection from fft complex data
COMPLEX HomogeneousTextureExtractor::GetProjectionFromFFT(const CARTESIAN cart, COMPLEX ** inimage, const int size2) {
    int x, y;
    int x2, y2;
    double rx, ry;

    COMPLEX ret;
    COMPLEX buf1, buf2;

    ret.r = ret.i = 0;

    x = static_cast<int>(cart.x);
    y = static_cast<int>(cart.y);

    rx = cart.x - x;
    ry = cart.y - y;

    if (x < 0 || y < 0 || x > size2 - 1 || y > size2 - 1) {
        return ret;
    }

    x2 = x + 1; // append
    y2 = y + 1;	// append

    if (x2 == size2) {
        x2 = 0; // append
    }

    if (y2 == size2) {
        y2 = 0;	// append
    }

    buf1.r = inimage[y][x].r  + (inimage[y][x2].r  - inimage[y][x].r) * rx;
    buf1.i = inimage[y][x].i  + (inimage[y][x2].i  - inimage[y][x].i) * rx;

    buf2.r = inimage[y2][x].r + (inimage[y2][x2].r - inimage[y2][x].r) * rx;
    buf2.i = inimage[y2][x].i + (inimage[y2][x2].i - inimage[y2][x].i) * rx;

    ret.r = buf1.r + (buf2.r - buf1.r)*ry;
    ret.i = buf1.i + (buf2.i - buf1.i)*ry;

    return ret;
}

// Final quantization
void HomogeneousTextureExtractor::Quantization() {
    int dc1, std1, m1, d1, n, m;
    double dcstep, stdstep, mstep, dstep;

    dcstep = (dcmax - dcmin) / Quant_level;
    dc1 = static_cast<int>((dc - dcmin) / (dcmax - dcmin) * Quant_level);

    if (dc1 > 255) {
        dc1 = 255;
    }

    else if (dc1 < 0) {
        dc1 = 0;
    }
    else;

    dc = dc1;
    stdstep = (stdmax - stdmin) / Quant_level;
    std1 = static_cast<int>((stdev - stdmin) / (stdmax - stdmin) * Quant_level);

    if (std1 > 255) {
        std1 = 255;
    }
    else if (std1 < 0) {
        std1 = 0;
    }
    else;

    stdev = std1;

    for (n = 0; n < 5; n++) {
        for (m = 0; m < 6; m++) {
            mstep = (mmax[n][m] - mmin[n][m]) / Quant_level;
            m1 = static_cast<int>((vec[n][m] - mmin[n][m]) / (mmax[n][m] - mmin[n][m]) * Quant_level);

            if (m1 > 255) {
                m1 = 255;
            }
            else if (m1 < 0) {
                m1 = 0;
            }
            else;

            vec[n][m] = m1;
        }
    }

    for (n = 0; n < 5; n++) {
        for (m = 0; m < 6; m++) {
            dstep = (dmax[n][m] - dmin[n][m]) / Quant_level;
            d1 = static_cast<int>((dvec[n][m] - dmin[n][m]) / (dmax[n][m] - dmin[n][m]) * Quant_level);

            if (d1 > 255) {
                d1 = 255;
            }
            else if (d1 < 0) {
                d1 = 0;
            }
            else;

            dvec[n][m] = d1;
        }
    }
}

HomogeneousTextureExtractor::~HomogeneousTextureExtractor() {
    delete descriptor;
}

const double HomogeneousTextureExtractor::mmax[5][6] = { 
{ 18.392888,18.014313,18.002143,18.083845,18.046575,17.962099 },
{ 19.368960,18.628248,18.682786,19.785603,18.714615,18.879544 },
{ 20.816939,19.093605,20.837982,20.488190,20.763511,19.262577 },
{ 22.298871,20.316787,20.659550,21.463502,20.159304,20.280403 },
{ 21.516125,19.954733,20.381041,22.129800,20.184864,19.999331 }};

const double HomogeneousTextureExtractor::mmin[5][6] = { 
{ 6.549734, 8.886816,8.885367,6.155831,8.810013,8.888925 },
{ 6.999376, 7.859269,7.592031,6.754764,7.807377,7.635503 },
{ 8.299334, 8.067422,7.955684,7.939576,8.518458,8.672599 },
{ 9.933642, 9.732479,9.725933,9.802238,10.076958,10.428015 },
{ 11.704927,11.690975,11.896972,11.996963,11.977944,11.944282 }};

const double HomogeneousTextureExtractor::dmax[5][6] = { 
{ 21.099482,20.749788,20.786944,20.847705,20.772294,20.747129 },
{ 22.658359,21.334119,21.283285,22.621111,21.773690,21.702166 },
{ 24.317046,21.618960,24.396872,23.797967,24.329333,21.688523 },
{ 25.638742,24.102725,22.687910,25.216958,22.334769,22.234942 },
{ 24.692990,22.978804,23.891302,25.244315,24.281915,22.699811 }};

const double HomogeneousTextureExtractor::dmin[5][6] = { 
{ 9.052970,11.754891,11.781252,8.649997,11.674788,11.738701 },
{ 9.275178,10.386329,10.066189,8.914539,10.292868,10.152977 },
{ 10.368594,10.196313,10.211122,10.112823,10.648101,10.801070 },
{ 11.737487,11.560674,11.551509,11.608201,11.897524,12.246614 },
{ 13.303207,13.314553,13.450340,13.605001,13.547492,13.435994 }};