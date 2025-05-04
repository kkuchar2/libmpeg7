/** @file  EdgeHistogram.h
 *  @brief Edge Histogram descriptor data,
 *         method for loading parameters, read and wirte to XML,
 *         modify and access data and additional tables
 *         for modifying data.
 *
 *  @author Krzysztof Lech Kucharski
 *  @bug No bugs detected. */

#ifndef _EDGEHISTOGRAM_H 
#define _EDGEHISTOGRAM_H

#include "../../Descriptor.h"

#define	Te_Define			     11
#define	Desired_Num_of_Blocks	 1100

#define	NoEdge				     0
#define	vertical_edge		     1
#define	horizontal_edge			 2
#define	non_directional_edge	 3
#define	diagonal_45_degree_edge	 4
#define	diagonal_135_degree_edge 5

class EdgeHistogram : public Descriptor {
	private:
		char * m_pEdge_HistogramElement = new char[80];
	public:
        static const double QuantTable[5][8];
        double Local_Edge[80];

        EdgeHistogram();

        void loadParameters(const char ** params);
        void readFromXML(XMLElement * descriptorElement);
        std::string generateXML();

        void setEdgeHistogramElement(int index, int value);
        void setEdgeHistogramElement(char * pEdgeHistogram);

        char getEdgeHistogramElement(int index);

		~EdgeHistogram();
};
#endif