/** @file  HomogeneousTextureExtractor.h
*  @brief  Homogeneous Texture class for extraction.
*  @author Krzysztof Lech Kucharski
*  @bug    Code needs to be revised. Very often extraction results with error */

#pragma once

#include "../../DescriptorExtractor.h"
#include "../TextureBrowsing/TextureBrowsing.h"

// Headers for directory creation in Linux
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(_WIN32) && !defined(_WIN64)
    #include <unistd.h>
#endif

#ifndef M_PI
#define M_PI 3.141592653589793115997963468544185161590576171875
#endif

// define parameters used for Gabor decomposition
#define XM_UL			0.05	/* lower frequency */
#define XM_UH			0.4     /* upper frequency */
#define XM_SCALE		4		/* number of scales in generating filters */
#define XM_ORIENTATION	6		/* number of orientations in generating filters */
#define XM_FLAG			1		/* remove the DC */
#define XM_SIDE			40		/* filter mask = 2 * side + 1 x 2 * side + 1 */

// define the thresholds for quantizing PBC
#define	BOUNDARY1	5.1
#define	BOUNDARY2	10.1
#define	BOUNDARY3	19.9

#define SCALE		4
#define ORIENTATION 6
#define DEBUGLEV  0

#define sqr(x) ((x)*(x))
#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#define D_DirectionSize 2
#define LOG2 log(2.0)

typedef struct MatrixStruct {
    double ** data;
    int height, width;
} Matrix;

struct pbc_struct {
    // Regularity
    float structuredness = 1.401e-45;
    int tstructuredness = 0;
    // [Scale1, Direction1, Scale2 , Direction2]
	int element[4];
};

struct Peak {
    int index;
    double value;
    double contrast;
    int flag;
};

class TextureBrowsingExtractor : public DescriptorExtractor {
    private:
        TextureBrowsing * descriptor = nullptr;

        // Aribtrary shape methods
        void ArbitraryShape(unsigned char * a_image, unsigned char * y_image, int image_height, int image_width);
        bool max_test(int a, int & max);
        void min_test(int a, int & min);

        // Main extraction methods:

        // TBC calculation
        void PBC_Extraction(Matrix * img, int width, int height, int * pbc_out);

        // a) Directions
        int GaborFeature(Matrix * img, int side, double Ul, double Uh, int scale, int orientation, int flag, int * pbc);

        // b) Scale
        void pbcmain(struct pbc_struct * pbc, int size);

        // c) Regularity
        void threshold(struct pbc_struct * pbc);

        // Lower level methods:
        void Gabor(Matrix * Gr, Matrix * Gi, int s, int n, double Ul, double Uh, int scale, int orientation, int flag);
        void ComputeProjection(Matrix * inputImage, int xsize, int ysize, double angle, int proj_size, double * proj);
        void ProjectionAnalysis(char * image_name, int proj_type, float * credit, struct pbc_struct * pbc, int img_size);
        float ComputeProjectionContrast(double * B, int leng_B, int * PeakI, float * Peak, int num_peak);
        double ComputeHistogramContrast(int index, double * histo, int len);
         
        void peak_summary(float * dis_peak, float * var_dis, int * PeakI, float * Peak, int size_peak);
        void proj_credit_modi(int ** candidate, int size_cand, int size_count_candi, int flag, float * cand_credit);
        void get_map(int ** cand_pair, int size_cand, int total, int * num, int type, int ** map);
        void convertcand(int ** cand_pair, int ** direc_cand, int size_cand);
        void piksrtintS2B(int n, int * num, int * index);
        void sing_map(int ** inpair, int size_in, int *sing, int type);
        void four1(double * data, int nn, int isign);
        int CountLocalProjectionPeaks(double * B, int leng_B, int * PeakI, float * Peak);
        int RadonAutocorrelation(double * x_in, int x_long, double ** y);
        int JudgeContinuity(int index, int order, struct Peak * peak, int size);
        int agglom(float ** candi, int * label, int N, float thresh, int *count, int Nlabel, int fN);
        int get_num(int ** cand_pair, int size_cand, int * num, int len);
        int cand_cluster(float ** candi, int size_in, int ndim, int * y, int * pick_num);

        int * DominantDirection(float ** histo);

        // C Arrays Operations:
        double * AllocateVectorOfDouble(int nl, int nh);
        int ** AllocateMatrixInteger(int nr, int nc);
        float ** AllocateMatrixFloat(int nr, int nc);
        double ** AllocateMatrixDouble(int nrl, int nrh, int ncl, int nch);

        void FreeVectorOfDouble(double * v, int nl, int nh);
        void FreeMatrixInteger(int ** m, int nr);
        void FreeMatrixFloat(float ** m, int nr);
        void FreeMatrixOfDouble(double ** m, int nrl, int nrh, int ncl, int nch);

        float Vector2DVariance(float ** vector, int nvec, int ndim);
        void Convert2Matrix(unsigned char * R, int width, int height, Matrix * image);
        void FourierTransform2D(double ** fftr, double ** ffti, double ** rdata, double ** idata, int rs, int cs, int isign);
        void sort(double * Y, int * I, double * A, int length);
        float EuclideanVectorDistance(float * a, float * b, int dim);

        // Custom Matrix Operations:
        void CreateMatrix(Matrix ** M, int hei, int wid);
        void Mat_FFT2(Matrix * Output_real, Matrix * Output_imag, Matrix * Input_real, Matrix * Input_imag);
        void Mat_Copy(Matrix * A, Matrix * B, int h_target, int w_target, int h_begin, int w_begin, int h_end, int w_end);
        void Mat_Product(Matrix * A, Matrix * B, Matrix * C);
        void Mat_Substract(Matrix * A, Matrix * B, Matrix * C);
        void Mat_Sum(Matrix * A, Matrix * B, Matrix * C);
        void Mat_IFFT2(Matrix * Output_real, Matrix * Output_imag, Matrix * Input_real, Matrix * Input_imag);
        void ImgRotate(Matrix * inImg, float angle);
        int billinear(Matrix * img, float a, float b, int ii, int jj);
        void FreeMatrix(Matrix * M);

        // Custom method (KK) for memory cleanup when some exeptions raised
        void cleaupMainVariables(int orientation, int scale, double * temp_proj, double ** rowProjections, double ** columnProjections, float ** histo, Matrix * FilteredImageBuffer[4][6], Matrix * Gr, Matrix * Gi, Matrix * Tmp_1, Matrix * Tmp_2, Matrix * F_1, Matrix * F_2, Matrix * G_real, Matrix * G_imag, Matrix * F_real, Matrix * F_imag, Matrix * IMG, Matrix * IMG_imag, Matrix * temp_FilteredImage);
    public:
        TextureBrowsingExtractor();
        Descriptor * extract(Image & image, const char ** params);
        ~TextureBrowsingExtractor();
};
