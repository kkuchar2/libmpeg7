#ifndef _ERROR_CODE_H 
#define _ERROR_CODE_H 

enum ErrorCode {
    // Main errors
    PARAMS_NULL                     = 1,  //!< Parameters pointer is NULL (1)
    UNRECOGNIZED_DESCRIPTOR_TYPE    = 2,  //!< Unrecognized descriptor type (2)
    CANNOT_OPEN_IMAGE               = 3,  //!< Image cannot be opened (3)
    IMAGE_CHANNELS_NOT_SUPPORTED    = 4,  //!< Image has unsupported channels count (4)
    EXTRACTION_RESULT_NULL          = 5,  //!< Extraction result is NULL (5)
    EXTRACTION_MESSAGE_NULL         = 6,  //!< Message from extraction result is NULL (6)
    XML_NULL                        = 7,  //!< One of passed xmls is NULL (7)                                    
    PARSE_ERROR                     = 8,  //!< Error while parsing one of xmls (8)
    MPEG7_NODE_NOT_FOUND            = 9,  //!< Mpeg7 element could not be found in one of xmls (9)
    DESCRIPTION_UNIT_NODE_NOT_FOUND = 10, //!< DescriptionUnit element could not be found in one of xmls (10)
    DESCRIPTOR_NODE_NOT_FOUND       = 11, //!< Descriptor element could not be found in one of xmls (11)
    TYPE_ATTRIBUTE_NOT_FOUND        = 12, //!< xsi::type attribute of Descriptor element could not be found in one of xmls (12)
    XML_TYPE_NOT_RECOGNIZED         = 13, //!< One of descriptor type is not recognized (13)
    XML_TYPES_NOT_EQUAL             = 14, //!< Descriptor types are not equal (14)
    XML_READ_FAILED                 = 15, //!< Error while reading at least one of two descriptors from descriptor element (15)
    DISTANCE_RESULT_NULL            = 16, //!< Distance calculation result is NULL (16)
    DISTANCE_MESSAGE_NULL           = 17, //!< Message from distance calculation is NULL (17)
    // JNI
    JNI_DESCRIPTOR_TYPE_NULL        = 18, //!< Descriptor type passed from Java is NULL (18)
    JNI_IMG_NULL                    = 19, //!< Image passed from Java is NULL (path or array of file data) (19)
    JNI_PARAMS_NULL                 = 20, //!< Parameters array passed from Java is NULL (20)
    JNI_XML_NULL                    = 21, //!< Passed xml string from Java is NULL (21)

    // Color Layout
    COL_LAY_PARAMS_NULL            = 22, //!< Params are NULL (1)
    COL_LAY_PARAMS_NUMBER_ERROR    = 23, //!< Number of parameters is wrong (2)
    COL_LAY_PARAMS_NAME_ERROR      = 24, //!< Wrong name for parameter (3)
    COL_LAY_PARAMS_VALUE_ERROR     = 25, //!< Wrong value for parameter (4)
    COL_LAY_DC_NODE_ERROR          = 26, //!< Could not find some dc nodes in XML (5)
    COL_LAY_AC_NODE_ERROR          = 27, //!< Could not find some ac nodes in XML (6)
    COL_LAY_WRONG_COEFF_NUMBER     = 28, //!< Wrong number of coefficients in XML attribute (7)
    COL_LAY_COEFF_NUMBER_DIFFERENT = 29, //!< Xmls have different coefficients number (8)

