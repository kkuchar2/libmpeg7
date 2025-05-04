#include "DominantColorExtractor.h"

DominantColorExtractor::DominantColorExtractor(): currentColorNumber(0) {
    descriptor = new DominantColor();
}

Descriptor * DominantColorExtractor::extract(Image & image, const char ** params) {
    // Load parameters
    try {
        descriptor->loadParameters(params);
    }
    catch (ErrorCode exception) {
        throw exception;
    }

    // Get image information
    const int imageWidth     = image.getWidth();
    const int imageHeight    = image.getHeight();
    const int imageSize      = image.getSize();
    const int imageTotalSize = image.getTotalSize();
    const bool transparencyPresent = image.getTransparencyPresent();

    const float agglomeratingFactor = static_cast<float>(DSTMIN);
    const float splittingFactor     = static_cast<float>(SPLITTING_FACTOR);

    dominantColorWeights    = new float[DESCRIPTOR_SIZE];
    dominantColorCentroids  = new float * [DESCRIPTOR_SIZE];
    dominantColorsVariances = new float * [DESCRIPTOR_SIZE];

    for (int k = 0; k < DESCRIPTOR_SIZE; k++) {
        dominantColorCentroids[k] = new float[3];
        dominantColorsVariances[k] = new float[3];

        dominantColorWeights[k] = 0.0;

        dominantColorCentroids[k][0] = 0.0;
        dominantColorCentroids[k][1] = 0.0;
        dominantColorCentroids[k][2] = 0.0;

        dominantColorsVariances[k][0] = 0.0;
        dominantColorsVariances[k][1] = 0.0;
        dominantColorsVariances[k][2] = 0.0;
    }

    // Get image data and convert RGB to LUV
    const unsigned char * RGB = image.getRGB();
    LUV = new float[imageTotalSize]; 
    rgb2luv(image, LUV);
    delete[] RGB;
    //rgb2luv(image, LUV);

    unsigned char * alphaChannelBuffer = transparencyPresent ? image.getChannel_A() : nullptr;

    // Apply GLA algorithm and split color bins
    const auto pixelsClusters = new int[imageSize];
    double totalDistortion = FLT_MAX;
    double newDistortion;
    double distortionChange = 1.0;

    currentColorNumber = 1;

    // Assign pixels to clusters and calulate init distortion
    newDistortion = AssignPixelsToClusters(pixelsClusters, LUV, imageSize, alphaChannelBuffer);

    double dist;
    double tmp;

    int i = 0;
    while (distortionChange > MINIMUM_DISTORTION_CHANGE) {
        // Assign each color to its cluster - recalculate clusters colors as average of colors of pixels assigned to them
        RecalculateCentroids(pixelsClusters, LUV, imageSize, alphaChannelBuffer);

        // Assign pixels to clusters and calculate squareOfL2Norm as new distortion
        newDistortion = AssignPixelsToClusters(pixelsClusters, LUV, imageSize, alphaChannelBuffer);

        // Check distortion change
        if (totalDistortion > 0.0) {
            distortionChange = (totalDistortion - newDistortion) / totalDistortion;
        }
        else {
            distortionChange = 0.0;
        }
        totalDistortion = newDistortion;

        // Split color clusters
        if (i == 0 || (distortionChange < SPLIT_MINIMUM_DISTORTION_CHANGE && currentColorNumber < DESCRIPTOR_SIZE)) {
            Split(pixelsClusters, LUV, imageSize, alphaChannelBuffer, splittingFactor);
            newDistortion = AssignPixelsToClusters(pixelsClusters, LUV, imageSize, alphaChannelBuffer);
            distortionChange = 1.0;
        }

        // Check for identical codevectors
        for (int j = 0; j < currentColorNumber; j++) {
            for (int k = 0; k < j; k++) {
                dist = 0.0;
                tmp = dominantColorCentroids[j][0] - dominantColorCentroids[k][0];
                dist += tmp * tmp;
                tmp = dominantColorCentroids[j][1] - dominantColorCentroids[k][1];
                dist += tmp * tmp;
                tmp = dominantColorCentroids[j][2] - dominantColorCentroids[k][2];
                dist += tmp * tmp;
            }
        }
        i++;
    }

    // Merge using agglomerative clustering method
    Agglom(agglomeratingFactor);

    // Calculate variances
    newDistortion = AssignPixelsToClusters(pixelsClusters, LUV, imageSize, alphaChannelBuffer);
    RecalculateCentroids(pixelsClusters, LUV, imageSize, alphaChannelBuffer);
    newDistortion = AssignPixelsToClusters(pixelsClusters, LUV, imageSize, alphaChannelBuffer);

    if (descriptor->getVariancePresent()) {
        CalculateVariances(pixelsClusters, LUV, imageSize, alphaChannelBuffer);
    }

    delete pixelsClusters;

    // Normalize
    for (int j = 0; j < currentColorNumber; j++) {
        dominantColorWeights[j] /= imageSize;
    }

    // Initialize result variables and arrays
    descriptor->allocateResultArrays(currentColorNumber);

    // Get pointers to descriptor results memory:
    int ** resultDominantColors = descriptor->getResultDominantColors();
    int ** resultColorVariances = descriptor->getResultColorVariances();
    int *  resultPercentages    = descriptor->getResultPercentages();

    // Set result dominant color values (first without quantizing) and percentages 
    for (i = 0; i < currentColorNumber; i++) {
        luv2rgb(resultDominantColors[i], dominantColorCentroids[i], 3);   // set dominant color values (and convert back LUV to RGB)
        resultPercentages[i] = static_cast<int>(31.9999 * dominantColorWeights[i]);  // set dominant color percentages
    }

    // Set result dominant color variances
    for (i = 0; i < currentColorNumber; i++) {
        for (int j = 0; j < 3; j++) {
            resultColorVariances[i][j] = (dominantColorsVariances[i][j] > VARTHR) ? 1 : 0;
        }
    }

    // Calculate spatial coherency
    if (descriptor->getSpatialCoherencyPresent()) {
        descriptor->setSpatialCoherencyValue(static_cast<float>(GetSpatialCoherency(LUV, 3, currentColorNumber, dominantColorCentroids, alphaChannelBuffer, imageWidth, imageHeight)));
    }
    else {
        descriptor->setSpatialCoherencyValue(0);
    }

    delete[] LUV;

    descriptor->setResultDescriptorSize(currentColorNumber);

    return descriptor;
}

