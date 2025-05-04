/** @file DescriptorType.h
 *  @brief Supported descriptor types by library.
 *   Used both for extraction and distance calculation.
 *   In extraction, descriptor type decides which
 *   extractor object will be created. 
 *   In distance calculation, descriptor type decides, based on xmls
 *   what type two descriptor objects will be created.
 *   If xml is not valid. Type will have NONE value
 *   and library returns error.
 *
 *  @author Krzysztof Lech Kucharski
 *  @bug No known bugs.	*/

#pragma once

enum DescriptorType {
    NONE                  = 0, //!< Empty, not existing descriptor type
	DOMINANT_COLOR_D      = 1, //!< Dominant Color Descriptor
	SCALABLE_COLOR_D      = 2, //!< Scalable Color Descriptor
	COLOR_LAYOUT_D 	      = 3, //!< Color Layout Descriptor
	COLOR_STRUCTURE_D     = 4, //!< Color Structure Descriptor
	CT_BROWSING_D         = 5, //!< Color Temperature Browsing Descriptor
	HOMOGENEOUS_TEXTURE_D = 6, //!< Homogeneous Texture Descriptor
	TEXTURE_BROWSING_D    = 7, //!< Texture Browsing Descriptor
	EDGE_HISTOGRAM_D      = 8, //!< Edge Histogram Descriptor
	REGION_SHAPE_D 		  = 9, //!< Region Shape Descriptor
	CONTOUR_SHAPE_D       = 10 //!< Contour Shape Descriptor
};
