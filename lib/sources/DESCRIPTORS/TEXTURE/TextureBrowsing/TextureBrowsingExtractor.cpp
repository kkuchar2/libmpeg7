#include "TextureBrowsingExtractor.h"

/* ----- CONSTRUCTOR ----- */
TextureBrowsingExtractor::TextureBrowsingExtractor() {
    descriptor = new TextureBrowsing();
}

/* ----- EXTRACTION ----- */
Descriptor * TextureBrowsingExtractor::extract(Image & image, const char ** params) {
    // Load parameters (layer)
    try {
        descriptor->loadParameters(params);
    }
    catch (ErrorCode exception) {
        throw exception;
    }

    // 2. Get image data
    int imageHeight = image.getHeight();
    int imageWidth = image.getWidth();
    unsigned char * grayImage = image.getGray(GRAYSCALE_AVERAGE);
    unsigned char * aChannel = NULL;

    // 3. Calculate arbitrary shape (for transparent images to use only not transparent data)
    if (image.getTransparencyPresent()) {
        aChannel = image.getChannel_A();
        ArbitraryShape(aChannel, grayImage, imageHeight, imageWidth);
    }

    // 4. Create empty matrix object from image
    Matrix * image_matrix;

    try {
        CreateMatrix(&image_matrix, imageHeight, imageWidth);
    }
    catch (ErrorCode exception) {
        delete[] grayImage;

        if (aChannel) {
            delete[] aChannel;
        }
        throw exception;
    }

    // 4. Fill image matrix with image data
    Convert2Matrix(grayImage, imageWidth, imageHeight, image_matrix);

    /* 5. Compute TBC (it called PBC here), which are Texture Browsing components:
    PBC = [regularity, direction1, scale1, direction2, scale2] */
    int PBC[5];

    try {
        PBC_Extraction(image_matrix, imageWidth, imageHeight, PBC);
    }
    catch (ErrorCode exception) {
        delete[] grayImage;

        if (aChannel) {
            delete[] aChannel;
        }
        throw exception;
    }

    delete[] grayImage;

    if (aChannel) {
        delete[] aChannel;
    }

    // 6. Set Texture Browsing data inside its descriptor object
    if (descriptor->GetComponentNumberFlag() == 0) { 
        PBC[3] = PBC[4] = 0;
        descriptor->SetBrowsing_Component(PBC);
    }
    else {
        descriptor->SetBrowsing_Component(PBC);
    }

    FreeMatrix(image_matrix);

    return descriptor;
}