double DominantColorExtractor::AssignPixelsToClusters(int * pixelsClusters, float * imageData, int imageSize, unsigned char * alphaChannelBuffer) {
    /* Assign each pixel to it's cluster ISO/IEC 15938-8 4.2.3.1, 267  */
    int nearestClusterIndex;			   // index of cluster centroid nearest to pixel
    double currentDistance;				   // current distance
    double minimumDistance;				   // minimum distance for current pixel (distance to nearest cluster centroid)

    double sumOfMinimumDistances = 0.0;    // Sum of minimum distances

    int currentPixelIndex;
    int currentColorCentroid;

    float *im1, *im2, *im3;

    unsigned char * pAlpha;

    int notTransparentPixels = 0;          // number of not transparent pixels

    double d1, d2, d3;

    for (currentPixelIndex = 0, im1 = imageData, im2 = imageData + 1, im3 = imageData + 2; currentPixelIndex < imageSize;
    currentPixelIndex++, im1 += 3, im2 += 3, im3 += 3) {

        pAlpha = &alphaChannelBuffer[currentPixelIndex];

        if (!alphaChannelBuffer || *pAlpha) { // if that pixel is not transparent 
            nearestClusterIndex = 0;		// set closest cluster index to 0
            minimumDistance = FLT_MAX;

            // Iterates over all current color clusters centroids and decides which one is the closest one (by Euclidean distance)
            for (currentColorCentroid = 0; currentColorCentroid < currentColorNumber; currentColorCentroid++) {
                d1 = *im1 - dominantColorCentroids[currentColorCentroid][0];
                d2 = *im2 - dominantColorCentroids[currentColorCentroid][1];
                d3 = *im3 - dominantColorCentroids[currentColorCentroid][2];

                currentDistance = d1 * d1 + d2 * d2 + d3 * d3; // Square of L2 norm

                if (currentDistance < minimumDistance) {
                    nearestClusterIndex = currentColorCentroid;
                    minimumDistance = currentDistance;
                }
            }
            pixelsClusters[currentPixelIndex] = nearestClusterIndex;
            sumOfMinimumDistances += minimumDistance;
            notTransparentPixels++;
        }
    }

    // Total distortion is: sum of smallest distances / sum of not transparent pixels, so it is average minimum distance
    return  sumOfMinimumDistances / notTransparentPixels;
}