    // Color Structure
    COL_STRUCT_PARAMS_NULL              = 30, //!< Params are NULL (1)
    COL_STRUCT_PARAMS_NUMBER_ERROR      = 31, //!< Number of parameters is wrong (2)
    COL_STRUCT_PARAM_NAME_ERROR         = 32, //!< Wrong name for parameter (3)
    COL_STRUCT_PARAM_VALUE_ERROR        = 33, //!< Wrong value for parameter (4)
    COL_STRUCT_SETSIZE_ALLOCATION_ERROR = 34, //!< Allocation error for method SetSize (5)
    COL_STRUCT_SETSIZE_SIZE_ERROR       = 35, //!< Size error in SetSize function (6)
    COL_STRUCT_SETELEMENT_OUT_OF_BOUNDS = 36, //!< Out of bounds  while setting element (7)
    COL_STRUCT_GETELEMENT_WRONG_ACCESS  = 37, //!< Wrong element access (8)
    COL_STRUCT_GETBINSIZE_ERROR         = 38, //!< GetBinSize error (9)
    COL_STRUCT_GETBINSIZE_OUT_OF_BOUNDS = 39, //!< GetBinSize out of bounds (10)
    COL_STRUCT_UNIFYBINS_ERROR          = 40, //!< UnifyBins error (11)
    COL_STRUCT_DISTANCE_INCORRECT       = 41, //!< Distance is wrong (12)
    COL_STRUCT_XML_QUANT_VAL_ERROR      = 42, //!< Wrong xml ColorQuantSize value (13)
    COL_STRUCT_XML_VALUE_NOT_INTEGER    = 43, //!< Cannot read value for <Values> (14)
    COL_STRUCT_VALUES_NODE_NOT_FOUND    = 44, //!< <Values> node not found in xml (15)
    COL_STRUCT_VALUES_COUNT_ERROR       = 45, //!< Values coefficients wrong quantity (16)
    COL_STRUCT_DISTANCE_SIZE_ERROR      = 46, //!< Descriptors size not equal while comparing (17)
    COL_STRUCT_QUANTAMPL_NONLINEAR_TOTAL_LEVELS   = 47, //!< QuantAmplNonLinear totalLevels error (18)
    COL_STRUCT_QUANTAMPL_NOLINEAR_NORMALIZE_ERROR = 48, //!< QuantAmplNonLinear normalize error (19)
    COL_STRUCT_BUILD_TRANSFORM_TABLE_ERROR        = 49, //!< Error building transform table (20)
    COL_STUCT_GETCOLORQSPACE_ERROR                = 50, //!< Error in GetColorQuantSpace (21)
    COL_STRUCT_SLIDE_HISTOGRAM_ALLOCATION_FAILED  = 51, //!< Slide histogram allocation error (22)

    // CT Browsing
    CT_BROWSING_PARAMS_NULL                 = 52, //!< Params are NULL (1)
    CT_BROWSING_BROWSING_CATEGORY_NOT_FOUND = 53, //!< XML cannot find <BrowsingCategory> element (2)
    CT_BROWSING_UNRECOGNIZED_CATEGORY       = 54, //!< XML unrecognized <BrowsingCategory> value (3)
    CT_BROWSING_SUBRANGE_IDX_NOT_FOUND      = 55, //!< XML cannot find <SubRangeIndex> element (4)
    CT_BROWSING_GEN_CATEGORY_ERROR          = 56, //!< Wrong value for browsing category while generating XML (5)

    // Dominant Color
    DOM_COL_PARAMS_NULL                    = 57, //!< Params are NULL (1)
    DOM_COL_PARAMS_NUMBER_ERROR            = 58, //!< Number of parameters is wrong (2)
    DOM_COL_PARAM_NAME_ERROR               = 59, //!< Wrong name for parameter (3)
    DOM_COL_PARAM_VALUE_ERROR              = 60, //!< Wrong value for parameter (4)
    DOM_COL_XML_NO_PERCENTAGE_ELEMENT      = 61, //!< <Percentage> element missing in XML (5)
    DOM_COL_XML_NO_INDEX_ELEMENT           = 62, //!< <Index> element missing in XML (6)
    DOM_COL_XML_PERCENTAGE_NOT_INTEGER     = 63, //!< Cannot read percentage value (7)
    DOM_COL_XML_INDEX_VALUE_NOT_INTEGER    = 64, //!< Cannot read index value (8)
    DOM_COL_XML_VARIANCE_VALUE_NOT_INTEGER = 65, //!< Cannot read variance value (9)

    // Scalable Color
    SCAL_COL_PARAMS_NULL            = 66, //!< Params are NULL
    SCAL_COL_PARAMS_NUMBER_ERROR    = 67, //!< Number of parameters is wrong
    SCAL_COL_PARAMS_NAME_ERROR      = 68, //!< Wrong name for parameter
    SCAL_COL_PARAMS_VALUE_ERROR     = 69, //!< Wrong value for parameter
    SCAL_COL_XML_COEFF_ERROR        = 70, //!< Coefficients read/write and compare error 
    SCAL_COL_XML_BITS_DISC_ERROR    = 71, //!< NumberOfBitplanesDiscarded read/write error
    SCAL_COL_XML_COEFF_NODE_MISSING = 72, //!< <Coefficients> node missing in XML
    SCAL_COL_XML_COEFF_MATCH_ERROR  = 73, //!< Coefficients compare error when calculating distance (after first check)