/* ----- ARBITRARY SHAPE CALCULATION ----- */
void TextureBrowsingExtractor::ArbitraryShape(unsigned char * a_image, unsigned char * y_image, int image_height, int image_width) {
    int i, j, x, y;
    int flag, a_min, a_max;
    int center_x, center_y;
    int a_size, pad_height_count, pad_width_count;

    int ** m_arbitraryShape;
    int ** m_arbitraryShape_temp;
    unsigned char ** m_arbitraryShape_patch;

    m_arbitraryShape       = (int **) calloc(image_height + 2, sizeof(int *));
    m_arbitraryShape_temp  = (int **) calloc(image_height + 2, sizeof(int *));
    m_arbitraryShape_patch = (unsigned char **) calloc(image_height, sizeof(unsigned char *));

    for (i = 0; i<image_height + 2; i++) {
        m_arbitraryShape[i]      = (int *) calloc(image_width + 2, sizeof(int));
        m_arbitraryShape_temp[i] = (int *) calloc(image_width + 2, sizeof(int));
    }

    for (i = 0; i < image_height; i++) {
        m_arbitraryShape_patch[i] = (unsigned char *) calloc(image_width, sizeof(unsigned char));
    }

    for (i = 1; i < image_height + 1; i++) {
        for (j = 1; j < image_width + 1; j++) {
            if (a_image[(i - 1) * image_width + (j - 1)] != (unsigned char) (255)) { // the pixel is white
                m_arbitraryShape[i][j] = 1;
                m_arbitraryShape_temp[i][j] = 1;
            }
        }
    }

    flag = 1;
    a_max = 0;

    while (flag) {
        flag = 0;
        for (i = 0; i < image_height + 2; i++) {
            for (j = 0; j < image_width + 2; j++) {

                m_arbitraryShape[i][j] = m_arbitraryShape_temp[i][j];

                if (max_test(m_arbitraryShape[i][j], a_max)) {
                    center_y = i - 1;
                    center_x = j - 1;
                }
            }
        }

        for (i = 1; i < image_height + 1; i++) {
            for (j = 1; j < image_width + 1; j++) {

                if (m_arbitraryShape[i][j] != 0) {

                    a_min = m_arbitraryShape[i][j];

                    min_test(m_arbitraryShape[i - 1][j], a_min);
                    min_test(m_arbitraryShape[i + 1][j], a_min);
                    min_test(m_arbitraryShape[i][j - 1], a_min);

                    min_test(m_arbitraryShape[i + 1][j - 1], a_min);
                    min_test(m_arbitraryShape[i - 1][j - 1], a_min);

                    min_test(m_arbitraryShape[i][j + 1], a_min);
                    min_test(m_arbitraryShape[i + 1][j + 1], a_min);
                    min_test(m_arbitraryShape[i - 1][j + 1], a_min);

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

    }	//while(flag ==1)

    a_size = a_max - 1;
    pad_height_count = (int) (((float) image_height) / (2.0 * ((float) a_size))) + 1;
    pad_width_count  = (int) (((float) image_width)  / (2.0 * ((float) a_size))) + 1;

    for (y = 0; y < pad_height_count; y++) {
        for (x = 0; x < pad_width_count; x++) {
            for (i = 0; i < 2 * a_size + 1; i++) {
                for (j = 0; j < 2 * a_size + 1; j++) {
                    if (((i + y * (2 * a_size + 1)) < image_height) && ((j + x * (2 * a_size + 1)) < image_width))
                        m_arbitraryShape_patch[i + y * (2 * a_size + 1)]
                                              [j + x * (2 * a_size + 1)] = y_image[(center_y - a_size + i) * image_width + center_x - a_size + j];
                }
            }
        }
    }

    for (i = 0; i < image_height; i++) {
        for (j = 0; j < image_width; j++) {
            y_image[i * image_width + j] = m_arbitraryShape_patch[i][j];
        }
    }

    free(m_arbitraryShape);
    free(m_arbitraryShape_temp);
    free(m_arbitraryShape_patch);
}

void TextureBrowsingExtractor::min_test(int a, int & min) {
    if (a < min) {
        min = a;
    }
}

bool TextureBrowsingExtractor::max_test(int a, int & max) {
    if (a > max) {
        max = a;
        return true;
    }
    else {
        return false;
    }
}

/* ----- TBC EXTRACTION ----- */
void TextureBrowsingExtractor::PBC_Extraction(Matrix * img, int width, int height, int * pbc_out) {
    /* (KK) Method extracting TBC from image matrix */


    /* Extract both directions */
    int * temp_direction = (int *) malloc(2 * sizeof(int));

    try {
        GaborFeature(img, XM_SIDE, XM_UL, XM_UH, XM_SCALE, XM_ORIENTATION, XM_FLAG, temp_direction);
    }
    catch (ErrorCode exception) {
        if (temp_direction) {
            free(temp_direction);
        }
        throw exception;
    }

    struct pbc_struct pbc;

	// (KK) Initializing pbc struct 'element' array - XM was creating random values (was no initialized)
	pbc.element[0] = -1;
	pbc.element[1] = 0;
	pbc.element[2] = -1;
	pbc.element[3] = 0;

    // Save directions to struct
    pbc.element[1] = temp_direction[0];
    pbc.element[3] = temp_direction[1];

    free(temp_direction);

    /* Extract scale */
    int imgsize = width;

    try {
        pbcmain(&pbc, imgsize);
    }
    catch (ErrorCode exception) {
        throw exception;
    }

    /* Extract regularity */
    threshold(&pbc);

    pbc_out[0] = pbc.tstructuredness;   // REGULARITY
    pbc_out[1] = pbc.element[1];        // DIRECTION 1
    pbc_out[3] = pbc.element[3];        // DIRECTION 2
    pbc_out[2] = pbc.element[0];        // SCALE 1
    pbc_out[4] = pbc.element[2];        // SCALE 2  

}

void TextureBrowsingExtractor::cleaupMainVariables(int orientation, int scale, double  * temp_proj, double ** rowProjections, double ** columnProjections, float ** histo,
                         Matrix * FilteredImageBuffer[4][6], Matrix * Gr, Matrix * Gi, Matrix * Tmp_1, Matrix * Tmp_2, Matrix * F_1, Matrix * F_2, Matrix * G_real, Matrix * G_imag, 
                         Matrix * F_real, Matrix * F_imag, Matrix * IMG, Matrix * IMG_imag, Matrix * temp_FilteredImage) {
    free(temp_proj);

    for (int i = 0; i < orientation * scale; i++) {
        free(rowProjections[i]);
    }

    free(rowProjections);

    for (int i = 0; i < orientation * scale; i++) {
        free(columnProjections[i]);
    }

    free(columnProjections);

    for (int s = 0; s < scale; s++) {
        free(histo[s]);
    }

    free(histo);

    for (int s = 0; s < 4; s++) {
        for (int n = 0; n < 6; n++) {
            FreeMatrix(FilteredImageBuffer[s][n]);
        }
    }

    FreeMatrix(Gr);
    FreeMatrix(Gi);
    FreeMatrix(Tmp_1);
    FreeMatrix(Tmp_2);
    FreeMatrix(F_1);
    FreeMatrix(F_2);
    FreeMatrix(G_real);
    FreeMatrix(G_imag);
    FreeMatrix(F_real);
    FreeMatrix(F_imag);
    FreeMatrix(IMG);
    FreeMatrix(IMG_imag);

    FreeMatrix(temp_FilteredImage);
}

int TextureBrowsingExtractor::GaborFeature(Matrix * img, int side, double Ul, double Uh, int scale, int orientation, int flag, int * pbc) {
    /* (KK) Main steps of calculating directions (more at page 281-282 from 15938-8):
       - create new image matrix from default image matrix passed as argument
       - make Gabor Transform filtered image for each scale-direction combination for that matrix
       - calculate directions from those filtered images (using histograms)
       - calculate projections from filtered images and save them to files (later usage) */

    /* (KK) FIRST ALLOCATIONS AND VARIABLES */
    char  str[200];
    unsigned char dummy2;
    FILE * fp;

    int i, h, w, wid, hei, imgwid, imghei, s, n, base;
    int border, r1, r2, r3, r4;

    double dummy1;
    double sum_double;

    double ** rowProjections;
    double ** columnProjections;

    double  xshift = 0, yshift = 0;
    double	his_mean, his_std, his_threshold;
    float	**histo;
    int		*d_direction;

    char	OutBuffer[] = "RotatedImage";
    double  * temp_proj;
    double  angle[2];
    double 	fmin, fmax;

    Matrix * IMG      = NULL;
    Matrix * IMG_imag = NULL;
    Matrix * Gr       = NULL;
    Matrix * Gi       = NULL;
    Matrix * Tmp_1    = NULL;
    Matrix * Tmp_2    = NULL;
    Matrix * F_1      = NULL;
    Matrix * F_2      = NULL;
    Matrix * G_real   = NULL;
    Matrix * G_imag   = NULL;
    Matrix * F_real   = NULL;
    Matrix * F_imag   = NULL;
    Matrix * temp1    = NULL;
    Matrix * temp2    = NULL;
    Matrix * temp_FilteredImage = NULL;

    Matrix * FilteredImageBuffer[4][6];

    int ProjectionSize;

    imghei = img->height;
    imgwid = img->width;
    border = side;

    ProjectionSize = (int) (imgwid * 0.625);

    hei = (int) pow(2.0, ceil(log2((double) (img->height + 2.0 * border))));
    wid = (int) pow(2.0, ceil(log2((double) (img->width  + 2.0 * border))));

    rowProjections = (double **) malloc(orientation * scale * sizeof(double *));

    for (i = 0; i < orientation * scale; i++) {
        rowProjections[i] = (double *) malloc(ProjectionSize * sizeof(double));
    }

    columnProjections = (double **) malloc(orientation * scale * sizeof(double *));

    for (i = 0; i < orientation * scale; i++) {
        columnProjections[i] = (double *) malloc(ProjectionSize*sizeof(double));
    }
    /* (KK) Allocate all the ImageHeaders, imageData and Matrices. 
    Newly created images are all initialized to 0 by default */

    try {
        CreateMatrix(&IMG, hei, wid);
    }
    catch (ErrorCode exception) {
        for (i = 0; i < orientation * scale; i++) {
            free(rowProjections[i]);
        }

        free(rowProjections);

        for (i = 0; i < orientation * scale; i++) {
            free(columnProjections[i]);
        }

        if (IMG->data) {
            FreeMatrix(IMG);
        }

        throw exception;
    }

    r1 = img->width + border;
    r2 = img->width + border * 2;

    for (h = 0; h < border; h++) {
        for (w = 0; w < border; w++) {
            IMG->data[h][w] = img->data[border - 1 - h][border - 1 - w];
        }
        for (w = border; w < r1; w++) {
            IMG->data[h][w] = img->data[border - 1 - h][w - border];
        }
        for (w = r1; w < r2; w++) {
            IMG->data[h][w] = img->data[border - 1 - h][2 * img->width - w + border - 1];
        }
    }

    r1 = img->height + border;
    r2 = img->width + border;
    r3 = img->width + border * 2;

    for (h = border; h < r1; h++) {
        for (w = 0; w < border; w++) {
            IMG->data[h][w] = img->data[h - border][border - 1 - w];
        }
        for (w = border; w < r2; w++) {
            IMG->data[h][w] = img->data[h - border][w - border];
        }
        for (w = r2; w < r3; w++) {
            IMG->data[h][w] = img->data[h - border][2 * img->width - w + border - 1];
        }
    }

    r1 = img->height + border;
    r2 = img->height + border * 2;
    r3 = img->width + border;
    r4 = img->width + border * 2;

    for (h = r1; h < r2; h++) {
        for (w = 0; w < border; w++) {
            IMG->data[h][w] = img->data[2 * img->height - h + border - 1][border - 1 - w];
        }
        for (w = border; w < r3; w++) {
            IMG->data[h][w] = img->data[2 * img->height - h + border - 1][w - border];
        }

        for (w = r3; w < r4; w++) {
            IMG->data[h][w] = img->data[2 * img->height - h + border - 1][2 * img->width - w + border - 1];
        }
    }

    /* (KK)
    Allocate memory check for allocation exceptions and cleanup (when happened) */
    try {
        CreateMatrix(&F_real, hei, wid);
        CreateMatrix(&F_imag, hei, wid);
        CreateMatrix(&IMG_imag, hei, wid);
        CreateMatrix(&Gr, 2 * side + 1, 2 * side + 1);
        CreateMatrix(&Gi, 2 * side + 1, 2 * side + 1);
        CreateMatrix(&Tmp_1, hei, wid);
        CreateMatrix(&Tmp_2, hei, wid);
        CreateMatrix(&F_1, hei, wid);
        CreateMatrix(&F_2, hei, wid);
        CreateMatrix(&G_real, hei, wid);
        CreateMatrix(&G_imag, hei, wid);

        CreateMatrix(&temp_FilteredImage, imghei, imgwid);
    }
    catch (ErrorCode exception) {
        for (i = 0; i < orientation * scale; i++) {
            free(rowProjections[i]);
        }

        free(rowProjections);

        for (i = 0; i < orientation * scale; i++) {
            free(columnProjections[i]);
        }

        FreeMatrix(IMG);
        FreeMatrix(F_real);
        FreeMatrix(F_imag);
        FreeMatrix(IMG_imag);
        FreeMatrix(Gr);
        FreeMatrix(Gi);
        FreeMatrix(Tmp_1);
        FreeMatrix(Tmp_2);
        FreeMatrix(F_1);
        FreeMatrix(F_2);
        FreeMatrix(G_real);
        FreeMatrix(G_imag);

        throw exception; 
    }

    histo = (float **) malloc(scale * sizeof(float *));

    for (s = 0; s < scale; s++) {
        histo[s] = (float *) malloc(orientation * sizeof(float));
    }

    for (s = 0; s < scale; s++) {
        for (n = 0; n < orientation; n++) {
            histo[s][n] = 0.0;
        }
    }

    for (s = 0; s < 4; s++) {
        for (n = 0; n < 6; n++) {
            CreateMatrix(&FilteredImageBuffer[s][n], imghei, imgwid);
        }
    }

    /* 
    -----------------------------------------------------------
    (KK) ALLOCATIONS END
    End of allocations, it's time to calculate                  */

    /* ----------- Compute the Gabor filtered output ------------- */

    base = scale * orientation;
    Mat_FFT2(F_real, F_imag, IMG, IMG_imag);

    /* (KK) CREATE FILTERED IMAGES
    Computes 4 x 6 filtered images at different scales (4.3.2.1.3 Computation of the direction)  */
    for (s = 0; s < scale; s++) {
        for (n = 0; n < orientation; n++) {
            /* Gabor transfrom */
            Gabor(Gr, Gi, s + 1, n + 1, Ul, Uh, scale, orientation, flag);

            Mat_Copy(F_1, Gr, 0, 0, 0, 0, 2 * side, 2 * side);
            Mat_Copy(F_2, Gi, 0, 0, 0, 0, 2 * side, 2 * side);
            Mat_FFT2(G_real, G_imag, F_1, F_2);

            Mat_Product(Tmp_1, G_real, F_real);
            Mat_Product(Tmp_2, G_imag, F_imag);
            Mat_Substract(IMG, Tmp_1, Tmp_2);

            Mat_Product(Tmp_1, G_real, F_imag);
            Mat_Product(Tmp_2, G_imag, F_real);
            Mat_Sum(IMG_imag, Tmp_1, Tmp_2);

            Mat_IFFT2(Tmp_1, Tmp_2, IMG, IMG_imag);

            CreateMatrix(&temp1, imghei, imgwid);
            CreateMatrix(&temp2, imghei, imgwid);

            Mat_Copy(temp1, Tmp_1, 0, 0, 2 * side, 2 * side, imghei + 2 * side - 1, imgwid + 2 * side - 1);
            Mat_Copy(temp2, Tmp_2, 0, 0, 2 * side, 2 * side, imghei + 2 * side - 1, imgwid + 2 * side - 1);

            /* Fill image buffer with filtered data */
            for (h = 0; h < imghei; h++) {
                for (w = 0; w < imgwid; w++) {
                    dummy1 = sqrt(temp1->data[h][w] * temp1->data[h][w] + temp2->data[h][w] * temp2->data[h][w]);
                    FilteredImageBuffer[s][n]->data[h][w] = dummy1;
                }
            }

            FreeMatrix(temp1);
            FreeMatrix(temp2);
        }
    }

    /* (KK) DETECT DOMINANT DIRECTIONS
    Directions are based on directional histograms computed from Gabor transform filtered images at different scales
    So with 4 scales and 6 orientations we have 4 directional histograms as result. (4.3.2.1.3) */
    for (s = 0; s < scale; s++) {
        // Calulate mean histogram value
        sum_double = 0.0;
        for (n = 0; n < orientation; n++) {
            for (h = 0; h < imghei; h++) {
                for (w = 0; w < imgwid; w++) {
                    dummy1 = FilteredImageBuffer[s][n]->data[h][w];
                    sum_double += dummy1;
                }
            }
        }

        his_mean = sum_double / (orientation * imghei * imgwid);

        // Calculate standard deviation histogram value
        sum_double = 0.0;
        for (n = 0; n < orientation; n++) {
            for (h = 0; h < imghei; h++) {
                for (w = 0; w < imgwid; w++) {
                    dummy1 = FilteredImageBuffer[s][n]->data[h][w];
                    sum_double += pow((dummy1 - his_mean), 2);
                }
            }
        }

        his_std = sqrt(sum_double / (orientation * imghei * imgwid));

        // Calculate histogram with: mean + standard deviation what makes threshold histogram value
        his_threshold = his_mean + his_std;

        // Calculate histogram values from image values > threshold histogram value
        for (n = 0; n < orientation; n++) {
            for (h = 0; h < imghei; h++) {
                for (w = 0; w < imgwid; w++) {
                    dummy1 = FilteredImageBuffer[s][n]->data[h][w];

                    if (dummy1 > his_threshold) {
                        histo[s][n]++;
                    }
                }
            }
        }
    }

    /* (KK) 
    Identify final directions.
    Two directions with the two histogram-peaks of highest contrast are chosen (end of page 281 I guess) */
    d_direction = DominantDirection(histo);

    if (d_direction[0] == 6 && d_direction[1] == 6) {
        pbc[0] = 0;
        pbc[1] = 0;
        angle[0] = 0;
        angle[1] = 90;
    }
    else if (d_direction[0] != 6 && d_direction[1] == 6) {

        angle[0] = d_direction[0] * 30;
        angle[1] = angle[0] + 90;

        if (angle[1] > 180) {
            angle[1] = angle[1] - 180;
        }

        pbc[0] = d_direction[0] + 1;

        if (pbc[0] >= 4) {
            pbc[1] = 0;
        }
        else {
            pbc[1] = 0;
        }
    }
    else {
        angle[0] = d_direction[0] * 30;
        angle[1] = angle[0] + 90;

        if (angle[1] > 180) {
            angle[1] = angle[1] - 180;
        }

        pbc[0] = d_direction[0] + 1;
        pbc[1] = d_direction[1] + 1;
    }

    free(d_direction);

    /* (KK) After operations made above we have two values 
    stored in 'temp_direction' array (which here is called 'pbc')
    declared in method PBC_Extraction.

    temp_direction[direction1,   direction2]
                     ||                 ||
                     ||                 ||
    [regularity, direction1, scale1, direction2, scale2]  */


    /* (KK) PROJECTIONS COMPUTATION 
    Compute projections of rotated filtered images.
    As far as I see, all projections rows and cols are being written to seperate file (for later usage I guess) */
    for (s = 0; s < scale; s++) {
        for (n = 0; n < orientation; n++) {
            fmin = FilteredImageBuffer[s][n]->data[0][0]; fmax = fmin;

            for (h = 0; h < imghei; h++) {
                for (w = 0; w<imgwid; w++) {

                    if (FilteredImageBuffer[s][n]->data[h][w]>fmax) {
                        fmax = FilteredImageBuffer[s][n]->data[h][w];
                    }

                    if (FilteredImageBuffer[s][n]->data[h][w] < fmin) {
                        fmin = FilteredImageBuffer[s][n]->data[h][w];
                    }
                }
            }

            for (h = 0; h < imghei; h++) {
                for (w = 0; w < imgwid; w++) {
                    dummy1 = FilteredImageBuffer[s][n]->data[h][w];
                    dummy2 = (unsigned char) ((dummy1 - fmin) / (fmax - fmin) * 255);

                    temp_FilteredImage->data[h][w] = dummy2;
                }
            }

            // Here is the rotation
            ImgRotate(temp_FilteredImage, (float) angle[0]);

            temp_proj = (double *) malloc(ProjectionSize * sizeof(double));

            // And here projection H computation of this rotated image (4.3.2.1.4)
            try {
                ComputeProjection(temp_FilteredImage, imgwid, imghei, angle[0], ProjectionSize, temp_proj);
            }
            catch (ErrorCode exception) {
                // Cleanup
                cleaupMainVariables(orientation, scale, temp_proj, rowProjections, columnProjections, histo,
                                    FilteredImageBuffer, Gr, Gi, Tmp_1, Tmp_2, F_1, F_2, G_real, G_imag,
                                    F_real, F_imag, IMG, IMG_imag, temp_FilteredImage);
                // Throw level higher
                throw exception;
            }

            // Extract projections rows
            for (h = 0; h < ProjectionSize; h++) {
                rowProjections[s * orientation + n][h] = temp_proj[h];
            }

            /* (KK) Create directory for temporary projection files (based on operating system)
            On Windows when using tinyXML header it was not possible to create directory by classic way
            using windows.h header (XMLDocument redefinition error) */
            std::string directory_name = "text_brows_tmp";

            #if defined(_WIN32) || defined(_WIN64) // Windows
                struct stat info;
                // If directory does not exist
                if (stat(directory_name.c_str(), &info) != 0) { 
                    std::string command = "mkdir " +  directory_name;
                    system(command.c_str());
                }
            #else // Linux
                struct stat st = {0};
                // If directory does not exist
                if (stat("text_brows_tmp", &st) == -1) {
                    mkdir("text_brows_tmp", 0700);
                }
            #endif

            /* (KK) Save projection rows to files */
            sprintf(str, "text_brows_tmp/default_Row_%d%d", s, n);

            if ((fp = fopen(str, "wb")) == NULL) {
                // Cleanup
                cleaupMainVariables(orientation, scale, temp_proj, rowProjections, columnProjections, histo,
                                    FilteredImageBuffer, Gr, Gi, Tmp_1, Tmp_2, F_1, F_2, G_real, G_imag,
                                    F_real, F_imag, IMG, IMG_imag, temp_FilteredImage);                    
                // Throw level higher
                throw TEXT_BROWSING_CANNOT_OPEN_FILE;
            }

            fwrite(rowProjections[s * orientation + n], sizeof(double), ProjectionSize, fp);
            fclose(fp);

            for (h = 0; h < imghei; h++) {
                for (w = 0; w < imgwid; w++) {
                    dummy1 = FilteredImageBuffer[s][n]->data[h][w];
                    dummy2 = (unsigned char) ((dummy1 - fmin) / (fmax - fmin) * 255);
                    temp_FilteredImage->data[h][w] = dummy2;
                }
            }

            // Here is the rotation
            ImgRotate(temp_FilteredImage, (float) angle[1]);

            // And here projection V computation of this rotated image (4.3.2.1.4)
            try {
                ComputeProjection(temp_FilteredImage, imgwid, imghei, angle[1], ProjectionSize, temp_proj);
            }
            catch (ErrorCode exception) {
                // Cleanup
                cleaupMainVariables(orientation, scale, temp_proj, rowProjections, columnProjections, histo,
                                    FilteredImageBuffer, Gr, Gi, Tmp_1, Tmp_2, F_1, F_2, G_real, G_imag,
                                    F_real, F_imag, IMG, IMG_imag, temp_FilteredImage);
                // Throw level higher
                throw exception;
            }

            // Extract projections columns
            for (h = 0; h < ProjectionSize; h++) {
                columnProjections[s * orientation + n][h] = temp_proj[h];
            }

            /* (KK) Save projection columns to files */
            sprintf(str, "text_brows_tmp/default_Column_%d%d", s, n);

            if ((fp = fopen(str, "wb")) == NULL) {
                // Cleanup
                cleaupMainVariables(orientation, scale, temp_proj, rowProjections, columnProjections, histo,
                    FilteredImageBuffer, Gr, Gi, Tmp_1, Tmp_2, F_1, F_2, G_real, G_imag,
                    F_real, F_imag, IMG, IMG_imag, temp_FilteredImage);
                // Throw level higher
                throw TEXT_BROWSING_CANNOT_OPEN_FILE;
            }

            free(temp_proj);

            fwrite(columnProjections[s * orientation + n], sizeof(double), ProjectionSize, fp);
            fclose(fp);
        }
    }

    /* (KK) FREE MEMORY */
    for (i = 0; i < orientation * scale; i++) {
        free(rowProjections[i]);
    }

    free(rowProjections);

    for (i = 0; i < orientation * scale; i++) {
        free(columnProjections[i]);
    }

    free(columnProjections);

    for (s = 0; s < scale; s++) {
        free(histo[s]);
    }

    free(histo);

    for (s = 0; s < 4; s++) {
        for (n = 0; n < 6; n++) {
            FreeMatrix(FilteredImageBuffer[s][n]);
        }
    }

    FreeMatrix(Gr);
    FreeMatrix(Gi);
    FreeMatrix(Tmp_1);
    FreeMatrix(Tmp_2);
    FreeMatrix(F_1);
    FreeMatrix(F_2);
    FreeMatrix(G_real);
    FreeMatrix(G_imag);
    FreeMatrix(F_real);
    FreeMatrix(F_imag);
    FreeMatrix(IMG);
    FreeMatrix(IMG_imag);

    FreeMatrix(temp_FilteredImage);

    return 1;
}

void TextureBrowsingExtractor::Gabor(Matrix * Gr, Matrix * Gi, int s, int n, double Ul, double Uh, int scale, int orientation, int flag) {
    double base, a, u0, var, X, Y, G, t1, t2, m;
    int x, y, side;

    base = Uh / Ul;
    a = pow(base, 1.0 / (double) (scale - 1));

    u0 = Uh / pow(a, (double) scale - s);

    var = pow(0.6 / Uh * pow(a, (double) scale - s), 2.0);

    t1 = cos((double) M_PI / orientation * (n - 1.0));
    t2 = sin((double) M_PI / orientation * (n - 1.0));

    side = (int) (Gr->height - 1) / 2;

    for (x = 0; x < 2 * side + 1; x++) {
        for (y = 0; y < 2 * side + 1; y++) {

            X = (double) (x - side)  * t1 + (double) (y - side) * t2;
            Y = (double) -(x - side) * t2 + (double) (y - side) * t1;

            G = 1.0 / (2.0 * M_PI * var) * pow(a, (double) scale - s) * exp(-0.5 * (X * X + Y * Y) / var);

            Gr->data[x][y] = G*cos(2.0 * M_PI * u0 * X);
            Gi->data[x][y] = G*sin(2.0 * M_PI * u0 * X);
        }
    }

    /* If flag == 1, then remove the DC from the real part of Gabor */

    if (flag == 1) {
        m = 0;
        for (x = 0; x < 2 * side + 1; x++) {
            for (y = 0; y < 2 * side + 1; y++) {
                m += Gr->data[x][y];
            }
        }

        m /= pow((double) 2.0 * side + 1, 2.0);

        for (x = 0; x < 2 * side + 1; x++) {
            for (y = 0; y < 2 * side + 1; y++) {
                Gr->data[x][y] -= m;
            }
        }
    }
}

void TextureBrowsingExtractor::ImgRotate(Matrix * inImg, float angle) {
    int i, j, ci, cj;
    float sina, cosa, oldi, oldj;
    float alpha, beta;
    int NN, MM, ii, jj;
    Matrix *rImg;

    CreateMatrix(&rImg, inImg->height, inImg->width);

    angle = static_cast<float>(angle * (3.1415926535 / 180.0));

    MM = inImg->width;
    NN = inImg->height;

    sina = (float) sin(angle); cosa = (float) cos(angle);
    ci = NN / 2; cj = MM / 2;

    for (i = 0; i < NN; i++)
        for (j = 0; j < MM; j++) {
            oldi = (i - ci) * cosa - (j - cj) * sina + ci;
            oldj = (i - ci) * sina + (j - cj) * cosa + cj;

            ii = (int) oldi;
            jj = (int) oldj;

            alpha = oldi - (float) ii;
            beta = oldj - (float) jj;

            rImg->data[i][j] = (unsigned char) billinear(inImg, alpha, beta, ii, jj);
        }

    for (i = 0; i < NN; i++) {
        for (j = 0; j < MM; j++) {
            inImg->data[i][j] = rImg->data[i][j];
        }
    }

    FreeMatrix(rImg);
}

int * TextureBrowsingExtractor::DominantDirection(float ** histo) {
    int i, j, k;
    double *temp_ori, *out_ori;
    int *temp_index;
    struct Peak histo_peak[4][2];
    int 	candi_size = 2;
    int    temp_flag;
    int    lable, count;
    int	*index_value;
    double *contrast;
    double sum, *temp_contrast;
    int    *final_index, peak_count;
    struct Peak *temp_peak;
    double *temp_value;
    int *temp_c_index;

    temp_index = (int *) malloc(ORIENTATION * sizeof(int));
    temp_ori = (double *) malloc(ORIENTATION * sizeof(double));
    out_ori = (double *) malloc(ORIENTATION * sizeof(double));
    final_index = (int *) malloc(2 * sizeof(int));

    for (i = 0; i<SCALE; i++) {
        for (j = 0; j<ORIENTATION; j++) {
            temp_ori[j] = (double) histo[i][j];
        }

        temp_peak = (struct Peak *)malloc(ORIENTATION*sizeof(struct Peak));

        peak_count = 0;
        for (j = 0; j < ORIENTATION; j++) {
            if (j == 0) {
                if (temp_ori[j] > temp_ori[j + 1]) {
                    temp_peak[peak_count].index = j;
                    temp_peak[peak_count].value = temp_ori[j];
                    peak_count++;
                }
            }

            if (j == (ORIENTATION - 1)) {
                if (temp_ori[j] > temp_ori[j - 1]) {
                    temp_peak[peak_count].index = j;
                    temp_peak[peak_count].value = temp_ori[j];
                    peak_count++;
                }
            }

            if (j > 0 && j < (ORIENTATION - 1)) {
                if (temp_ori[j] >= temp_ori[j - 1] && temp_ori[j] >= temp_ori[j + 1]) {
                    temp_peak[peak_count].index = j;
                    temp_peak[peak_count].value = temp_ori[j];
                    peak_count++;
                }
            }
        }

        temp_value = (double *) malloc(peak_count * sizeof(double));

        for (j = 0; j < peak_count; j++) {
            temp_value[j] = temp_peak[j].value;
        }

        sort(out_ori, temp_index, temp_value, peak_count);

        if (peak_count == 1) {
            for (j = 0; j < candi_size; j++) {
                histo_peak[i][j].index = temp_peak[peak_count - 1].index;
                histo_peak[i][j].value = temp_peak[peak_count - 1].value;
                histo_peak[i][j].contrast = ComputeHistogramContrast(histo_peak[i][j].index, temp_ori, ORIENTATION);
                histo_peak[i][j].flag = 0;
            }
        }
        else {
            for (j = 0; j < candi_size; j++) {
                histo_peak[i][j].index = temp_peak[temp_index[peak_count - 1 - j]].index;
                histo_peak[i][j].value = out_ori[peak_count - 1 - j];
                histo_peak[i][j].contrast = ComputeHistogramContrast(histo_peak[i][j].index, temp_ori, ORIENTATION);
                histo_peak[i][j].flag = 0;
            }
        }
        free(temp_value);
        free(temp_peak);
    }

    for (i = 0; i < (SCALE - 1); i++) {
        for (j = 0; j < candi_size; j++) {
            temp_flag = JudgeContinuity(histo_peak[i][j].index, j, histo_peak[i + 1], candi_size);

            if (histo_peak[i][j].flag == 0) {
                histo_peak[i][j].flag = temp_flag;
            }
        }
    }

    sum = 0.0;
    for (i = 0; i < SCALE; i++) {
        for (j = 0; j < candi_size; j++) {
            sum += histo_peak[i][j].flag;
        }
    }

    if (sum == 0.0) {
        final_index[0] = 6;
        final_index[1] = 6;
    }
    else {
        count = 0;

        index_value = (int *) malloc(ORIENTATION * sizeof(int));

        for (i = 0; i < SCALE; i++) {
            lable = 0;
            for (j = 0; j < candi_size; j++) {
                if (histo_peak[i][j].flag == 1) {
                    index_value[count] = histo_peak[i][j].index;
                    lable = 1;
                    break;
                }
            }
            if (lable == 1) {
                break;
            }
        }

        for (i = 0; i < SCALE; i++) {
            for (j = 0; j < candi_size; j++) {
                if (histo_peak[i][j].flag == 1) {
                    lable = 0;

                    for (k = 0; k < count + 1; k++) {
                        if (histo_peak[i][j].index == index_value[k]) {
                            lable = 1;
                            break;
                        }
                    }
                    if (lable == 0) {
                        count++;
                        index_value[count] = histo_peak[i][j].index;
                    }
                }
            }
        }

        if (count == 0) {
            final_index[0] = index_value[0];
            final_index[1] = 6;
        }
        else {
            contrast = (double *) malloc((count + 1) * sizeof(double));

            for (i = 0; i < count + 1; i++) {
                contrast[i] = 0.0;

                for (j = 0; j < SCALE; j++) {
                    for (k = 0; k < candi_size; k++) {
                        if (histo_peak[j][k].flag == 1 && histo_peak[j][k].index == index_value[i]) {
                            if (histo_peak[j][k].contrast > contrast[i]) {
                                contrast[i] = histo_peak[j][k].contrast;
                            }
                        }
                    }
                }
            }

            temp_contrast = (double *) malloc((count + 1) * sizeof(double));
            temp_c_index = (int *) malloc((count + 1) * sizeof(int));

            sort(temp_contrast, temp_index, contrast, count + 1);

            for (j = 0; j < 2; j++) {
                for (i = 0; i < count + 1; i++) {
                    if (contrast[i] == temp_contrast[count - j]) {
                        final_index[j] = index_value[i];
                        break;
                    }
                }
            }
            free(contrast);
            free(temp_contrast);
            free(temp_c_index);
        }
        free(index_value);
    }

    free(temp_index);
    free(temp_ori);
    free(out_ori);

    return final_index;
}

void TextureBrowsingExtractor::ComputeProjection(Matrix * inputImage, int xsize, int ysize, double angle, int proj_size, double * proj) {
    int xcenter, ycenter;
    int j, l, count_pixel;
    double sum_pixel;
    unsigned char dummy;

    xcenter = xsize / 2;
    ycenter = ysize / 2;

    for (j = (xcenter - proj_size / 2); j < (xcenter + proj_size / 2); j++) {
        count_pixel = 0;
        sum_pixel = 0.0;

        for (l = 0; l < ysize; l++) {
            dummy = 0; 
            if (sqrt(pow((double) (j - xcenter), 2) + pow((double) (l - ycenter), 2)) <= 127.0) {
                if (l >= inputImage->height || j >= inputImage->width) {
                    throw TEXT_BROWS_PROJECTION_COMPUTATION_ERROR;
                }
                dummy = (unsigned char) inputImage->data[l][j];
                sum_pixel += (double) dummy;
                count_pixel++;
            }
        }

        proj[j - (xcenter - proj_size / 2)] = sum_pixel / count_pixel;
    }
}

/* ----- SCALE CALCULATION MAIN METHODS I GUESS  ----- */
void TextureBrowsingExtractor::pbcmain(struct pbc_struct * pbc, int size) {
    char c5[200];
    float row_credit[3], column_credit[3], image_credit;

    int img_size;

    img_size = (int) (size * 0.625);

    sprintf(c5, "default");

    try {
        ProjectionAnalysis(c5, 1, row_credit, pbc, img_size);
        ProjectionAnalysis(c5, 2, column_credit, pbc, img_size);
    }
    catch (ErrorCode exception) {
        throw exception;
    }

    image_credit = row_credit[2] + column_credit[2];

    pbc->structuredness = image_credit;
}

int TextureBrowsingExtractor::RadonAutocorrelation(double * x_in, int x_long, double ** y) {
    int x_start, x_end, long0, long1, i, j, k, cut;
    double *x, *sum, *A, *C, temp1, temp2, temp3, *temp_out;

    // (KK) Ommits zeros from the left side of projection
    x_start = 1;
    for (i = 0; i < x_long; i++) {
        if (x_in[i] != 0) {
            x_start = i;
            break;
        }
    }

    x_end = x_long - 1;

    // (KK) Ommits zeros from the right side of projection
    for (i = x_long - 1; i >= 0; i--) {
        if (x_in[i] != 0) {
            x_end = i;
            break;
        }
    }

    // Lenghts of sum calculation
    long0 = x_end - x_start + 1;
    long1 = long0 - 1;

    // Array for considered projection elements (after cutting zeros)
    x = (double *) calloc(long0, sizeof(double));

    // Fill it
    for (i = 0; i < long0; i++) {
        x[i] = x_in[i + x_start];
    }

    // Array for NAC
    sum = (double *) calloc(long1, sizeof(double));

    // For each element
    for (k = 0; k < long1; k++) {
        sum[k] = 0;
        /*
        A --> P(m-k) in range from m = k to N - 1
        C --> P(m)   in range from m = k to N - 1 */
        A = (double *) calloc(long0 - k, sizeof(double));
        C = (double *) calloc(long0 - k, sizeof(double));

        for (j = k; j < long0; j++) {
            A[j - k] = x[j];
        }

        for (j = 0; j < long0 - k; j++) {
            C[j] = x[j];
        }

        temp1 = 0.0;
        temp2 = 0.0;
        temp3 = 0.0;


        for (j = 0; j < long0 - k; j++) {
            temp1 += A[j] * C[j];   // sum from m = k to N - 1 of P(m - k) * P (m)
            temp2 += A[j] * A[j];   // sum from m = k to N - 1 of (P(m - k))^2
            temp3 += C[j] * C[j];   // sum from m = k to N - 1 of (P(m))^2
        }

        if (temp2 == 0 || temp3 == 0) {
            sum[k] = 0;
        }
        else {
            /*
            (KK) 4.3.2.1.4 Autocorrelation:
            NAC(k) =   sum(m-k to N-1, P(m-k) * P(m))  / sqrt ( (sum(m=k to N-1, (P(m-k))^2) * sum(m=k to N-1, (P(m))^2))) */
            sum[k] = temp1 / (sqrt(temp2) * sqrt(temp3));
        }
        free(A); free(C);
    }

    /* (KK)
    Here is part I don't understand - after autocorrelation it still makes operations on NAC. Some kind of (threshold?) cut. */

    temp_out = (double *) calloc(2 * long1 - 1, sizeof(double));

    for (i = 0; i < long1 - 1; i++) {
        temp_out[i] = sum[long1 - i - 1];
    }

    for (i = long1 - 1; i < 2 * long1 - 1; i++) {
        temp_out[i] = sum[i - long1 + 1];
    }

    cut = static_cast<int>(round((float) ((long1 - 1) * 0.75)));

    *y = (double *) calloc(2 * cut + 1, sizeof(double));

    for (i = 0; i < cut; i++) {
        (*y)[i] = temp_out[long1 - 1 - cut + i];
    }

    for (i = 0; i < cut; i++) {
        (*y)[i + cut + 1] = temp_out[long1 + i];
    }

    (*y)[cut] = temp_out[long1 - 1];

    free(temp_out);
    free(sum);
    free(x);

    return 2 * cut + 1;
}

void TextureBrowsingExtractor::ProjectionAnalysis(char * image_name, int proj_type, float * credit, pbc_struct * pbc, int img_size) {
    char c5[200];

    float ** Proj_Candi_Valu = NULL;
    float ** contrast = NULL;
    float * Peak, peak_diff, dis_ratio, dis_peak, var_dis;
    float ** picked_cand, max_contrast;
    int ** Proj_Candi_Posi = NULL;
    int Candi_Avail, count_candi, leng_B, size_peak, *PeakI, flag;
    int * refine_cand, pick_num, ** picked_cand_posi;
    int i, j, k;
    double * A, *B;

    FILE * fp;

    Proj_Candi_Posi = (int **) AllocateMatrixInteger(24, 2);
    Proj_Candi_Valu = (float **) AllocateMatrixFloat(24, 2);
    Candi_Avail = 0;
    count_candi = 0;
    contrast = (float **) AllocateMatrixFloat(4, 6);

    // I guess A is used for the storage of the projections
    A = (double *) calloc(img_size, sizeof(double));

    // For each filtered image W_mn(x,y) (4.3.2.1.4 Computation of the scale, Projection)
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 6; j++) {
            /* PROJECTION READ FROM FILE        
            Choose projection files names based on projection type currently calculated
            for corresponding scale and orientation (i, j) */
            if (proj_type == 1) {
                sprintf(c5, "text_brows_tmp/%s_Row_%d%d", image_name, i, j);
            }
            else if (proj_type == 2) {
                sprintf(c5, "text_brows_tmp/%s_Column_%d%d", image_name, i, j);
            }

            /* Open current scale and orientation projection file */
            fp = fopen(c5, "rb");

            if (!fp) { // Could not open file
                free(A);
                FreeMatrixInteger(Proj_Candi_Posi, 24);
                FreeMatrixFloat(Proj_Candi_Valu, 24);
                FreeMatrixFloat(contrast, 4);
                throw TEXT_BROWS_FILE_READ_ERROR; 
            }

            /* Read projection file to A array */
            fread(A, sizeof(double), img_size, fp);
            fclose(fp);

            /* PROJECTION READ END */

            /* (KK) 
            Autocorreclation of Radon transform (which projections aready went through before) */
            leng_B = RadonAutocorrelation(A, img_size, &B);

            PeakI = (int *) calloc(leng_B, sizeof(int));
            Peak = (float *) calloc(leng_B, sizeof(float));

            size_peak = CountLocalProjectionPeaks(B, leng_B, PeakI, Peak);

            if (size_peak > 1) {
                /* (KK) Calculate contrast for current projection */
                contrast[i][j] = ComputeProjectionContrast(B, leng_B, PeakI, Peak, size_peak);
            }

            if (size_peak > 1) {
                if (size_peak == 2) {
                    peak_diff = (float) fabs(Peak[0] - Peak[1]);

                    if (peak_diff <= 0.13 && contrast[i][j] > 0.5) {

                        dis_peak = (float) (PeakI[1] - PeakI[0]);
                        var_dis = 0.0;

                        Proj_Candi_Posi[count_candi][0] = i;
                        Proj_Candi_Posi[count_candi][1] = j;
                        Proj_Candi_Valu[count_candi][0] = dis_peak;
                        Proj_Candi_Valu[count_candi][1] = var_dis;

                        Candi_Avail = 1;
                        count_candi++;
                    }
                }
                else {
                    peak_summary(&dis_peak, &var_dis, PeakI, Peak, size_peak);
                    dis_ratio = var_dis / dis_peak;

                    if (dis_ratio < 0.14) {
                        Proj_Candi_Posi[count_candi][0] = i;
                        Proj_Candi_Posi[count_candi][1] = j;
                        Proj_Candi_Valu[count_candi][0] = dis_peak;
                        Proj_Candi_Valu[count_candi][1] = var_dis;

                        Candi_Avail = 1;
                        count_candi++;
                    }
                }
            }

            free(Peak); 
            free(PeakI);
            free(B);
            Candi_Avail = 0;
        }
    }
    free(A);

    flag = 0;

    if (count_candi > 0) {
        refine_cand = (int *) calloc(count_candi, sizeof(int));
        flag = cand_cluster(Proj_Candi_Valu, count_candi, 2, refine_cand, &pick_num);

        picked_cand = (float **) AllocateMatrixFloat(pick_num, 2);
        picked_cand_posi = (int **) AllocateMatrixInteger(pick_num, 2);

        for (i = 0; i < pick_num; i++) {
            picked_cand[i][0] = Proj_Candi_Valu[refine_cand[i]][0];
            picked_cand[i][1] = Proj_Candi_Valu[refine_cand[i]][1];
            picked_cand_posi[i][0] = Proj_Candi_Posi[refine_cand[i]][0];
            picked_cand_posi[i][1] = Proj_Candi_Posi[refine_cand[i]][1];
        }

        max_contrast = -100000.0;

        for (i = 0; i < pick_num; i++) {
            if (max_contrast < contrast[picked_cand_posi[i][0]][picked_cand_posi[i][1]]) {
                max_contrast = contrast[picked_cand_posi[i][0]][picked_cand_posi[i][1]];
                k = i;
            }
        }

        if (proj_type == 1) { // (KK) if projection type was H
            pbc->element[0] = picked_cand_posi[k][0] + 1;
            //pbc->element[1] = picked_cand_posi[k][1]+1;
        }
        else if (proj_type == 2) { // (KK) if projection type was V
            pbc->element[2] = picked_cand_posi[k][0] + 1;
            //pbc->element[3] = picked_cand_posi[k][1]+1;
        }

        proj_credit_modi(picked_cand_posi, pick_num, count_candi, flag, credit);

        FreeMatrixInteger(picked_cand_posi, pick_num);
        FreeMatrixFloat(picked_cand, pick_num);
        free(refine_cand);
    }

    FreeMatrixInteger(Proj_Candi_Posi, 24);
    FreeMatrixFloat(Proj_Candi_Valu, 24);
    FreeMatrixFloat(contrast, 4);
}

int TextureBrowsingExtractor::CountLocalProjectionPeaks(double * B, int leng_B, int * PeakI, float * Peak) {
    int count = 0, i;

    for (i = 1; i < leng_B - 1; i++) {
        if (B[i] > B[i - 1] && B[i] > B[i + 1]) {
            PeakI[count] = i;
            Peak[count] = (float) B[i];
            count++;
        }
    }

    return count;
}

/* ----- OTHER ----- */
void TextureBrowsingExtractor::sort(double * Y, int * I, double * A, int length) {
    int i, j;
    double max, *tmp;

    tmp = (double *) calloc(length, sizeof(double));

    for (i = 0; i < length; i++) {
        tmp[i] = A[i];
    }

    max = tmp[0];
    for (i = 1; i < length; i++) {
        if (tmp[i] > max) {
            max = tmp[i];
        }
    }

    max = fabs(10 * max);

    for (i = 0; i < length; i++) {
        Y[i] = tmp[0];
        I[i] = 0;

        for (j = 1; j < length; j++) {
            if (tmp[j] < Y[i]) {
                Y[i] = tmp[j];
                I[i] = j;
            }
        }

        tmp[I[i]] = max;
    }

    free(tmp);
}

double TextureBrowsingExtractor::ComputeHistogramContrast(int index, double * histo, int len) {
    double slope;
    double temp_slop1, temp_slop2;

    if (index == 0) {
        slope = histo[index] - histo[index + 1];
    }
    else {
        if (index == len - 1) {
            slope = histo[index] - histo[index - 1];
        }
        else {
            temp_slop1 = histo[index] - histo[index - 1];
            temp_slop2 = histo[index] - histo[index + 1];

            slope = (temp_slop1 + temp_slop2) / 2;
        }
    }

    return slope;
}

int TextureBrowsingExtractor::JudgeContinuity(int index, int order, Peak * peak, int size) {
    int status;
    int i;

    status = 0;
    for (i = 0; i < size; i++) {
        if (index == peak[i].index) {
            peak[i].flag = 1;
            status = 1;
            break;
        }
    }
    return status;
}

float TextureBrowsingExtractor::ComputeProjectionContrast(double * B, int leng_B, int * PeakI, float * Peak, int num_peak) {
    /*
    (KK)
    Method calculating contrast for current projection
    (4.3.2.1.4 Peak detection) 
    
    This is note from documentation: 

    For detected peaks and valleys , their position and magnitude are recorded.
    M - number of peaks   ( which is here 'num_peak)
    N - number of valleys ( which is here 'count' )
    
    p_posi and p_magn are positions and magnitudes of peak points (here: only magnitude passed as argument as 'Peak')
    v_posi and v_magn are positions and magnitudes of valley points (here: 'index_valley', 'valley_value') */

    int count, i, j, *index_valley;
    float *valley_value, meanp, meanv;

    count = num_peak - 1;

    // Preapare arrays for valleys indices and values
    index_valley = (int   *) calloc(count, sizeof(int));
    valley_value = (float *) calloc(count, sizeof(float));

    // (KK) Calculate mean of v_magn (second part of equation)
    meanv = 0;
    for (i = 0; i < count; i++) {
        /* (KK) Somehow from projection (which is B I think, and 'PeakI' argument valley data is gathered) */
        for (j = PeakI[i] + 1; j <= PeakI[i + 1] - 1; j++) {
            if (B[j] <= B[j - 1] && B[j] <= B[j + 1]) {
                index_valley[i] = j;
                valley_value[i] = (float) B[j];
                break;
            }
        }
        meanv += valley_value[i];
    }
    meanv /= count;

    // (KK) Calculate mean of p_magn (first part of equation)
    meanp = 0;

    for (i = 0; i < num_peak; i++) {
        meanp += Peak[i];
    }

    meanp /= num_peak;

    // Free memory
    free(index_valley); 
    free(valley_value);

    // (KK) return contrast (mean p_magn - mean v_magn
    return (meanp - meanv); 
}

void TextureBrowsingExtractor::peak_summary(float * dis_peak, float * var_dis, int * PeakI, float * Peak, int size_peak) {
    float *dis_array, dis, sum;
    int len, i;

    len = size_peak - 1;

    dis_array = (float *) calloc(len, sizeof(float));

    dis = 0;

    for (i = 1; i < size_peak; i++) {
        dis_array[i - 1] = (float) (PeakI[i] - PeakI[i - 1]);
        dis += dis_array[i - 1];
    }
    dis /= len;

    sum = 0;

    for (i = 0; i < len; i++) {
        sum += (dis_array[i] - dis) * (dis_array[i] - dis);
    }

    *var_dis = (float) sqrt(sum / len);
    *dis_peak = dis;
    free(dis_array);
}

int TextureBrowsingExtractor::cand_cluster(float ** candi, int size_in, int ndim, int * y, int * pick_num) {
    int flag = 0, *cluster, *count, i, newN, maxcount, domi_num;
    float temp;

    if (size_in <= 2) {
        if (size_in == 1) {
            y[0] = 0;

            if (candi[0][1] > 2) {
                flag = 1;
            }

            *pick_num = 1;
        }
        if (size_in == 2) {
            y[0] = 0;
            y[1] = 1;

            temp = Vector2DVariance(candi, size_in, ndim);

            if (temp > 2) {
                flag = 1;
            }

            *pick_num = 2;
        }

    }
    else {
        cluster = (int *) calloc(size_in, sizeof(int));
        count   = (int *) calloc(size_in, sizeof(int));

        newN = agglom(candi, cluster, ndim, 2.0, count, size_in, 2);

        if (newN == size_in) {
            flag = 1;
        }

        if (newN == size_in) {
            *pick_num = newN;

            for (i = 0; i < newN; i++) {
                y[i] = i;
            }
        }
        else {
            maxcount = 0;

            for (i = 0; i < newN; i++) {
                if (maxcount < count[i]) { 
                    maxcount = count[i]; domi_num = i; 
                }
            }

            *pick_num = 0;

            for (i = 0; i < size_in; i++) {
                if (cluster[i] == domi_num) { 
                    y[(*pick_num)] = i; 
                    (*pick_num)++; 
                }
            }
        }
        free(count); free(cluster);
    }
    return flag;
}

int TextureBrowsingExtractor::agglom(float ** candi, int * label, int N, float thresh, int * count, int Nlabel, int fN) {
    int i, j, newNlabel, mini, minj, total;
    float ** dist, mindist, **labelcf;

    labelcf = (float **) AllocateMatrixFloat(Nlabel, N);

    for (i = 0; i < Nlabel; i++) {
        for (j = 0; j < N; j++) {
            labelcf[i][j] = candi[i][j];
        }
    }

    for (i = 0; i < Nlabel; i++) { 
        label[i] = i; count[i] = 1; 
    }

    dist = (float **) AllocateMatrixFloat(Nlabel, Nlabel);

    mindist = 100000;

    for (i = 0; i < Nlabel - 1; i++) {
        for (j = i + 1; j < Nlabel; j++) {
            dist[i][j] = EuclideanVectorDistance(labelcf[i], labelcf[j], N);
            dist[j][i] = dist[i][j];

            if (dist[i][j] < mindist) { 
                mindist = dist[i][j];
                mini = i;
                minj = j; 
            }
        }
    }

    newNlabel = Nlabel;

    while (mindist < thresh && newNlabel > fN) {
        for (i = 0; i < Nlabel; i++) {

            if (label[i] == minj) {
                label[i] = mini;
            }
            else if (label[i] > minj) {
                label[i]--;
            }
        }

        total = count[mini] + count[minj];

        for (j = 0; j < N; j++) {
            labelcf[mini][j] = (count[mini] * labelcf[mini][j] + count[minj] * labelcf[minj][j]) / total;
        }

        count[mini] = total;

        for (i = minj; i < newNlabel - 1; i++) {
            count[i] = count[i + 1];
            for (j = 0; j < N; j++) {
                labelcf[i][j] = labelcf[i + 1][j];
            }
        }

        for (i = minj; i < newNlabel - 1; i++) {
            for (j = 0; j < minj; j++) {
                dist[i][j] = dist[i + 1][j];
            }
        }

        for (j = minj; j < newNlabel - 1; j++) {
            for (i = 0; i < minj; i++) {
                dist[i][j] = dist[i][j + 1];
            }
        }

        for (i = minj; i < newNlabel - 1; i++) {
            for (j = minj; j < newNlabel - 1; j++) {
                dist[i][j] = dist[i + 1][j + 1];
            }
        }

        newNlabel--;

        for (j = 0; j < newNlabel; j++) {
            dist[mini][j] = EuclideanVectorDistance(labelcf[mini], labelcf[j], N);
            dist[j][mini] = dist[mini][j];
        }

        mindist = 100000;

        for (i = 0; i < newNlabel - 1; i++) {
            for (j = i + 1; j < newNlabel; j++) {
                if (dist[i][j] < mindist) {
                    mindist = dist[i][j]; mini = i; minj = j; 
                }
            }
        }
    }
    FreeMatrixFloat(dist, Nlabel);
    FreeMatrixFloat(labelcf, Nlabel);
    return newNlabel;
}

void TextureBrowsingExtractor::proj_credit_modi(int ** candidate, int size_cand, int size_count_candi, int flag, float * cand_credit) {
    int i, j, **cand_pair, **cand_class_map, **scale_map, **direc_map, num_scales;
    int scale_num[4], direc_num[6], num_direc, **direc_cand, pbc_class[3];
    float value[3];

    for (i = 0; i < 3; i++) {
        cand_credit[i] = 0;
    }

    if (size_cand == 1) {
        cand_credit[0] = (float)0.2;
        cand_credit[1] = (float)0.0;
        cand_credit[2] = (float)0.2;
    }
    else if (size_cand == size_count_candi && flag == 1) {
        cand_credit[2] = (float) (size_cand * 0.2);
    }
    else {

        cand_class_map = (int **) AllocateMatrixInteger(4, 6);
        scale_map      = (int **) AllocateMatrixInteger(4, 6);
        direc_map      = (int **) AllocateMatrixInteger(4, 6);
        cand_pair      = (int **) AllocateMatrixInteger(size_cand, 2);

        for (i = 0; i < size_cand; i++) {
            cand_pair[i][0] = candidate[i][0];
            cand_pair[i][1] = candidate[i][1];
        }

        num_scales = get_num(cand_pair, size_cand, scale_num, 4);

        get_map(cand_pair, size_cand, num_scales, scale_num, 1, scale_map);

        direc_cand = (int **) AllocateMatrixInteger(size_cand, 2);

        convertcand(cand_pair, direc_cand, size_cand);

        num_direc = get_num(direc_cand, size_cand, direc_num, 6);

        get_map(direc_cand, size_cand, num_direc, direc_num, 2, direc_map);

        for (i = 0; i < 4; i++) {
            for (j = 0; j < 6; j++) {
                cand_class_map[i][j] = MIN(scale_map[i][j], direc_map[i][j]);
            }
        }

        value[0] = (float)1.0; 
        value[1] = (float)0.5; 
        value[2] = (float)0.2;

        pbc_class[0] = 0; 
        pbc_class[1] = 0; 
        pbc_class[2] = 0;

        for (i = 0; i < 4; i++) {
            for (j = 0; j < 6; j++) {
                if (cand_class_map[i][j] == 1) {
                    pbc_class[0]++;
                }
                else if (cand_class_map[i][j] == 2) {
                    pbc_class[1]++;
                }
                else if (cand_class_map[i][j] == 3) {
                    pbc_class[2]++;
                }
            }
        }

        for (i = 0; i < 3; i++) {
            cand_credit[2] += pbc_class[i] * value[i];
        }

        FreeMatrixInteger(direc_cand, size_cand);
        FreeMatrixInteger(cand_class_map, 4);
        FreeMatrixInteger(scale_map, 4);
        FreeMatrixInteger(direc_map, 4);
        FreeMatrixInteger(cand_pair, size_cand);
    }
}

int TextureBrowsingExtractor::get_num(int ** cand_pair, int size_cand, int * num, int len) {
    int i, temp_scale, total;

    for (i = 0; i < len; i++) {
        num[i] = 0;
    }

    total = 1;
    temp_scale = cand_pair[0][0];
    num[0] = 1;

    for (i = 1; i < size_cand; i++) {

        if (cand_pair[i][0] != temp_scale) {
            total++;
            temp_scale = cand_pair[i][0];
        }

        num[total - 1]++;
    }

    return total;
}

void TextureBrowsingExtractor::get_map(int ** cand_pair, int size_cand, int total, int * num, int type, int ** map) {
    int start, i, j, **in_pair, *sing;

    start = 0;
    for (i = 0; i < total; i++) {
        in_pair = (int **) AllocateMatrixInteger(num[i], sizeof(int));

        for (j = start; j < start + num[i]; j++) {
            in_pair[j - start][0] = cand_pair[j][0];
            in_pair[j - start][1] = cand_pair[j][1];
        }

        sing = (int *) calloc(num[i], sizeof(int));
        sing_map(in_pair, num[i], sing, type);

        if (type == 1) {
            for (j = 0; j < num[i]; j++) {
                map[in_pair[j][0]][in_pair[j][1]] = sing[j];
            }
        }
        else if (type == 2) {
            for (j = 0; j < num[i]; j++) {
                map[in_pair[j][1]][in_pair[j][0]] = sing[j];
            }
        }

        free(sing);
        FreeMatrixInteger(in_pair, num[i]);
        start += num[i];
    }
}

void TextureBrowsingExtractor::convertcand(int ** cand_pair, int ** direc_cand, int size_cand) {
    int i, *index, *tmpcand;

    tmpcand = (int *) calloc(size_cand, sizeof(int));
    index   = (int *) calloc(size_cand, sizeof(int));

    for (i = 0; i < size_cand; i++) {
        tmpcand[i] = cand_pair[i][1];
    }

    piksrtintS2B(size_cand, tmpcand, index);

    for (i = 0; i < size_cand; i++) {
        direc_cand[i][0] = cand_pair[index[i]][1];
        direc_cand[i][1] = cand_pair[index[i]][0];
    }

    free(index); free(tmpcand);
}

void TextureBrowsingExtractor::piksrtintS2B(int n, int * num, int * index) {
    int i, j;
    int indextmp;
    int numtmp;

    for (i = 0; i < n; i++) {
        index[i] = i;
    }

    for (j = 1; j < n; j++) {
        numtmp   = num[j];
        indextmp = index[j];

        i = j - 1;

        while (i >= 0 && num[i]>numtmp) {
            num[i + 1]   = num[i];
            index[i + 1] = index[i];
            i--;
        }

        num[i + 1] = numtmp;
        index[i + 1] = indextmp;
    }
}

void TextureBrowsingExtractor::sing_map(int ** inpair, int size_in, int * sing, int type) {
    int i;

    if (size_in == 1) {
        sing[0] = 3;
    }
    else {
        if (type == 1 && inpair[0][1] == 0 && inpair[size_in - 1][1] == 5) {
            sing[0] = 1; 
            sing[size_in - 1] = 1;

            if (size_in > 2) {
                for (i = 1; i < size_in - 1; i++) {
                    if (inpair[i][1] == inpair[i - 1][1] + 1 || inpair[i][1] == inpair[i + 1][1] - 1) {
                        sing[i] = 1;
                    }
                    else {
                        sing[i] = 2;
                    }
                }
            }
        }
        else {
            if (size_in == 2) {
                if (inpair[0][1] == inpair[1][1] - 1) { 
                    sing[0] = 1; sing[1] = 1; 
                }
                else { 
                    sing[0] = 2; sing[1] = 2; 
                }
            }
            else {
                for (i = 1; i < size_in - 1; i++) {
                    if (inpair[i][1] == inpair[i - 1][1] + 1 || inpair[i][1] == inpair[i + 1][1] - 1) {
                        sing[i] = 1;
                    }
                    else {
                        sing[i] = 2;
                    }
                }

                if (inpair[0][1] == inpair[1][1] - 1) {
                    sing[0] = 1;
                }
                else {
                    sing[0] = 2;
                }

                if (inpair[size_in - 1][1] == inpair[size_in - 2][1] + 1) {
                    sing[size_in - 1] = 1;
                }
                else {
                    sing[size_in - 1] = 2;
                }
            }
        }
    }
}

void TextureBrowsingExtractor::four1(double * data, int nn, int isign) {
    int n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;

    n = nn << 1;
    j = 1;

    for (i = 1; i < n; i += 2) {
        if (j > i) {
            #define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr
                SWAP(data[j], data[i]);
                SWAP(data[j + 1], data[i + 1]);
            #undef SWAP
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

        theta = 6.28318530717959 / (isign*mmax);

        wtemp = sin(0.5 * theta);
        wpr   = -2.0 * wtemp * wtemp;
        wpi   = sin(theta);
        wr    = 1.0;
        wi    = 0.0;

        for (m = 1; m < mmax; m += 2) {
            for (i = m; i <= n; i += istep) {
                j = i + mmax;

                tempr = wr * data[j]     - wi * data[j + 1];
                tempi = wr * data[j + 1] + wi * data[j];

                data[j]     = data[i]     - tempr;
                data[j + 1] = data[i + 1] - tempi;

                data[i]     += tempr;
                data[i + 1] += tempi;
            }
            wr = (wtemp = wr) * wpr - wi * wpi + wr;
            wi = wi * wpr + wtemp * wpi + wi;
        }
        mmax = istep;
    }
}

int TextureBrowsingExtractor::billinear(Matrix * img, float a, float b, int ii, int jj) {
    double y;

    if ((ii < 0) || (ii >= img->height - 1)) {
        return 255;
    }

    if ((jj < 0) || (jj >= img->width - 1)) {
        return 255;
    }

    if ((a == 0.0) && (b == 0.0)) {
        return (int) img->data[ii][jj];
    }

    if (a == 0.0) {
        y = (1 - b) * img->data[ii][jj] + b * img->data[ii][jj + 1];
        return (int) (y + 0.5);
    }

    if (b == 0.0) {
        y = (1 - a) * img->data[ii][jj] + a * img->data[ii + 1][jj];
        return (int) (y + 0.5);
    }

    y = (1 - a) * (1 - b) * img->data[ii][jj]     + (1 - a) * b * img->data[ii][jj + 1] +
             a  * (1 - b) * img->data[ii + 1][jj] + a       * b * img->data[ii + 1][jj + 1];

    return (int) (y + 0.5);
}

void TextureBrowsingExtractor::threshold(pbc_struct * pbc) {
    pbc->tstructuredness = 0;

    if ((pbc->structuredness > 0) && (pbc->structuredness <= BOUNDARY1)) {
        pbc->tstructuredness = 1;
    }

    else if ((pbc->structuredness > BOUNDARY1) && (pbc->structuredness <= BOUNDARY2)) {
        pbc->tstructuredness = 2;
    }

    else if ((pbc->structuredness > BOUNDARY2) && (pbc->structuredness <= BOUNDARY3)) {
        pbc->tstructuredness = 3;
    }

    else if (pbc->structuredness > BOUNDARY3) {
        pbc->tstructuredness = 4;
    }
}

/* ----- ALLOCATING AND FREEING METHODS FOR C ARRAYS ----- */
int ** TextureBrowsingExtractor::AllocateMatrixInteger(int nr, int nc) {
    int i, j;
    int **m;

    m = (int **) malloc(nr*sizeof(int *));
    if (!m) {
        return NULL;
    }

    for (i = 0; i < nr; i++) {
        m[i] = (int *) malloc(nc * sizeof(int));

        if (!m[i]) {
            return NULL;
        }

        for (j = 0; j < nc; j++) {
            m[i][j] = 0;
        }
    }

    return m;
}

float ** TextureBrowsingExtractor::AllocateMatrixFloat(int nr, int nc) {
    int i, j;
    float **m;

    m = (float **) malloc(nr * sizeof(float *));

    if (!m) {
        return NULL;
    }

    for (i = 0; i < nr; i++) {

        m[i] = (float *) malloc(nc * sizeof(float));

        if (!m[i]) {
            return NULL;
        }

        for (j = 0; j < nc; j++) {
            m[i][j] = 0;
        }
    }
    return m;
}

double ** TextureBrowsingExtractor::AllocateMatrixDouble(int nrl, int nrh, int ncl, int nch) {
    int i;
    double **m;

    m = (double **) calloc((unsigned) (nrh - nrl + 1), sizeof(double*));

    m -= nrl;

    for (i = nrl; i <= nrh; i++) {
        m[i] = (double *) calloc((unsigned) (nch - ncl + 1), sizeof(double));
        m[i] -= ncl;
    }
    return m;
}

double * TextureBrowsingExtractor::AllocateVectorOfDouble(int nl, int nh) {
    double * v;

    v = (double *) calloc((unsigned) (nh - nl + 1), sizeof(double));

    return v - nl;
}

void TextureBrowsingExtractor::FreeMatrixInteger(int ** m, int nr) {
    int i;

    for (i = 0; i < nr; i++) {
        free(m[i]);
    }

    free(m);
}

void TextureBrowsingExtractor::FreeMatrixFloat(float ** m, int nr) {
    int i;

    for (i = 0; i < nr; i++) {
        free(m[i]);
    }

    free(m);
}

void TextureBrowsingExtractor::FreeMatrixOfDouble(double ** m, int nrl, int nrh, int ncl, int nch) {
    int i;

    for (i = nrh; i >= nrl; i--) {
        free((char*) (m[i] + ncl));
    }

    free((char*) (m + nrl));
}

void TextureBrowsingExtractor::FreeVectorOfDouble(double * v, int nl, int nh) {
    free((char*) (v + nl));
}

/* ----- BASIC OPERATIONS ON C ARRAYS ----- */
float TextureBrowsingExtractor::EuclideanVectorDistance(float * a, float * b, int dim) {
    int i;
    float dist = 0.0;

    for (i = 0; i < dim; i++) {
        dist += sqr(a[i] - b[i]);
    }

    dist = (float) sqrt(dist);
    return dist;
}

float TextureBrowsingExtractor::Vector2DVariance(float ** vector, int nvec, int ndim) {
    float *mean, var;
    int i, j;

    // (KK) Count mean value
    mean = (float *) calloc(ndim, sizeof(float));

    for (i = 0; i < nvec; i++) {
        for (j = 0; j < ndim; j++) {
            mean[j] += vector[i][j];
        }
    }

    for (j = 0; j < ndim; j++) {
        mean[j] /= nvec;
    }

    // (KK) Count variance
    var = 0;

    for (i = 0; i < nvec; i++) {
        for (j = 0; j < ndim; j++) {
            var += (vector[i][j] - mean[j]) * (vector[i][j] - mean[j]);
        }
    }

    var = (float) sqrt(var / nvec);

    free(mean);
    return var;
}

void TextureBrowsingExtractor::FourierTransform2D(double ** fftr, double ** ffti, double ** rdata, double ** idata, int rs, int cs, int isign) {
    /************************************************************
    2-D fourier transform of data with real part stored in
    "rdata" and imaginary part in "idata" with size "rs" x
    "cs". The result is in "fftr" and "ffti". The isign is
    "isign" =  1 forward, and "isign" = -1 inverse */

    double **T, *tmp1, *tmp2;
    int i, j;

    tmp1 = AllocateVectorOfDouble(1, 2 * cs);
    tmp2 = AllocateVectorOfDouble(1, 2 * rs);
    T = AllocateMatrixDouble(1, 2 * rs, 1, cs);

    for (i = 1; i <= rs; i++) {
        for (j = 1; j <= cs; j++) {
            tmp1[j * 2 - 1] = rdata[i][j];
            tmp1[j * 2] = idata[i][j];
        }

        four1(tmp1, cs, isign);

        for (j = 1; j <= cs; j++) {
            T[i * 2 - 1][j] = tmp1[j * 2 - 1];
            T[i * 2][j] = tmp1[j * 2];
        }
    }

    for (i = 1; i <= cs; i++) {
        for (j = 1; j <= rs; j++) {
            tmp2[j * 2 - 1] = T[j * 2 - 1][i];
            tmp2[j * 2] = T[j * 2][i];
        }

        four1(tmp2, rs, isign);

        for (j = 1; j <= rs; j++) {
            fftr[j][i] = tmp2[j * 2 - 1];
            ffti[j][i] = tmp2[j * 2];
        }
    }
    FreeVectorOfDouble(tmp1, 1, 2 * cs);
    FreeVectorOfDouble(tmp2, 1, 2 * rs);
    FreeMatrixOfDouble(T, 1, 2 * rs, 1, cs);
}

/* ----- CUSTOM MATRIX SECTION ----- */
void TextureBrowsingExtractor::CreateMatrix(Matrix ** M, int hei, int wid) {
    int h;

    Matrix * tmp;

    tmp = (Matrix *) calloc(1, sizeof(Matrix));

    tmp->data = (double **) calloc(hei, sizeof(double *));

    if (!(tmp->data)) {
        // Cleanup
        if (tmp) {
            delete tmp;
        }
        throw TEXT_BROWS_ALLOCATION_MATRIX_ERROR;
    }

    for (h = 0; h < hei; h++) {
        tmp->data[h] = (double *) calloc(wid, sizeof(double));

        if (!(tmp->data[h])) {
            // Cleanup
            if (tmp) {
                delete tmp;
            }
            throw TEXT_BROWS_ALLOCATION_MATRIX_ERROR;
        }
    }

    tmp->height = hei;
    tmp->width = wid;

    *M = tmp;
}

void TextureBrowsingExtractor::Convert2Matrix(unsigned char * R, int width, int height, Matrix * image) {
    int i, j;
    int count;

    count = 0;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            image->data[i][j] = R[count];
            count++;
        }
    }
}