void DominantColorExtractor::RecalculateCentroids(int * pixelsClusters, float * imageData, int imageSize, unsigned char * alphaChannelBuffer) {
    /* Calculate new color cluster centroids - as average of pixels assigned for them */
    int currentPixelIndex;
    int currentColorCentroid;

    // Reset weights and centroids:
    for (currentColorCentroid = 0; currentColorCentroid < currentColorNumber; currentColorCentroid++) {
        dominantColorWeights[currentColorCentroid] = 0.0;
        dominantColorCentroids[currentColorCentroid][0] = 0.0;
        dominantColorCentroids[currentColorCentroid][1] = 0.0;
        dominantColorCentroids[currentColorCentroid][2] = 0.0;
    }

    float * im1, *im2, *im3;
    unsigned char * pAlpha;

    // Calculate new centroids:
    for (currentPixelIndex = 0, im1 = imageData, im2 = imageData + 1, im3 = imageData + 2; currentPixelIndex < imageSize;
         currentPixelIndex++, im1 += 3, im2 += 3, im3 += 3) {

        pAlpha = &alphaChannelBuffer[currentPixelIndex];

        if (!alphaChannelBuffer || *pAlpha) {
            const int nearestColorCluster = pixelsClusters[currentPixelIndex]; // Get nearest color cluster for current pixel

            // Each centroid gets weight as number of pixels assigned to it:
            dominantColorWeights[nearestColorCluster]++;

            // Each centroid gets sum of colors of pixel assigned to it:
            dominantColorCentroids[nearestColorCluster][0] += *im1;
            dominantColorCentroids[nearestColorCluster][1] += *im2;
            dominantColorCentroids[nearestColorCluster][2] += *im3;
        }
    }

    double weight;

    for (currentColorCentroid = 0; currentColorCentroid < currentColorNumber; currentColorCentroid++) {
        weight = dominantColorWeights[currentColorCentroid]; // get weight for current centroid
        if (weight != 0.0) {
            // New centroid is: (sum of colors of pixels previously assigned to it) / (number of those pixels)
            // This is basically average of colors of those pixels:
            dominantColorCentroids[currentColorCentroid][0] /= static_cast<float>(weight);
            dominantColorCentroids[currentColorCentroid][1] /= static_cast<float>(weight);
            dominantColorCentroids[currentColorCentroid][2] /= static_cast<float>(weight);
        }
        else {
            std::cout << "Zero weight for colour " << currentColorCentroid << std::endl;
        }
    }
}

void DominantColorExtractor::CalculateVariances(int * pixelsClusters, float * imageData, int imageSize, unsigned char * alphaChannelBuffer) {
    int i, j;
    double tmp;
    unsigned char * pAlpha;

    // Reset variances
    for (i = 0; i < currentColorNumber; i++) {
        dominantColorsVariances[i][0] = 0.0;
        dominantColorsVariances[i][1] = 0.0;
        dominantColorsVariances[i][2] = 0.0;
    }

    // Estimate variances
    for (i = 0; i < imageSize; i++) {
        pAlpha = &alphaChannelBuffer[i];

        if (!alphaChannelBuffer || *pAlpha) {
            j = pixelsClusters[i];

            tmp = imageData[3 * i] - dominantColorCentroids[j][0];
            dominantColorsVariances[j][0] += static_cast<float>(tmp * tmp);

            tmp = imageData[3 * i + 1] - dominantColorCentroids[j][1];
            dominantColorsVariances[j][1] += static_cast<float>(tmp * tmp);

            tmp = imageData[3 * i + 2] - dominantColorCentroids[j][2];
            dominantColorsVariances[j][2] += static_cast<float>(tmp * tmp);
        }
    }

    // Normalize
    for (j = 0; j < currentColorNumber; j++) {
        dominantColorsVariances[j][0] /= dominantColorWeights[j];
        dominantColorsVariances[j][1] /= dominantColorWeights[j];
        dominantColorsVariances[j][2] /= dominantColorWeights[j];
    }
}

