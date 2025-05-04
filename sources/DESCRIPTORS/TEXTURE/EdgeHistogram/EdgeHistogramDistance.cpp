#include "EdgeHistogramDistance.h"

EdgeHistogramDistance::EdgeHistogramDistance() = default;

double EdgeHistogramDistance::getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params) {
    EdgeHistogram * edgeHistogramDescriptor1 = static_cast<EdgeHistogram *>(descriptor1);
    EdgeHistogram * edgeHistogramDescriptor2 = static_cast<EdgeHistogram *>(descriptor2);

    double Total_EdgeHist_Ref[150];       // Local(80)+ Global(5)+Semi_Global(65) 
    double Total_EdgeHist_Query[150];

    Make_Global_SemiGlobal(edgeHistogramDescriptor1->Local_Edge,   Total_EdgeHist_Ref);
    Make_Global_SemiGlobal(edgeHistogramDescriptor2->Local_Edge, Total_EdgeHist_Query);

    double dist = 0.0;

    for (int i = 0; i < 80 + 70; i++) {
        dist += (fabs((Total_EdgeHist_Ref[i] - Total_EdgeHist_Query[i])));   // Global(5)+Semi_Global(65) 
    }

    return dist;
}

void EdgeHistogramDistance::Make_Global_SemiGlobal(double * LocalHistogramOnly, double * TotalHistogram) {
    int i, j;

    memcpy(TotalHistogram + 5, LocalHistogramOnly, 80 * sizeof(double));

    // Make Global Histogram Start
    for (i = 0; i < 5; i++) {
        TotalHistogram[i] = 0.0;
    }

    for (j = 0; j < 80; j += 5) {
        for (i = 0; i < 5; i++) {
            TotalHistogram[i] += TotalHistogram[5 + i + j];
        }
    }

    for (i = 0; i < 5; i++) {
        TotalHistogram[i] = TotalHistogram[i] * 5 / 16.0; // Global * 5.
    }
    // Make Global Histogram end

    // Make Semi-Global Histogram start
    for (i = 85; i <105; i++) {
        j = i - 85;
        TotalHistogram[i] = (TotalHistogram[5 + j]
                           + TotalHistogram[5 + 20 + j]
                           + TotalHistogram[5 + 40 + j]
                           + TotalHistogram[5 + 60 + j]) / 4.0;
    }

    for (i = 105; i < 125; i++) {
        j = i - 105;
        TotalHistogram[i] = (TotalHistogram[5 + 20 * (j / 5) + j % 5]
                           + TotalHistogram[5 + 20 * (j / 5) + j % 5 + 5]
                           + TotalHistogram[5 + 20 * (j / 5) + j % 5 + 10]
                           + TotalHistogram[5 + 20 * (j / 5) + j % 5 + 15]) / 4.0;
    }

    ///////////////////////////////////////////////////////
    //				4 area Semi-Global
    ///////////////////////////////////////////////////////

    //  Upper area 2.
    for (i = 125; i < 135; i++) {
        j = i - 125;    // j = 0 ~ 9
        TotalHistogram[i] = (TotalHistogram[5 + 10 * (j / 5) + 0 + j % 5]
                           + TotalHistogram[5 + 10 * (j / 5) + 5 + j % 5]
                           + TotalHistogram[5 + 10 * (j / 5) + 20 + j % 5]
                           + TotalHistogram[5 + 10 * (j / 5) + 25 + j % 5]) / 4.0;
    }

    //  Down area 2.
    for (i = 135; i < 145; i++) {
        j = i - 135;    // j = 0 ~ 9
        TotalHistogram[i] = (TotalHistogram[5 + 10 * (j / 5) + 40 + j % 5]
                           + TotalHistogram[5 + 10 * (j / 5) + 45 + j % 5]
                           + TotalHistogram[5 + 10 * (j / 5) + 60 + j % 5]
                           + TotalHistogram[5 + 10 * (j / 5) + 65 + j % 5]) / 4.0;
    }

    // Center Area 1 
    for (i = 145; i < 150; i++) {
        j = i - 145;    // j = 0 ~ 9
        TotalHistogram[i] = (TotalHistogram[5 + 25 + j % 5]
                           + TotalHistogram[5 + 30 + j % 5]
                           + TotalHistogram[5 + 45 + j % 5]
                           + TotalHistogram[5 + 50 + j % 5]) / 4.0;
    }
    // Make Semi-Global Histogram end
}

EdgeHistogramDistance::~EdgeHistogramDistance() = default;