void TextureBrowsingExtractor::Mat_Copy(Matrix * A, Matrix * B, int h_target, int w_target, int h_begin, int w_begin, int h_end, int w_end) {
    int i, j, h, w, h_done, w_done;

    if ((h_target >= 0) && (h_target < A->height) && (w_target >= 0) && (w_target < A->width)) {
        if ((h_begin >= 0) && (h_begin < B->height) && (w_begin >= 0) && (w_begin < B->width)) {

            h = h_end - h_begin + 1;
            w = w_end - w_begin + 1;

            if ((h >= 1) && (w >= 1)) {

                h_done = h_target + h - 1;
                w_done = w_target + w - 1;

                if ((h_done < A->height) && (w_done < A->width)) {
                    for (i = 0; i<h; i++) {
                        for (j = 0; j<w; j++) {
                            A->data[i + h_target][j + w_target] = B->data[i + h_begin][j + w_begin];
                        }
                    }
                }
            }
        }
    }
}

void TextureBrowsingExtractor::Mat_Product(Matrix * A, Matrix * B, Matrix * C) {
    int h, w;

    for (h = 0; h<A->height; h++) {
        for (w = 0; w<A->width; w++) {
            A->data[h][w] = B->data[h][w] * C->data[h][w];
        }
    }
}