void DominantColorExtractor::Split(int * pixelsClusters, float *imageData, int imageSize, unsigned char * alphaChannelBuffer, double factor) {
    /*  Splitting color clusters (KK)
    NewcolorBin1 = OldcolorBin + PerturbanceVector;
    NewcolorBin2 = OldcolorBin - PerturbanceVector; */

    // Variance components 
    double d1s[DESCRIPTOR_SIZE], d2s[DESCRIPTOR_SIZE], d3s[DESCRIPTOR_SIZE];

    // Total distortion
    double dists[DESCRIPTOR_SIZE];

    double distmax = 0.0;
    int i, j;

    // Reset distortions 
    for (j = 0; j < currentColorNumber; j++) {
        d1s[j] = 0.0;
        d2s[j] = 0.0;
        d3s[j] = 0.0;
        dists[j] = 0.0;
    }

    int jmax = 0;
    unsigned char * pAlpha;

    double d1, d2, d3;

    // Calculate local distortions - how much current pixel in loop is different from its nearest assigned cluster
    for (i = 0; i < imageSize; i++) {
        pAlpha = &alphaChannelBuffer[i];

        if (!alphaChannelBuffer || *pAlpha) {
            j = pixelsClusters[i];

            d1 = imageData[3 * i] - dominantColorCentroids[j][0];
            d2 = imageData[3 * i + 1] - dominantColorCentroids[j][1];
            d3 = imageData[3 * i + 2] - dominantColorCentroids[j][2];

            d1s[j] += d1 * d1;
            d2s[j] += d2 * d2;
            d3s[j] += d3 * d3;
        }
    }

    // Calculate total distortion for each cluster (sum of local distortions) and normalize to weights
    for (j = 0; j < currentColorNumber; j++) {
        dists[j] = d1s[j] + d2s[j] + d3s[j];

        d1s[j] /= dominantColorWeights[j];
        d2s[j] /= dominantColorWeights[j];
        d3s[j] /= dominantColorWeights[j];
        // As result we have average total distortion for each cluster
    }

    // Find cluster with highest average total distortion
    for (j = 0; j < currentColorNumber; j++) {
        if (dists[j] > distmax) {
            jmax = j;
            distmax = dists[j];
        }
    }

    // Split cluster with highest distortion;
    const double perturbanceVector[3] {
        factor * sqrt(d1s[jmax]),
        factor * sqrt(d2s[jmax]),
        factor * sqrt(d3s[jmax])
    };

    // Split one color bin to two color bins (old one is discarded)
    dominantColorCentroids[currentColorNumber][0] = dominantColorCentroids[jmax][0] + static_cast<float>(perturbanceVector[0]);
    dominantColorCentroids[currentColorNumber][1] = dominantColorCentroids[jmax][1] + static_cast<float>(perturbanceVector[1]);
    dominantColorCentroids[currentColorNumber][2] = dominantColorCentroids[jmax][2] + static_cast<float>(perturbanceVector[2]);

    dominantColorCentroids[jmax][0] = dominantColorCentroids[jmax][0] - static_cast<float>(perturbanceVector[0]);
    dominantColorCentroids[jmax][1] = dominantColorCentroids[jmax][1] - static_cast<float>(perturbanceVector[1]);
    dominantColorCentroids[jmax][2] = dominantColorCentroids[jmax][2] - static_cast<float>(perturbanceVector[2]);
    currentColorNumber++;
}

void DominantColorExtractor::Agglom(double distthr) {
    double d1, d2, d3;
    double dists[DESCRIPTOR_SIZE][DESCRIPTOR_SIZE];
    double distmin = 0.0;

    double w1min, w2min;

    int ja, jb = 0;
    int jamin, jbmin;

    /* While two pixelsClusters colours are closer than DISTTHR,
    merge the pixelsClusters pair */

    do {
        // Initialize distance table
        for (ja = 0; ja < currentColorNumber; ja++) {
            for (jb = 0; jb < ja; jb++) {
                d1 = dominantColorCentroids[ja][0] - dominantColorCentroids[jb][0];
                d2 = dominantColorCentroids[ja][1] - dominantColorCentroids[jb][1];
                d3 = dominantColorCentroids[ja][2] - dominantColorCentroids[jb][2];

                dists[ja][jb] = d1 * d1 + d2 * d2 + d3 * d3;
            }
        }

        // Find two pixelsClusters colours
        distmin = FLT_MAX;
        jamin = 0;
        jbmin = 0;

        for (ja = 0; ja < currentColorNumber; ja++) {
            for (jb = 0; jb < ja; jb++) {
                if (dists[ja][jb] < distmin) {
                    distmin = dists[ja][jb];
                    jamin = ja;
                    jbmin = jb;
                }
            }
        }

        if (distmin > distthr) {
            break;
        }

        // Merge two pixelsClusters colours
        w1min = dominantColorWeights[jamin];
        w2min = dominantColorWeights[jbmin];

        dominantColorCentroids[jbmin][0] = static_cast<float>((w1min * dominantColorCentroids[jamin][0] + w2min * dominantColorCentroids[jbmin][0]) / (w1min + w2min));
        dominantColorCentroids[jbmin][1] = static_cast<float>((w1min * dominantColorCentroids[jamin][1] + w2min * dominantColorCentroids[jbmin][1]) / (w1min + w2min));
        dominantColorCentroids[jbmin][2] = static_cast<float>((w1min * dominantColorCentroids[jamin][2] + w2min * dominantColorCentroids[jbmin][2]) / (w1min + w2min));

        dominantColorWeights[jbmin] += static_cast<float>(w1min);

        currentColorNumber--;

        // Remove jamin
        for (jb = jamin; jb < currentColorNumber; jb++) {
            dominantColorWeights[jb] = dominantColorWeights[jb + 1];

            dominantColorCentroids[jb][0] = dominantColorCentroids[jb + 1][0];
            dominantColorCentroids[jb][1] = dominantColorCentroids[jb + 1][1];
            dominantColorCentroids[jb][2] = dominantColorCentroids[jb + 1][2];

            dominantColorsVariances[jb][0] = dominantColorsVariances[jb + 1][0];
            dominantColorsVariances[jb][1] = dominantColorsVariances[jb + 1][1];
            dominantColorsVariances[jb][2] = dominantColorsVariances[jb + 1][2];
        }

    } while (currentColorNumber > 1 && distmin < distthr);
}

