from ctypes import *
import sys
import os
import math
import re
import platform
import urllib
import urllib.request

'''
###############################################################
# pympeg7 - A wrapper for libmpeg7pw library
###############################################################
'''

# ------------ LOAD LIBRARY -----------------------------------
operating_system = platform.system();

if operating_system == "Linux":
        cdll.LoadLibrary("libopencv_core.so.3.3.0");
        lib = cdll.LoadLibrary("libmpeg7pw.so");
elif operating_system == "Windows":
        cdll.LoadLibrary("opencv_world310");
        lib = cdll.LoadLibrary("libmpeg7pw");
else:
    raise PyMpeg7Exception("Unrecognized operating system", -1)

# ----------------------- DESCRIPTOR TYPE ----------------------
class DescriptorType:
   DOMINANT_COLOR_D      = 1
   SCALABLE_COLOR_D      = 2
   COLOR_LAYOUT_D        = 3
   COLOR_STRUCTURE_D     = 4
   CT_BROWSING_D         = 5
   HOMOGENEOUS_TEXTURE_D = 6
   TEXTURE_BROWSING_D    = 7
   EDGE_HISTOGRAM_D      = 8
   REGION_SHAPE_D        = 9
   CONTOUR_SHAPE_D       = 10

# ----------------------- MAIN FUNCTIONS  ----------------------
def extractDescriptor(descriptorType, imgPath, parameters):
    # ------------ CHECK ARGUMENTS ------------

    if descriptorType == None or type(descriptorType) != int:
        raise PyMpeg7Exception("Descriptor type is 'None' or is not type 'int'", 1)

    if imgPath == None or type(imgPath) != str:
        raise PyMpeg7Exception("Image path is 'None' or is not type 'str'", 2)

    if parameters == None or type(parameters) != list:
        raise PyMpeg7Exception("Python error: Parameters are 'None' or are not type 'list'", 3)

    # ------------ CONVERT ARGUMENTS --------------

    # Convert image path to const char *
    imgPath = toConstCharPointer(imgPath)  
    # Convert parameters to const char **
    parameters = toConstCharPointerArray(parameters) 

    # ------------ SET DATATYPES ------------------
    lib.extractDescriptor.argtypes = [c_int, c_char_p, type(parameters)]
    lib.extractDescriptor.restype  = c_void_p

    # ------------ EXTRACT DESCRIPTOR  ------------

    # Call - extract and receive pointer to result
    result = lib.extractDescriptor(descriptorType, imgPath, parameters)
        
    # Get  pointer value
    result_pointer_value = cast(result, c_char_p).value;
        
    # Free pointer from C
    freeResultPointer(result)

    # Decode received value
    final_result = result_pointer_value.decode("utf-8");

    # Detect possible errors from library
    if isInteger(final_result):
        parseErrorCode(final_result, descriptorType)
    else:
        return final_result;
    
def extractDescriptorFromData(descriptorType, image_bytes, parameters):
    # ------------ CHECK ARGUMENTS ------------

    if descriptorType == None or type(descriptorType) != int:
        raise PyMpeg7Exception("Descriptor type is 'None' or is not type 'int'", 1)

    if image_bytes == None or type(image_bytes) != bytes:
        raise PyMpeg7Exception("Image buffer is 'None' or is not type 'bytes'", 2)

    if parameters == None or type(parameters) != list:
        raise PyMpeg7Exception("Python error: Parameters are 'None' or are not type 'list'", 4)

    # ------------ CONVERT ARGUMENTS ------------  
        
    # Convert image byte array to unsigned char *
    size = len(bytearray(image_bytes))
    imgBuffer = toUnsignedCharPointer(image_bytes, size) 

    # Convert parameters to const char **
    parameters = toConstCharPointerArray(parameters) 
    
    # ------------ SET  DATATYPES ----------------

    lib.extractDescriptorFromData.argtypes = [c_int, type(imgBuffer), c_int, type(parameters)]
    lib.extractDescriptorFromData.restype  = c_void_p

    # ------------ EXTRACT DESCRIPTOR  ------------

    # Call - extract and receive pointer to result
    result = lib.extractDescriptorFromData(descriptorType, imgBuffer, size, parameters)

    # Get result pointer value
    result_pointer_value = cast(result, c_char_p).value;

    # Free pointer from C
    freeResultPointer(result)

    # Decode received value
    final_result = result_pointer_value.decode("utf-8");

    # Detect possible errors from library
    if isInteger(final_result):
        parseErrorCode(final_result, descriptorType)
    else:
        return final_result;

