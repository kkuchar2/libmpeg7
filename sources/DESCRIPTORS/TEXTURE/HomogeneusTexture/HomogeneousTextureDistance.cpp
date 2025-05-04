#include "HomogeneousTextureDistance.h"

HomogeneousTextureDistance::HomogeneousTextureDistance() {
}

void HomogeneousTextureDistance::loadParameters(const char ** params) {
    if (params[0] == nullptr) {
        return; // default value remains default
    }

    /* SIZE | PARAM COMBINATION
    ------------------------------
    0 |  [NULL]
    2 |  [option, value, NULL]
    ------------------------------ */

    // Count parameters size
    int size = 0;
    for (int i = 0; params[i] != nullptr; i++) {
        size++;
    }

    // Check size
    if (size != 0 && size != 2) {
        throw HOMOG_TEXT_PARAMS_NUMBER_ERROR;
    }

    for (int i = 0; params[i] != nullptr; i += 2) {
        if (params[i] == nullptr || params[i + 1] == nullptr) {
            break; // default value remains default
        }

        std::string p1(params[i]);
        std::string p2(params[i + 1]);

        //----- Distance parameters -----------------------------------
        // n        - default value
        // r        - rotation invariant
        // s        - scale invariant, 
        // rs || sr - rotation & scale invariant
        //-------------------------------------------------------------

        if (!p1.compare("option")) {
            option = !p2.compare("n")  ? nullptr
                         :
                     !p2.compare("r")  ? "r"  :
                     !p2.compare("s")  ? "s"  :
                     !p2.compare("rs") ? "rs" :
                     !p2.compare("sr") ? "rs" : nullptr;
        }
        else {
            throw HOMOG_TEXT_PARAMS_NAME_ERROR;
        }
    }
}