int DominantColorExtractor::GetSpatialCoherency(float * ColorData, int dim, int N, float ** col_float, unsigned char * alphaChannelBuffer, int imageWidth, int imageHeight) {
    double CM = .0;
    const int NeighborRange = 1;
    const float SimColorAllow = static_cast<float>(sqrt(DSTMIN));
    unsigned char *pAlpha;

    const auto IVisit = new bool[imageWidth * imageHeight];

    for (int x = 0; x < imageWidth * imageHeight; x++) {
        pAlpha = &alphaChannelBuffer[x];
        if (!alphaChannelBuffer || *pAlpha) {
            IVisit[x] = false;
        }
        else {
            IVisit[x] = true;
        }
    }

    int All_Pixels = 0;

    for (int x = 0; x < imageWidth * imageHeight; x++) {
        if (IVisit[x] == false) {
            All_Pixels++;
        }
    }

    for (int i = 0; i < N; i++) {
        unsigned int Corres_Pixels = 0;
        const double Coherency = GetCoherencyWithColorAllow(ColorData, dim, IVisit, col_float[i][0], col_float[i][1], col_float[i][2],
                                                      SimColorAllow, NeighborRange, &Corres_Pixels, imageWidth, imageHeight);
        CM += Coherency * static_cast<double>(Corres_Pixels) / static_cast<double>(All_Pixels);
    }

    delete[]IVisit;

    return QuantizeSC(CM);
}

double DominantColorExtractor::GetCoherencyWithColorAllow(float * ColorData, int dim, bool * IVisit, float l, float u, float v, float Allow, int NeighborRange, unsigned int * OUTPUT_Corres_Pixel_Count, int imageWidth, int imageHeight) {
    int Neighbor_Count = 0;
    unsigned int Pixel_Count = 0;
    double Coherency = 0.0;
    int count, i, j;

    const int width  = imageWidth;
    const int height = imageHeight;
    const int ISize  = width * height * dim;

    for (count = 0; count < ISize; count += dim) {
        i = (count / dim) % width; //width
        j = (count / dim) / width; //height

        float l1, u1, v1;
        l1 = ColorData[count];
        u1 = ColorData[count + 1];
        v1 = ColorData[count + 2];

        // Distance
        double distance;
        distance = sqrt(sqr(l - l1) + sqr(u - u1) + sqr(v - v1));

        if ((distance < Allow) && (IVisit[count / dim] == false)) { //no overlap checking
            IVisit[count / dim] = true;
            Pixel_Count++;
            int nSameNeighbor = 0;

            for (int y = j - NeighborRange; y <= j + NeighborRange; y++) {
                for (int x = i - NeighborRange; x <= i + NeighborRange; x++) {

                    if (!((i == x) && (j == y))) {

                        const int Index = (y * width + x) * dim;

                        if ((Index >= 0) && (Index < ISize)) {
                            const float l2 = ColorData[Index];
                            const float u2 = ColorData[Index + 1];
                            const float v2 = ColorData[Index + 2];

                            const double distance = sqrt(sqr(l - l2) + sqr(u - u2) + sqr(v - v2));

                            if (distance < Allow) {
                                nSameNeighbor++;
                            }
                        }
                    }
                }
            }
            Neighbor_Count += nSameNeighbor;
        }
    }
    *OUTPUT_Corres_Pixel_Count = Pixel_Count;

    int neighbor_check_window_size = NeighborRange * 2 + 1;

    neighbor_check_window_size *= neighbor_check_window_size;

    if (Pixel_Count == 0) {
        Coherency = 0.0;
    }
    else {
        Coherency = static_cast<double>(Neighbor_Count) / static_cast<double>(Pixel_Count) / static_cast<double>(neighbor_check_window_size - 1);
    }

    return Coherency;
}