def getDistance(xml1, xml2, parameters):
    # ------------ CHECK ARGUMENTS ------------
    if xml1 == None or type(xml1) != str:
        raise PyMpeg7Exception("xml1 is None or is not type 'str'", 1)

    if xml2 == None or type(xml2) != str:
        raise PyMpeg7Exception("xml2 is None or is not type 'str'", 2)

    if parameters == None or type(parameters) != list:
        raise PyMpeg7Exception("Python error: Parameters are 'None' or are not type 'list'", 3)

    # ------------ CONVERT ARGUMENTS ------------ 

    # Convert xml arguments to const char * :
    xml_1 = toConstCharPointer(xml1);
    xml_2 = toConstCharPointer(xml2);

    # Convert parameters to const char **
    parameters = toConstCharPointerArray(parameters) 

    # ------------ SET DATATYPES ----------------

    lib.getDistance.argtypes = [c_char_p, c_char_p]
    lib.getDistance.restype  = c_void_p

    # ------------ GET DISTANCE  ----------------

    # Call - get distance and receive pointer to result
    result = lib.getDistance(xml_1, xml_2, parameters)

    # Get result pointer value
    result_pointer_value = cast(result, c_char_p).value;

    # Free pointer from C
    freeResultPointer(result)

    # Decode received value
    final_result = result_pointer_value.decode("utf-8");

    # Detect possible errors from library
    if isInteger(final_result):
        parseErrorCode(final_result)
    else:
        return float(final_result)

def freeResultPointer(ptr):
    lib.freeResultPointer.argtypes = [c_void_p]
    lib.freeResultPointer.restype = None

    lib.freeResultPointer(ptr)

# ----------------------- MAIN CONVERSION METHODS ---------------------
'''
Since Python 3.x text literal is unicode object. 
In that case we have to use byte-string literal e.g. b'text'
or bytes(string_sequence, encoding='utf-8')
'''

# Return const char * from string
def toConstCharPointer(text):
    '''
    # For future implementation:
        if sys.version_info[0] < 3: 
            return py2ToConstCharPointer(text); # Python 2.x conversion function
        else:
            return py3ToConstCharPointer(text); # Python 3.x conversion function
    '''
    return py3ToConstCharPointer(text);

# Return const char ** from list of strings
def toConstCharPointerArray(string_list):
    '''
    # For future implementation:
    if sys.version_info[0] < 3:
        return py2ToConstCharPointerArray(string_list); # Python 2.x conversion function
    else:
        return py3ToConstCharPointerArray(string_list); # Python 3.x conversion function
    '''
    return py3ToConstCharPointerArray(string_list);

def toUnsignedCharPointer(text, size):
    '''
    # For future implementation:
    if sys.version_info[0] < 3: 
        return py2ToUnsignedCharPointer(text, size); # Python 2.x conversion function
    else:
        return py3ToUnsignedCharPointer(text, size); # Python 3.x conversion function
    '''
    return py3ToUnsignedCharPointer(text, size);

# ----------------------- PYTHON 3.x ----------------------------------

def py3ToConstCharPointer(text):
    # Create single byte string from given string
    byte_string = bytes(text, encoding='utf-8')
    # Create const char * variable from byte string
    constCharPointer = (c_char_p * 1)(byte_string)
    # Return created variable
    return constCharPointer[0]

def py3ToConstCharPointerArray(string_list):
    # Convert each string to byte string
    list_bytes = list();

    for i in range (0, len(string_list)):
        current_string = string_list[i]
        current_byte_string =  bytes(current_string, encoding='utf-8')
        list_bytes.append(current_byte_string)

    # Create const char ** variable from list of bytes (+1 size for nullpointer)
    constCharPointerArray = (c_char_p * (len(list_bytes) + 1))(*list_bytes)

    # Append NULL pointer at the end
    constCharPointerArray[len(list_bytes)] = None;

    # Return created variable
    return constCharPointerArray

def py3ToUnsignedCharPointer(image_bytes, size):
    return cast(image_bytes, POINTER(c_ubyte * size))[0]

# ----------------------- OTHER ---------------------------------------

