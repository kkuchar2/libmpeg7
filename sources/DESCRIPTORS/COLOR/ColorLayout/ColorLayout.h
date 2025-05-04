/** @file   ColorLayout.h                                                                               
 *  @brief  Color Layout descriptor data,                                                               
 *          method for loading parameters, read and wirte to XML,                                       
 *          modify and access data.                                                                     
 *                                                                                                      
 *  Descriptor data:
 *                   Elements                      | Description
 * ----------------------------------------------- | -----------------------
 * numberOfYCoefficients, numberOfCCoefficients    | Number of (Y,Cb,Cr) coefficients for descriptor
 * yCoefficients, cbCoefficients, crCoefficients   | Result series of zigzag scanned DCT coefficients values                                           
 *
 * Parameters:
 *
 * Parameter Name  | Parameter values    | Description
 * --------------- | ------------------- | ------------
 * NumberOfYCoeff  | 3,6,10,15,21,28,64  | Number of coefficients for color component Y
 * NumberOfCCoeff  | 3,6,10,15,21,28,64  | Number of coefficients for color component Cb, Cr
 *
 * Default values:
 *  - 6 for Y
 *  - 3 for Cb and Cr
 *
 * Usage:
 *
 * Size of parameter array  | Parameters
 * ------------------------ | ----------------------------------------------
 * 0 |  [NULL]
 * 2 |  ["NumberOfYCoeff", "[value]", NULL]
 * 2 |  ["NumberOfCCoeff", "[value]", NULL]
 * 4 |  ["NumberOfYCoeff", "[value]", "NumberOfCCoeff", "[value]", NULL]
 * 4 |  ["NumberOfCCoeff", "[value]", "NumberOfYCoeff", "[value]", NULL]
 *
 *
 * Wrong size        => COL_LAY_PARAMS_NUMBER_ERROR thrown \n
 * Params NULL       => default values                     \n
 * size = 0          => default values                     \n
 * Wrong param name  => COL_LAY_PARAMS_NAME_ERROR thrown   \n
 * Wrong param value => COL_LAY_PARAMS_VALUE_ERROR thrown  \n
 *
 *  @author Krzysztof Lech Kucharski                                                                    
 *  @bug    No bugs detected. */

#pragma once

#include "../../Descriptor.h"

class ColorLayout : public Descriptor {
	private:
        // Number of coefficients with default values
        int numberOfYCoefficients = 6; 
        int numberOfCCoefficients = 3;

        // Result coefficients arrays
        int * yCoefficients  = nullptr;
        int * cbCoefficients = nullptr;
        int * crCoefficients = nullptr;
	public:
        ColorLayout();

        // General descriptor functions
        void loadParameters(const char ** params);
        void readFromXML(XMLElement * descriptorElement);
        std::string generateXML();

        // Allocating result arrays
        void allocateYCoefficients();
        void allocateCbCoefficients();
        void allocateCrCoefficients();

        // Setting number of coefficients
        void setNumberOfYCoefficients(int yCoeffNumber);
        void setNumberOfCCoefficients(int cCoeffNumber);

        // Setting specific coefficient at specific index
        void setYCoefficient(int index, int value);
        void setCbCoefficient(int index, int valu);
        void setCrCoefficient(int index, int value);

        // Access to number of coefficients
        int getNumberOfYCoeffcients();
        int getNumberOfCCoefficients();

        // Access to result coefficients array
        int * getYCoefficients();
        int * getCbCoefficients();
        int * getCrCoefficients();

        ~ColorLayout();
};