int DominantColorExtractor::QuantizeSC(double sc) {
    if (sc < 0.70) {
        return 1;
    }
    else {
        return static_cast<int>((sc - 0.70) / (1.0 - 0.70) * (pow(2.0, (double) SC_BIT) - 3.0) + .5) + 2;
    }
}

void DominantColorExtractor::rgb2yuv(int r, int g, int b, int & y, int & u, int & v) {
    y = static_cast<int>((y < 0) ? 0 : ((y > 255) ? 255 :  0.587 * static_cast<float>(r) + 0.114 * static_cast<float>(g) + 0.299 * static_cast<float>(b) + 0.5));
    u = static_cast<int>((u < 0) ? 0 : ((u > 255) ? 255 : -0.331 * static_cast<float>(r) + 0.500 * static_cast<float>(g) - 0.169 * static_cast<float>(b) + 0.5 + 128));
    v = static_cast<int>((v < 0) ? 0 : ((v > 255) ? 255 : -0.419 * static_cast<float>(r) - 0.081 * static_cast<float>(g) + 0.500 * static_cast<float>(b) + 0.5 + 128));
}

void DominantColorExtractor::rgb2xyz(Image &image, double * XYZ) {
    // Get RGB data from the image
    const unsigned char * RGB = image.getRGB();
    if (!RGB) {
        return;
    }
    
    int width = image.getWidth();
    int height = image.getHeight();
    const int imageSize = image.getSize();
    
    int i = 0;
    double r, g, b;

    // Process all pixels in the image
    for (int i = 0; i < imageSize; i++) {
        const int rgbIndex = i * 3;
        
        // In our implementation, RGB is stored as R,G,B
        r = rgb_pow_table[RGB[rgbIndex]];       // R
        g = rgb_pow_table[RGB[rgbIndex + 1]];   // G
        b = rgb_pow_table[RGB[rgbIndex + 2]];   // B

        // Convert RGB to XYZ using the same transformation matrix
        XYZ[rgbIndex]     = 0.412453 * r + 0.357580 * g + 0.180423 * b;
        XYZ[rgbIndex + 1] = 0.212671 * r + 0.715160 * g + 0.072169 * b;
        XYZ[rgbIndex + 2] = 0.019334 * r + 0.119193 * g + 0.950227 * b;
    }
    
    // Clean up the RGB buffer allocated by Image::getRGB()
    delete[] RGB;
}

void DominantColorExtractor::xyz2luv(double * XYZ, float * LUV, int size) {
    double x, y;
    double den, u2, v2;
    double X0, Z0, Y0;
    double u20, v20;

    // X, Y, Z components for color reference white:
    X0 = (0.607 + 0.174 + 0.201);
    Y0 = (0.299 + 0.587 + 0.114);
    Z0 = (0.000 + 0.066 + 1.117);

    /* Y0 = 1.0 */
    u20 = 4 * X0 / (X0 + 15 * Y0 + 3 * Z0);
    v20 = 9 * Y0 / (X0 + 15 * Y0 + 3 * Z0);

    double X, Y, Z;
    for (int i = 0; i < size; i += 3) {
        X = XYZ[i];
        Y = XYZ[i + 1];
        Z = XYZ[i + 2];

        if (X == 0.0 && Y == 0.0 && Z == 0.0) {
            x = 1.0 / 3.0;
            y = 1.0 / 3.0;
        }
        else {
            den = X + Y + Z;
            x = X / den;
            y = Y / den;
        }

        den = -2 * x + 12 * y + 3;

        u2 = 4 * x / den;
        v2 = 9 * y / den;

        if (Y > 0.008856) {
            LUV[i] = static_cast<float>(116 * pow(Y, 1.0 / 3.0) - 16);
        }
        else {
            LUV[i] = static_cast<float>(903.3 * Y);
        }
        LUV[i + 1] = static_cast<float>(13 * LUV[i] * (u2 - u20));
        LUV[i + 2] = static_cast<float>(13 * LUV[i] * (v2 - v20));
    }
}

void DominantColorExtractor::rgb2luv(Image &image, float * LUV) {
    const auto XYZ = new double[image.getSize() * 3];
    rgb2xyz(image, XYZ);
    xyz2luv(XYZ, LUV, image.getSize() * 3);
    delete XYZ;
}