void TextureBrowsingExtractor::Mat_Substract(Matrix * A, Matrix * B, Matrix * C) {
    int h, w;

    for (h = 0; h<A->height; h++) {
        for (w = 0; w<A->width; w++) {
            A->data[h][w] = B->data[h][w] - C->data[h][w];
        }
    }
}

void TextureBrowsingExtractor::Mat_Sum(Matrix * A, Matrix * B, Matrix * C) {
    int h, w;

    for (h = 0; h<A->height; h++) {
        for (w = 0; w<A->width; w++) {
            A->data[h][w] = B->data[h][w] + C->data[h][w];
        }
    }
}

void TextureBrowsingExtractor::Mat_FFT2(Matrix * Output_real, Matrix * Output_imag, Matrix * Input_real, Matrix * Input_imag) {
    int xs, ys, i, j;
    double **R, **I, **Fr, **Fi;

    xs = Input_real->height;
    ys = Input_real->width;

    R = AllocateMatrixDouble(1, xs, 1, ys);
    I = AllocateMatrixDouble(1, xs, 1, ys);
    Fr = AllocateMatrixDouble(1, xs, 1, ys);
    Fi = AllocateMatrixDouble(1, xs, 1, ys);

    for (i = 1; i <= Input_real->height; i++) {
        for (j = 1; j <= Input_real->width; j++) {
            R[i][j] = Input_real->data[i - 1][j - 1];
            I[i][j] = Input_imag->data[i - 1][j - 1];
        }
    }

    FourierTransform2D(Fr, Fi, R, I, xs, ys, 1); /* 2-D FFT */

    for (i = 1; i <= Input_real->height; i++) {
        for (j = 1; j <= Input_real->width; j++) {
            Output_real->data[i - 1][j - 1] = Fr[i][j];
            Output_imag->data[i - 1][j - 1] = Fi[i][j];
        }
    }

    FreeMatrixOfDouble(R, 1, xs, 1, ys);
    FreeMatrixOfDouble(I, 1, xs, 1, ys);
    FreeMatrixOfDouble(Fr, 1, xs, 1, ys);
    FreeMatrixOfDouble(Fi, 1, xs, 1, ys);
}