double HomogeneousTextureDistance::getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params) {
    HomogeneousTexture * homogeneousTextureDescriptor1 = static_cast<HomogeneousTexture *>(descriptor1);
    HomogeneousTexture * homogeneousTextureDescriptor2 = static_cast<HomogeneousTexture *>(descriptor2);

    // Load parameters    
    try {
        loadParameters(params);
    }
    catch (ErrorCode exception) {
        throw exception;
    }

    // Calculate the distance
    int RefFeature[NUMofFEATURE] = { 0 };
    int QueryFeature[NUMofFEATURE] = { 0 };

    float fRefFeature[NUMofFEATURE] = { 0.0 };
    float fQueryFeature[NUMofFEATURE] = { 0.0 };

    memcpy(QueryFeature, homogeneousTextureDescriptor1->GetHomogeneousTextureFeature(), sizeof(int) * NUMofFEATURE);
    memcpy(RefFeature, homogeneousTextureDescriptor2->GetHomogeneousTextureFeature(),   sizeof(int) * NUMofFEATURE);
    
    Dequantization(RefFeature, fRefFeature);
    Dequantization(QueryFeature, fQueryFeature);

    Normalization(fRefFeature);
    Normalization(fQueryFeature);

    double temp_distance = 0.0;
    double distance = 0.0;

    double wdc = 0.28;
    double wstd = 0.22;

    distance =  (wdc  * fabs(fRefFeature[0] - fQueryFeature[0]));
    distance += (wstd * fabs(fRefFeature[1] - fQueryFeature[1]));

    int flag = homogeneousTextureDescriptor1->GetHomogeneousTextureFeatureFlag() * homogeneousTextureDescriptor2->GetHomogeneousTextureFeatureFlag();

    double wm[RadialDivision] = { 0.42,1.00,1.00,0.08,1.00 };
    double wd[RadialDivision] = { 0.32,1.00,1.00,1.00,1.00 };


    // ROTATION INVARIANT
    if (option != nullptr && strcmp(option, "r") == 0) {
        double min = DBL_MAX;

        for (int i = AngularDivision; i > 0; i--) {
            temp_distance = 0.0;

            for (int n = 0; n < RadialDivision; n++)
                for (int m = 0; m < AngularDivision; m++) {
                    if (m >= i) {
                        temp_distance += (wm[n] * fabs(fRefFeature[n * AngularDivision + m + 2 - i]      - fQueryFeature[n * AngularDivision + m + 2]))
                                + flag * (wd[n] * fabs(fRefFeature[n * AngularDivision + m + 30 + 2 - i] - fQueryFeature[n * AngularDivision + m + 30 + 2]));
                    }
                    else {
                        temp_distance += (wm[n] * fabs(fRefFeature[(n + 1) * AngularDivision + m + 2 - i]      - fQueryFeature[n * AngularDivision + m + 2]))
                                + flag * (wd[n] * fabs(fRefFeature[(n + 1) * AngularDivision + m + 30 + 2 - i] - fQueryFeature[n * AngularDivision + m + 30 + 2]));
                    }
                }

            if (temp_distance < min) {
                min = temp_distance;
            }
        }
        distance += min;
        return distance;
    }

    // SCALE INVARIANT 
    else if (option != nullptr && strcmp(option, "s") == 0) {
        double min = DBL_MAX;
        int i;

        for (i = 0; i < 3; i++) {
            temp_distance = 0.0;

            for (int n = 2; n < RadialDivision; n++) {
                for (int m = 0; m < AngularDivision; m++) {
                    temp_distance += (wm[n] * fabs(fRefFeature[n * AngularDivision + m + 2]      - fQueryFeature[(n - i) * AngularDivision + m + 2]))
                            + flag * (wd[n] * fabs(fRefFeature[n * AngularDivision + m + 30 + 2] - fQueryFeature[(n - i) * AngularDivision + m + 30 + 2]));
                }
            }

            if (temp_distance < min) {
                min = temp_distance;
            }
        }

        for (i = 1; i < 3; i++) {
            temp_distance = 0.0;

            temp_distance =  (wdc  * fabs(fRefFeature[0] - fQueryFeature[0]));
            temp_distance += (wstd * fabs(fRefFeature[1] - fQueryFeature[1]));

            for (int n = 2; n < RadialDivision; n++) {
                for (int m = 0; m < AngularDivision; m++) {
                    temp_distance += (wm[n] * fabs(fRefFeature[(n - i) * AngularDivision + m + 2]      - fQueryFeature[n * AngularDivision + m + 2]))
                            + flag * (wd[n] * fabs(fRefFeature[(n - i) * AngularDivision + m + 30 + 2] - fQueryFeature[n * AngularDivision + m + 30 + 2]));
                }
            }

            if (temp_distance < min) {
                min = temp_distance;
            }
        }

        distance += min;
        return distance;
    }

    // ROTATION AND SCALE INVARIANT
    else if (option != nullptr && strcmp(option, "rs") == 0) {
        double min = DBL_MAX;
        
        for (int j = 0; j < 3; j++) {
            for (int i = AngularDivision; i > 0; i--) {

                temp_distance = 0.0;

                for (int n = 2; n < RadialDivision; n++) {
                    for (int m = 0; m < AngularDivision; m++) {
                        if (m >= i) {
                            temp_distance += (wm[n] * fabs(fRefFeature[n * AngularDivision + m + 2 - i]      - fQueryFeature[(n - j) * AngularDivision + m + 2]))
                                    + flag * (wd[n] * fabs(fRefFeature[n * AngularDivision + m + 30 + 2 - i] - fQueryFeature[(n - j) * AngularDivision + m + 30 + 2]));
                        }
                        else {
                            temp_distance += (wm[n] * fabs(fRefFeature[(n + 1) * AngularDivision + m + 2 - i]      - fQueryFeature[(n - j) * AngularDivision + m + 2]))
                                    + flag * (wd[n] * fabs(fRefFeature[(n + 1) * AngularDivision + m + 30 + 2 - i] - fQueryFeature[(n - j) * AngularDivision + m + 30 + 2]));
                        }
                    }
                }
                if (temp_distance < min) {
                    min = temp_distance;
                }
            }
        }

        for (int j = 1; j < 3; j++) {
            for (int i = AngularDivision; i > 0; i--) {
                temp_distance = 0.0;

                for (int n = 2; n < RadialDivision; n++) {
                    for (int m = 0; m < AngularDivision; m++) {
                        if (m >= i) {
                            temp_distance += (wm[n] * fabs(fRefFeature[(n - j) * AngularDivision + m + 2 - i]      - fQueryFeature[n * AngularDivision + m + 2]))
                                    + flag * (wd[n] * fabs(fRefFeature[(n - j) * AngularDivision + m + 30 + 2 - i] - fQueryFeature[n * AngularDivision + m + 30 + 2]));
                        }
                        else {
                            temp_distance += (wm[n] * fabs(fRefFeature[(n + 1 - j) * AngularDivision + m + 2 - i]      - fQueryFeature[n * AngularDivision + m + 2]))
                                    + flag * (wd[n] * fabs(fRefFeature[(n + 1 - j) * AngularDivision + m + 30 + 2 - i] - fQueryFeature[n * AngularDivision + m + 30 + 2]));
                        }
                    }
                }

                if (temp_distance < min) {
                    min = temp_distance;
                }
            }
        }
        distance = min + distance;
        return distance;
    }

    // DEFAULT
    else {

        for (int n = 0; n < RadialDivision; n++) {
            for (int m = 0; m < AngularDivision; m++) {
                distance += (wm[n] * fabs(fRefFeature[n * AngularDivision + m + 2]      - fQueryFeature[n * AngularDivision + m + 2]))
                   + flag * (wd[n] * fabs(fRefFeature[n * AngularDivision + m + 30 + 2] - fQueryFeature[n * AngularDivision + m + 30 + 2]));
            }
        }
        return distance;
    }
}