void DominantColorExtractor::luv2rgb(int * RGB, float *LUV, int size) {
    int i, k;
    double x, y, X, Y, Z, den, u2, v2, X0, Z0, Y0, u20, v20, vec[3];
    float veckf;

    X0 = (0.607 + 0.174 + 0.201);
    Y0 = (0.299 + 0.587 + 0.114);
    Z0 = (0.066 + 1.117);

    /* Y0 = 1.0 */
    u20 = 4 * X0 / (X0 + 15 * Y0 + 3 * Z0);
    v20 = 9 * Y0 / (X0 + 15 * Y0 + 3 * Z0);

    for (i = 0; i < size; i += 3) {
        if (LUV[i] > 0) {
            if (LUV[i] < 8.0) {
                Y = static_cast<double>(LUV[i]) / 903.3;
            }
            else {
                Y = pow((static_cast<double>(LUV[i]) + 16) / 116.0, 3.0);
            }

            u2 = static_cast<double>(LUV[i + 1]) / 13.0 / static_cast<double>(LUV[i]) + u20;
            v2 = static_cast<double>(LUV[i + 2]) / 13.0 / static_cast<double>(LUV[i]) + v20;

            den = 6 + 3 * u2 - 8 * v2;

            if (den < 0) {
                printf("den<0\n");
            }
            if (den == 0) {
                printf("den==0\n");
            }

            x = 4.5 * u2 / den;
            y = 2.0 * v2 / den;

            X = x / y * Y;
            Z = (1 - x - y) / y * Y;
        }
        else {
            X = 0.0; Y = 0.0; Z = 0.0;
        }

        vec[0] = ( 3.240479 * X - 1.537150 * Y - 0.498536 * Z);
        vec[1] = (-0.969256 * X + 1.875992 * Y + 0.041556 * Z);
        vec[2] = ( 0.055648 * X - 0.204043 * Y + 1.057311 * Z);

        for (k = 0; k < 3; k++) {
            if (vec[k] <= 0.018) {
                vec[k] = 255 * 4.5 * vec[k];
            }
            else {
                vec[k] = 255 * (1.099 * pow(vec[k], 0.45) - 0.099);
            }
            if (vec[k] > 255) {
                vec[k] = 255;
            }
            else if (vec[k] < 0) {
                vec[k] = 0;
            }
            veckf = static_cast<float>(vec[k]);

            RGB[i + k] = lroundf(veckf);
        }
    }
}

DominantColorExtractor::~DominantColorExtractor() {
    if (dominantColorWeights) {
        delete[] dominantColorWeights;
    }

    if (dominantColorCentroids) {
        for (int i = 0; i < DESCRIPTOR_SIZE; ++i) {
            delete[] dominantColorCentroids[i];
        }
        delete[] dominantColorCentroids;
    }

    if (dominantColorsVariances) {
        for (int i = 0; i < DESCRIPTOR_SIZE; ++i) {
            delete[] dominantColorsVariances[i];
        }
        delete[] dominantColorsVariances;
    }
    delete descriptor;
}