void TextureBrowsingExtractor::Mat_IFFT2(Matrix * Output_real, Matrix * Output_imag, Matrix * Input_real, Matrix * Input_imag) {
    int xs, ys, i, j;
    double **R, **I, **Fr, **Fi, NN;

    xs = Input_real->height;
    ys = Input_real->width;

    R = AllocateMatrixDouble(1, xs, 1, ys);
    I = AllocateMatrixDouble(1, xs, 1, ys);
    Fr = AllocateMatrixDouble(1, xs, 1, ys);
    Fi = AllocateMatrixDouble(1, xs, 1, ys);

    for (i = 1; i <= Input_real->height; i++) {
        for (j = 1; j <= Input_real->width; j++) {
            R[i][j] = Input_real->data[i - 1][j - 1];
            I[i][j] = Input_imag->data[i - 1][j - 1];
        }
    }

    FourierTransform2D(Fr, Fi, R, I, xs, ys, -1);         /* 2-D IFFT */

    NN = (double) (xs * ys);

    for (i = 1; i <= Input_real->height; i++) {
        for (j = 1; j <= Input_real->width; j++) {
            Output_real->data[i - 1][j - 1] = Fr[i][j] / NN;
            Output_imag->data[i - 1][j - 1] = Fi[i][j] / NN;
        }
    }

    FreeMatrixOfDouble(R, 1, xs, 1, ys);
    FreeMatrixOfDouble(I, 1, xs, 1, ys);
    FreeMatrixOfDouble(Fr, 1, xs, 1, ys);
    FreeMatrixOfDouble(Fi, 1, xs, 1, ys);
}

void TextureBrowsingExtractor::FreeMatrix(Matrix * M) {
    int h;

    for (h = 0; h < M->height; h++) {
        free(M->data[h]);
    }
    free(M->data);

    free(M);
}

/* ----- DESTRUCTOR ----- */
TextureBrowsingExtractor::~TextureBrowsingExtractor() {
    delete descriptor;
}