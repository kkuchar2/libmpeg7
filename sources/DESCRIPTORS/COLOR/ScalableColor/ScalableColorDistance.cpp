#include "ScalableColorDistance.h"

ScalableColorDistance::ScalableColorDistance() = default;

void ScalableColorDistance::loadParameters(const char ** params) {
    if (params == nullptr) {
        return; // default value remains default
    }

    /* SIZE | PARAM COMBINATION
    ------------------------------------------------------------------
    0 |  [NULL]
    2 |  [NumberOfCoefficients, value, NULL]
    ------------------------------------------------------------------ */

    // Count parameters size
    int size = 0;
    for (int i = 0; params[i] != nullptr; i++) {
        size++;
    }

    // Check size
    if (size != 0 && size != 2) {
        throw SCAL_COL_PARAMS_NUMBER_ERROR;;
    }

    for (int i = 0; params[i] != nullptr; i += 2) {
        if (params[i] == nullptr || params[i + 1] == nullptr) {
            break; // default value remains default
        }

        std::string p1(params[i]);
        std::string p2(params[i + 1]);

        if (!p1.compare("NumberOfCoefficients")) {
            numberOfCoefficients = 
                !p2.compare("16") ||
                !p2.compare("32") ||
                !p2.compare("64") ||
                !p2.compare("128") ||
                !p2.compare("256") ? std::stoi(p2) :
                throw SCAL_COL_PARAMS_VALUE_ERROR;
        }
        else {
            throw SCAL_COL_PARAMS_NAME_ERROR;
        }
    }
}

double ScalableColorDistance::getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params) {
    ScalableColor * scalableColorDescriptor1 = static_cast<ScalableColor *>(descriptor1);
    ScalableColor * scalableColorDescriptor2 = static_cast<ScalableColor *>(descriptor2);

    // ----------------- LOAD PARAMETERS ------------------------------------------------

    /* In parameters user specifies number of coefficients used for calculation.
    For example, if we compare two xmls, one with 256 coefficients and second with 128 coefficients
    parameter "NumberOfCoefficients" has to be 128 or lower (64, 32, 16). 
    Algorithm will compare only those coefficients count.
    
    Parameter "NumberOfBitplanesDiscarded was not used in XM by default 
    (it was different option RecHistogram value in code) */
    try {
        loadParameters(params);
    }
    catch (ErrorCode exception) {
        throw exception;
    }

    // ------------------ SIZE CHECK -----------------------------------------------------

    /* Check, if specified in parameters NumberOfCoefficients value 
    is not greater, than first descriptor coefficients array size
    (For descriptor size 128 we can calculate only 16 | 32 | 64 | 128 coefficients) 
       
     In this case, there are 2 possibilities:
        - user has not definied distance parameters
        - user has definied that parameter, but greater, than acual size 

    Either way numberOfCoeffficients to calculate will get number of coefficients in first descriptor */
    if (scalableColorDescriptor1->getNumberOfCoefficients() < numberOfCoefficients) {
        numberOfCoefficients = scalableColorDescriptor1->getNumberOfCoefficients();
    }

    // If user wants to compare for example 64 coefficients, second descriptor has to be size 64)
    if (scalableColorDescriptor2->getNumberOfCoefficients() != numberOfCoefficients) {
        throw SCAL_COL_XML_COEFF_ERROR;
    }

    // Bits discarded have to same value
    if (scalableColorDescriptor1->getNumberOfBitplanesDiscarded() != scalableColorDescriptor2->getNumberOfBitplanesDiscarded()) {
        throw  SCAL_COL_XML_BITS_DISC_ERROR;
    }

    // ------------------- CALCULATE DISTANCE ---------------------------------------------

    // Prepare full histograms for 256 coefficients (filled with zeros)
    int * histogram1 = (int *) calloc(256, sizeof(int));
    int * histogram2 = (int *) calloc(256, sizeof(int));

    // Fill histograms (only specified coefficient count in parameters will be added)
    for (unsigned int i = 0; i < numberOfCoefficients; i++) {
        histogram1[i] = scalableColorDescriptor1->getCoefficient(i);
        histogram2[i] = scalableColorDescriptor2->getCoefficient(i);
    }

    // Distance algorithm
    double sqrerr = 0;
    double distance;

    for (unsigned int j = 0; j < numberOfCoefficients; ++j) {
        distance = (double) histogram1[j] - (double) histogram2[j];

        if (distance < 0.0) {
            distance = -distance;
        }
        sqrerr += distance;
    }

    return sqrerr;
}

ScalableColorDistance::~ScalableColorDistance() = default;