const double DominantColorExtractor::rgb_pow_table[256] = {
    0.00000000000000000,0.00087150000000000,0.00174300000000000,0.00261450000000000,0.00348600000000000,0.00435750000000000,0.00522900000000000,0.00610050000000000,0.00697200000000000,0.00784350000000000,
    0.00871500000000000,0.00958650000000000,0.01045800000000000,0.01132950000000000,0.01220100000000000,0.01307250000000000,0.01394400000000000,0.01481550000000000,0.01568700000000000,0.01655850000000000,
    0.01743000000000000,0.01831933659099830,0.01921037827158137,0.02012472952058436,0.02106249800205880,0.02202378965300649,0.02300870874533372,0.02401735794440260,0.02504983836442886,0.02610624962095256,
    0.02718668988058822,0.02829125590824254,0.02942004311197209,0.03057314558563813,0.03175065614950360,0.03295266638890465,0.03417926669111881,0.03543054628054251,0.03670659325228132,0.03800749460424920,
    0.03933333626786472,0.04068420313742696,0.04206017909824685,0.04346134705360444,0.04488778895059806,0.04633958580494623,0.04781681772479902,0.04931956393361247,0.05084790279213459,0.05240191181955019,
    0.05398166771382697,0.05558724637130374,0.05721872290555849,0.05887617166559196,0.06055966625335983,0.06226927954068510,0.06400508368557994,0.06576715014800480,0.06755554970509059,0.06937035246584887,
    0.07121162788539302,0.07307944477869201,0.07497387133387773,0.07689497512512539,0.07884282312512503,0.08081748171716195,0.08281901670682246,0.08484749333334034,0.08690297628059925,0.08898552968780481,
    0.09109521715983984,0.09323210177731529,0.09539624610632910,0.09758771220794410,0.09980656164739615,0.10205285550304277,0.10432665437506188,0.10662801839391034,0.10895700722855112,0.11131368009445761,
    0.11369809576140300,0.11611031256104293,0.11855038839429849,0.12101838073854672,0.12351434665462525,0.12603834279365805,0.12859042540370749,0.13117065033625988,0.13377907305254896,0.13641574862972369,
    0.13908073176686489,0.14177407679085596,0.14449583766211291,0.14724606798017689,0.15002482098917547,0.15283214958315544,0.15566810631129177,0.15853274338297649,0.16142611267279158,0.16434826572536848,
    0.16729925376013891,0.17027912767597944,0.17328793805575340,0.17632573517075295,0.17939256898504441,0.18248848915971999,0.18561354505705766,0.18876778574459321,0.19195125999910592,0.19516401631052063,
    0.19840610288572896,0.20167756765233139,0.20497845826230252,0.20830882209558207,0.21166870626359310,0.21505815761268979,0.21847722272753653,0.22192594793442019,0.22540437930449747,0.22891256265697885,
    0.23245054356225084,0.23601836734493881,0.23961607908691010,0.24324372363022154,0.24690134558001073,0.25058898930733331,0.25430669895194774,0.25805451842504840,0.26183249141194892,0.26564066137471626,
    0.26947907155475731,0.27334776497535918,0.27724678444418371,0.28117617255571786,0.28513597169368143,0.28912622403339139,0.29314697154408609,0.29719825599120892,0.30128011893865270,0.30539260175096533,
    0.30953574559551872,0.31370959144464045,0.31791418007770983,0.32214955208321921,0.32641574786080074,0.33071280762322014,0.33504077139833721,0.33939967903103530,0.34378957018511830,0.34821048434517832,
    0.35266246081843255,0.35714553873653149,0.36165975705733805,0.36620515456667907,0.37078176988006922,0.37538964144440817,0.38002880753965124,0.38469930628045468,0.38940117561779564,0.39413445334056724,
    0.39889917707715028,0.40369538429695989,0.40852311231197036,0.41338239827821671,0.41827327919727392,0.42319579191771450,0.42814997313654474,0.43313585940061988,0.43815348710803881,0.44320289250951872,
    0.44828411170974980,0.45339718066873097,0.45854213520308595,0.46371901098736196,0.46892784355530848,0.47416866830113935,0.47944152048077698,0.48474643521307942,0.49008344748104982,0.49545259213303056,
    0.50085390388387963,0.50628741731613247,0.51175316688114680,0.51725118690023286,0.52278151156576813,0.52834417494229835,0.53393921096762098,0.53956665345385835,0.54522653608851324,0.55091889243551306,
    0.55664375593623883,0.56240115991054207,0.56819113755774731,0.57401372195764333,0.57986894607145978,0.58575684274283346,0.59167744469876060,0.59763078455053797,0.60361689479469316,0.60963580781390037,
    0.61568755587788793,0.62177217114433303,0.62788968565974557,0.63404013136034121,0.64022354007290405,0.64643994351563838,0.65268937329901067,0.65897186092658055,0.66528743779582300,0.67163613519893950,
    0.67801798432366012,0.68443301625403707,0.69088126197122635,0.69736275235426270,0.70387751818082456,0.71042559012798967,0.71700699877298257,0.72362177459391352,0.73026994797050804,0.73695154918482919,
    0.74366660842199095,0.75041515577086326,0.75719722122477029,0.76401283468217895,0.77086202594738207,0.77774482473117112,0.78466126065150421,0.79161136323416437,0.79859516191341240,0.80561268603263114,
    0.81266396484496373,0.81974902751394385,0.82686790311412028,0.83402062063167381,0.84120720896502765,0.84842769692545184,0.85568211323765986,0.86297048654040143,0.87029284538704554,0.87764921824616005,
    0.88503963350208426,0.89246411945549509,0.89992270432396804,0.90741541624253208,0.91494228326421789,0.92250333336060231,0.93009859442234510,0.93772809425972170,0.94539186060314961,0.95308992110371005,
    0.96082230333366447,0.96858903478696434,0.97639014287975745,0.98422565495088898,0.99209559826239624,1.00000000000000000 };