    // Region Shape
    REG_SHAPE_COEFF_COUNT_ERROR     = 74, //!< Wrong number of coefficients in XML
    REG_SHAPE_MAGNITUDE_ART_MISSING = 75, //!< <MagnitudeOfART> element missing from XML
    REG_SHAPE_WRONG_ELEMENT_ACCESS  = 76, //!< GetElement wrong element access

    // Contour Shape
    CONT_SHAPE_XML_GLOBAL_CURVATURE_MISSING       = 77, //!< <GlobalCurvature> element missing in XML
    CONT_SHAPE_XML_HIGHEST_PEAK_MISSING           = 78, //!< <HighestPeak> element missing from XML
    CONT_SHAPE_XML_GLOBAL_CURVARURE_SIZE_ERROR    = 79, //!< XML <GlobalCurvature> quantity of values wrong
    CONT_SHAPE_XML_PROTOTYPE_CURVARURE_SIZE_ERROR = 80, //!< XML <PrototypeCurvature> quantity of values wrong
    CONT_SHAPE_DISTANCE_ERROR                     = 81, //!< Descriptor distance wrong value

    // Homogeneous Texture
    HOMOG_TEXT_PARAMS_NULL           = 82, //!< Params are NULL
    HOMOG_TEXT_PARAMS_NUMBER_ERROR   = 83, //!< Number of parameters is wrong
    HOMOG_TEXT_PARAMS_NAME_ERROR     = 84, //!< Wrong name for parameter
    HOMOG_TEXT_PARAMS_VALUE_ERROR    = 85, //!< Wrong value for parameter
    HOMOG_TEXT_IMAGE_TOO_SMALL_128   = 86, //!< Analyzed image is smaller than 128 x 128 pix
    HOMOG_TEXT_XML_STANDDEV_MISSING  = 87, //!< <StandardDeviation> element missing from XML
    HOMOG_TEXT_XML_AVERAGE_MISSING   = 88, //!< <Average> element missing from XML
    HOMOG_TEXT_XML_ENERGY_MISSING    = 89, //!< <Energy> element missing from XML
    HOMOG_TEXT_XML_ENERGY_SIZE_ERROR = 90, //!< <EnergyDeviation> missing from XML
    HOMOG_TEXT_ARBITRARY_SHAPE_ERROR = 91, //!< Coefficients descriptor comparison error

    // Texture Browsing
    TEXT_BROWS_PARAMS_NULL                  = 92,  //!< Params are NULL
    TEXT_BROWS_PARAMS_NAME_ERROR            = 93,  //!< Wrong name for parameter
    TEXT_BROWS_PARAMS_VALUE_ERROR           = 94,  //!< Wrong value for parameter
    TEXT_BROWS_WRONG_COMPONENT_VALUE        = 95,  //!< Wrong component value in XML generation
    TEXT_BROWS_XML_REGULARITY_MISSING       = 96,  //!< <Regularity> element missing from XML
    TEXT_BROWS_XML_DIRECTION_MISSING        = 97,  //!< <Direction> element missing from XML
    TEXT_BROWS_XML_SCALE_MISSING            = 98,  //!< <Scale> element missing from XML
    TEXT_BROWS_XML_WRONG_VALUES             = 99,  //!< Wrong values in XML
    TEXT_BROWS_ALLOCATION_MATRIX_ERROR      = 100, //!< Allocation matrix error
    TEXT_BROWS_FILE_READ_ERROR              = 101, //!< Projection file read error
    TEXT_BROWS_PROJECTION_COMPUTATION_ERROR = 102, //!< Projection computation error
    TEXT_BROWSING_CANNOT_OPEN_FILE          = 103, //!< Cannot open projection file
    TEXT_BROWS_PARAMS_NUMBER_ERROR          = 104, //!< Number of parameters is wrong

    // Edge Histogram
    EDGE_HIST_BIN_COUNTS_SIZE_ERROR = 105, //!< BinCounts size in XML
    EDGE_HIST_XML_BINCOUNTS_MISSING = 106, //!< <BinCounts> element missing from XML
};

#endif