void HomogeneousTextureDistance::Dequantization(int * integerFeature, float * floatFeature) {
    double dcmin = 0.0;
    double dcmax = 255.0;

    double stdmin = 1.309462;
    double stdmax = 109.476530;

    double mmax[5][6] = { 
    { 18.392888,18.014313,18.002143,18.083845,18.046575,17.962099 },
    { 19.368960,18.628248,18.682786,19.785603,18.714615,18.879544 },
    { 20.816939,19.093605,20.837982,20.488190,20.763511,19.262577 },
    { 22.298871,20.316787,20.659550,21.463502,20.159304,20.280403 },
    { 21.516125,19.954733,20.381041,22.129800,20.184864,19.999331 } };

    double mmin[5][6] = { 
    { 6.549734, 8.886816, 8.885367, 6.155831, 8.810013, 8.888925 },
    { 6.999376, 7.859269, 7.592031, 6.754764, 7.807377, 7.635503 },
    { 8.299334, 8.067422, 7.955684, 7.939576, 8.518458, 8.672599 },
    { 9.933642, 9.732479, 9.725933, 9.802238,10.076958,10.428015 },
    { 11.704927,11.690975,11.896972,11.996963,11.977944,11.944282 } };

    double dmax[5][6] = { 
    { 21.099482,20.749788,20.786944,20.847705,20.772294,20.747129 },
    { 22.658359,21.334119,21.283285,22.621111,21.773690,21.702166 },
    { 24.317046,21.618960,24.396872,23.797967,24.329333,21.688523 },
    { 25.638742,24.102725,22.687910,25.216958,22.334769,22.234942 },
    { 24.692990,22.978804,23.891302,25.244315,24.281915,22.699811 } };

    double dmin[5][6] = { 
    { 9.052970,11.754891,11.781252, 8.649997,11.674788,11.738701 },
    { 9.275178,10.386329,10.066189, 8.914539,10.292868,10.152977 },
    { 10.368594,10.196313,10.211122,10.112823,10.648101,10.801070 },
    { 11.737487,11.560674,11.551509,11.608201,11.897524,12.246614 },
    { 13.303207,13.314553,13.450340,13.605001,13.547492,13.435994 } };

    int n, m;
    float dcstep, stdstep, mstep, dstep;

    dcstep  = static_cast<float>((dcmax - dcmin) / Quant_level);
    stdstep = static_cast<float>((stdmax - stdmin) / Quant_level);

    floatFeature[0] = static_cast<float>(dcmin + integerFeature[0] * dcstep);  
    floatFeature[1] = static_cast<float>((stdmin + integerFeature[1] * stdstep));

    for (n = 0; n < RadialDivision; n++) {
        for (m = 0; m < AngularDivision; m++) {
            mstep = static_cast<float>((mmax[n][m] - mmin[n][m]) / Quant_level);
            floatFeature[n * AngularDivision + m + 2] = static_cast<float>(mmin[n][m] + integerFeature[n * AngularDivision + m + 2] * mstep);
        }
    }

    for (n = 0; n < RadialDivision; n++) {
        for (m = 0; m < AngularDivision; m++) {
            dstep = static_cast<float>((dmax[n][m] - dmin[n][m]) / Quant_level);
            floatFeature[n * AngularDivision + m + 32] = static_cast<float>(dmin[n][m] + integerFeature[n * AngularDivision + m + 32] * dstep);
        }
    }
}

void HomogeneousTextureDistance::Normalization(float * feature) {
    int n, m;

    double dcnorm  = 122.331353;
    double stdnorm = 51.314701;

    double mmean[RadialDivision][AngularDivision] = { 
    { 13.948462, 15.067986, 15.077915, 13.865536, 15.031283, 15.145633 },
    { 15.557970, 15.172251, 15.357618, 15.166167, 15.414601, 15.414378 },
    { 17.212408, 16.173027, 16.742651, 16.913837, 16.911480, 16.582123 },
    { 17.911104, 16.761711, 17.065447, 17.867548, 17.250889, 17.050728 },
    { 17.942741, 16.891190, 17.101770, 18.032434, 17.295305, 17.202160 } };

    double dmean[RadialDivision][AngularDivision] = { 
    { 16.544933, 17.845844, 17.849176, 16.484509, 17.803377, 17.928810 },
    { 18.054886, 17.617800, 17.862095, 17.627794, 17.935352, 17.887453 },
    { 19.771456, 18.512341, 19.240444, 19.410559, 19.373478, 18.962496 },
    { 20.192045, 18.763544, 19.202494, 20.098207, 19.399082, 19.032280 },
    { 19.857040, 18.514065, 18.831860, 19.984838, 18.971045, 18.863575 } };

    feature[0] /= static_cast<float>(dcnorm);
    feature[1] /= static_cast<float>(stdnorm);

    for (n = 0; n < RadialDivision; n++) {
        for (m = 0; m < AngularDivision; m++) {
            feature[n * AngularDivision + m + 2] /= static_cast<float>(mmean[n][m]);
        }
    }

    for (n = 0; n < RadialDivision; n++) {
        for (m = 0; m < AngularDivision; m++) {
            feature[n * AngularDivision + m + 32] /= static_cast<float>(dmean[n][m]);
        }
    }
}

HomogeneousTextureDistance::~HomogeneousTextureDistance() {
}