'''
 Simple dowloading method. It can be modified for user needs.
'''
def simpleDownload(url, chunk_size=1024*8):
    print("Downloading: '{}'" .format(url))
    # Make request
    site = urllib.request.urlopen(url)
    # Get size
    total_size = int(site.info()['Content-Length'])
    # Prepare frame for data
    frame = bytes()
    # Download data chunk by chunk and append each one to frame
    print()
    downloaded = 0
    while True:
        chunk = site.read(chunk_size)
    
        if not chunk: 
            break

        downloaded += len(chunk)
        frame += chunk

        progress = str(math.floor( (downloaded / total_size) * 100 )) + "%"
        print(progress, end=' \r')

    print()
    return frame 

def isInteger(result):
    return re.match("^(\d+)$", result, flags = 0)

def parseErrorCode(result, descriptor_type=0):
    integer_result = int(result);
 
    if (integer_result <= 106):
        raise LibraryException(library_error_messages[integer_result], integer_result)
    else:
        raise LibraryException("Unrecognized library exception", -1)

# ----------------------- EXCEPTIONS ---------------------
class PyMpeg7Exception(Exception):
    def __init__(self, message, errors = -1):
        super(PyMpeg7Exception, self).__init__(message)
        self.errors = errors

class LibraryException(Exception):
    def __init__(self, message, errors = -1):
        super(LibraryException, self).__init__(message)
        self.errors = errors

pympeg7_exception_messages = {
    
}

library_error_messages = {
1:   "Main error - Parameters pointer is NULL",
2:   "Main error - Unrecognized descriptor type",
3:   "Main error - Image cannot be opened",
4:   "Main error - Image has unsupported channels count",
5:   "Main error - Extraction result is NULL",
6:   "Main error - Message from extraction result is NULL",
7:   "Main error - One of passed xmls is NULL",
8:   "Main error - Error while parsing one of xmls",
9:   "Main error - Mpeg7 element could not be found in one of xmls",
10:  "Main error - DescriptionUnit element could not be found in one of xmls",
11:  "Main error - Descriptor element could not be found in one of xmls",
12:  "Main error - xsi::type attribute of Descriptor element could not be found in one of xmls",
13:  "Main error - One of descriptor type is not recognized",
14:  "Main error - Descriptor types are not equal",
15:  "Main error - Error while reading at least one of two descriptors from descriptor element",
16:  "Main error - Distance calculation result is NULL",
17:  "Main error - Message from distance calculation is NULL",
18:  "Main error - Descriptor type passed from Java is NULL",
19:  "Main error - Image passed from Java is NULL (path or array of file data)",
20:  "Main error - Parameters array passed from Java is NULL",
21:  "Main error - Passed xml string from Java is NULL",
22:  "Color Layout - Params are NULL",
23:  "Color Layout - Number of parameters is wrong",
24:  "Color Layout - Wrong name for parameter",
25:  "Color Layout - Wrong value for parameter",
26:  "Color Layout - Could not find some dc nodes in XML",
27:  "Color Layout - Could not find some ac nodes in XML",
28:  "Color Layout - Wrong number of coefficients in XML attribute",
29:  "Color Layout - Xmls have different coefficients number",
30:  "Color Structure - Params are NULL",
31:  "Color Structure - Number of parameters is wrong",
32:  "Color Structure - Wrong name for parameter",
33:  "Color Structure - Wrong value for parameter",
34:  "Color Structure - Allocation error for method SetSize",
35:  "Color Structure - Size error in SetSize function",
36:  "Color Structure - Out of bounds  while setting element",
37:  "Color Structure - Wrong element access",
38:  "Color Structure - GetBinSize error",
39:  "Color Structure - GetBinSize out of bounds",
40:  "Color Structure - UnifyBins error",
41:  "Color Structure - Distance is wrong",
42:  "Color Structure - Wrong xml ColorQuantSize value",
43:  "Color Structure - Cannot read value for <Values>",
44:  "Color Structure - <Values> node not found in xml",
45:  "Color Structure - Values coefficients wrong quantity",
46:  "Color Structure - Descriptors size not equal while comparing",
47:  "Color Structure - QuantAmplNonLinear totalLevels error",
48:  "Color Structure - QuantAmplNonLinear normalize error",
49:  "Color Structure - Error building transform table",
50:  "Color Structure - Error in GetColorQuantSpace",
51:  "Color Structure - Slide histogram allocation error",
52:  "CTBrowsing - Params are NULL",
53:  "CTBrowsing - XML cannot find <BrowsingCategory> element",
54:  "CTBrowsing - XML unrecognized <BrowsingCategory> value",
55:  "CTBrowsing - XML cannot find <SubRangeIndex> element",
56:  "CTBrowsing - Wrong value for browsing category while generating XML",
57:  "Dominant Color - Params are NULL",
58:  "Dominant Color - Number of parameters is wrong",
59:  "Dominant Color - Wrong name for parameter",
60:  "Dominant Color - Wrong value for parameter",
61:  "Dominant Color - <Percentage> element missing in XML",
62:  "Dominant Color - <Index> element missing in XML",
63:  "Dominant Color - Cannot read percentage value",
64:  "Dominant Color - Cannot read index value",
65:  "Dominant Color - Cannot read variance value",
66:  "Scalable Color - Params are NULL",
67:  "Scalable Color - Number of parameters is wrong",
68:  "Scalable Color - Wrong name for parameter",
69:  "Scalable Color - Wrong value for parameter",
70:  "Scalable Color - Coefficients read/write and compare error",
71:  "Scalable Color - NumberOfBitplanesDiscarded read/write error",
72:  "Scalable Color - <Coefficients> node missing in XML",
73:  "Scalable Color - Coefficients compare error when calculating distance (after first check)",
74:  "Region Shape - Wrong number of coefficients in XML",
75:  "Region Shape - <MagnitudeOfART> element missing from XML",
76:  "Region Shape - GetElement wrong element access",
77:  "Contour Shape - <GlobalCurvature> element missing in XML",
78:  "Contour Shape - <HighestPeak> element missing from XML",
79:  "Contour Shape - XML <GlobalCurvature> quantity of values wrong",
80:  "Contour Shape - XML <PrototypeCurvature> quantity of values wrong",
81:  "Contour Shape - Descriptor distance wrong value",
82:  "Homogeneous Texture - Params are NULL",
83:  "Homogeneous Texture - Number of parameters is wrong",
84:  "Homogeneous Texture - Wrong name for parameter",
85:  "Homogeneous Texture - Wrong value for parameter",
86:  "Homogeneous Texture - Analyzed image is smaller than 128 x 128 pix",
87:  "Homogeneous Texture - <StandardDeviation> element missing from XML",
88:  "Homogeneous Texture - <Average> element missing from XML",
89:  "Homogeneous Texture - <Energy> element missing from XML",
90:  "Homogeneous Texture - <EnergyDeviation> missing from XML",
91:  "Homogeneous Texture - Coefficients descriptor comparison error",
92:  "Texture Browsing - Params are NULL",
93:  "Texture Browsing - Wrong name for parameter",
94:  "Texture Browsing - Wrong value for parameter",
95:  "Texture Browsing - Wrong component value in XML generation",
96:  "Texture Browsing - <Regularity> element missing from XML",
97:  "Texture Browsing - <Direction> element missing from XML",
98:  "Texture Browsing - <Scale> element missing from XML",
99:  "Texture Browsing - Wrong values in XML",
100: "Texture Browsing - Allocation matrix error",
101: "Texture Browsing - Projection file read error",
102: "Texture Browsing - Projection computation error",
103: "Texture Browsing - Cannot open projection file",
104: "Texture Browsing - Number of parameters is wrong",
105: "Edge Histogram - BinCounts size in XML",
106: "Edge Histogram - <BinCounts> element missing from XML",
}

