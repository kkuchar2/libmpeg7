/** @file  HomogeneousTexture.h
 *  @brief Homogeneous Texture descriptor data,
 *         method for loading parameters, read and wirte to XML,
 *         modify and access data and additional classes helpful with extraction
 *
 *  @author Krzysztof Lech Kucharski
 *  @bug No bugs detected. */

#pragma once

#include "../../Descriptor.h"

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

#define Quant_level 255
#define	imsize      128
#define Nray        128
#define Nview       180

#ifndef M_PI
#define	M_PI (3.14159265358979323846)
#endif

typedef struct {
	double r, i; // 
} COMPLEX;

typedef struct {
	double r; //radius
	double a; //angle
} CYLINDER;

typedef struct {
	double x; // x coordinate
	double y; // y coordinate
} CARTESIAN;

class HomogeneousTexture : public Descriptor {
	private:
        int energyDeviationFlag = 1;
		int outputFeature[62];
	public:
        HomogeneousTexture();

        void loadParameters(const char ** params);
        std::string generateXML();
        void readFromXML(XMLElement * descriptorElement);

        void SetHomogeneousTextureFeature(const int * pHomogeneousTextureFeature);
        int GetHomogeneousTextureFeatureFlag() const;
        int * GetHomogeneousTextureFeature();

		~HomogeneousTexture();
};