'''

###############################################################
# TEST
###############################################################

url1 = "http://orig08.deviantart.net/9595/f/2008/167/f/3/a_somewhat_strange_cat_by_brando94.jpg"
url2 = "https://upload.wikimedia.org/wikipedia/en/6/6f/Yoda_Attack_of_the_Clones.png"

empty_params = list([""]);

scalableColorExtractionParams1 = list(["NumberOfCoefficients", "256", "NumberOfBitplanesDiscarded", "0"])
scalableColorExtractionParams2 = list(["NumberOfCoefficients", "256", "NumberOfBitplanesDiscarded", "0"])
scalableColorDistanceParams1 = list([])

data1 = simpleDownload(url1, chunk_size=512);
data2 = simpleDownload(url2, chunk_size=512);

try:
    xml1     = extractDescriptorFromData(DescriptorType.SCALABLE_COLOR_D, data1, scalableColorExtractionParams1)
    xml2     = extractDescriptorFromData(DescriptorType.SCALABLE_COLOR_D, data2, scalableColorExtractionParams2)
    distance = getDistance(xml1, xml2, scalableColorDistanceParams1)

    print(xml1)
    print()
    print(xml2)
    print()
    print(distance)
except (PyMpeg7Exception, LibraryException) as e:
    print("Exception: {} - code: {}" .format(str(e),str(e.errors)))
'''
