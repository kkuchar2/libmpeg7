package com.mpeg7.lib;

import com.mpeg7.mpeg7.libmpeg7;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.HashMap;

public class mpeg7 {
	private static final libmpeg7 lib = new libmpeg7();

	private static HashMap<Integer, String> libraryErrorCodesMap = new HashMap<>();

	static {
		initializeErrorCodeMaps();
	}

	private static final String integerPattern = "^(\\d+)$";

	public static String extractDescriptor(int desType, String imgPath, String[] params) throws LibraryException {
		String result = lib.extractDescriptor(desType, imgPath, params);

		if (isInteger(result)) {
			parseMainErrors(result);
		}
		return result;
	}

	public static String extractDescriptorFromURL(int desType, String url, String[] params, boolean logProgress) throws LibraryException, IOException {
		return lib.extractDescriptorFromData(desType, simpleDownload(url, logProgress), params);
	}

	public static String extractDescriptorFromData(int desType, byte[] data, String[] params) throws LibraryException {
		String result = lib.extractDescriptorFromData(desType, data, params);

		if (isInteger(result)) {
			parseMainErrors(result);
		}

		return result;
	}

	public static double calculateDistance(String xml1, String xml2, String[] params) throws LibraryException {
		String result = lib.calculateDistance(xml1, xml2, params);

		if (isInteger(result)) {
			parseMainErrors(result);
		}

		return Double.parseDouble(result.replace(",","."));
	}

	public static byte[] simpleDownload(String urlPath, boolean logProgress) throws IOException {
		System.out.println("Downloading '" + urlPath + "'\n");

		// Create URL
		URL url = new URL(urlPath);

		// Open connection
		HttpURLConnection httpConnection = (HttpURLConnection) (url.openConnection());

		// Get size of content
		long completeFileSize = httpConnection.getContentLength();

		// Download content to byte array
		BufferedInputStream in = new java.io.BufferedInputStream(httpConnection.getInputStream());
		ByteArrayOutputStream outputStream = new ByteArrayOutputStream();

		int CHUNK_SIZE = 1024 * 4;

		byte[] contentData;
		byte[] chunk = new byte[CHUNK_SIZE];

		int totalDownloadedBytes = 0;
		int currentDowloadedBytes = 0;
		double progress = 0.0;

		while ((currentDowloadedBytes = in.read(chunk, 0, CHUNK_SIZE)) >= 0) {
			outputStream.write(chunk, 0, currentDowloadedBytes);
			totalDownloadedBytes += currentDowloadedBytes;

			if (logProgress) {
				// Display progress
				progress = (float)totalDownloadedBytes/(float)completeFileSize * 100.0;
				System.out.printf("Progress: %.2f %% \n", progress);
			}
		}

		contentData = outputStream.toByteArray();

		outputStream.close();
		in.close();

		return contentData;
	}

	private static boolean isInteger(String result) {
		return result.matches(integerPattern);
	}


	private static void parseMainErrors(String result) throws LibraryException {
      int res = Integer.parseInt(result);

      if (res >= 1 && res <= 106) {
          throw new LibraryException(result + " " + libraryErrorCodesMap.get(res));
      }
    }

	private static void initializeErrorCodeMaps() {
		libraryErrorCodesMap.put(1,   "Main error - Parameters pointer is NULL");
		libraryErrorCodesMap.put(2,   "Main error - Unrecognized descriptor type");
		libraryErrorCodesMap.put(3,   "Main error - Image cannot be opened");
		libraryErrorCodesMap.put(4,   "Main error - Image has unsupported channels count");
		libraryErrorCodesMap.put(5,   "Main error - Extraction result is NULL");
		libraryErrorCodesMap.put(6,   "Main error - Message from extraction result is NULL");
		libraryErrorCodesMap.put(7,   "Main error - One of passed xmls is NULL");
		libraryErrorCodesMap.put(8,   "Main error - Error while parsing one of xmls");
		libraryErrorCodesMap.put(9,   "Main error - Mpeg7 element could not be found in one of xmls");
		libraryErrorCodesMap.put(10,  "Main error - DescriptionUnit element could not be found in one of xmls");
		libraryErrorCodesMap.put(11,  "Main error - Descriptor element could not be found in one of xmls");
		libraryErrorCodesMap.put(12,  "Main error - xsi::type attribute of Descriptor element could not be found in one of xmls");
		libraryErrorCodesMap.put(13,  "Main error - One of descriptor type is not recognized");
		libraryErrorCodesMap.put(14,  "Main error - Descriptor types are not equal");
		libraryErrorCodesMap.put(15,  "Main error - Error while reading at least one of two descriptors from descriptor element");
		libraryErrorCodesMap.put(16,  "Main error - Distance calculation result is NULL");
		libraryErrorCodesMap.put(17,  "Main error - Message from distance calculation is NULL");
		libraryErrorCodesMap.put(18,  "Main error - Descriptor type passed from Java is NULL");
		libraryErrorCodesMap.put(19,  "Main error - Image passed from Java is NULL (path or array of file data)");
		libraryErrorCodesMap.put(20,  "Main error - Parameters array passed from Java is NULL");
		libraryErrorCodesMap.put(21,  "Main error - Passed xml string from Java is NULL");
		libraryErrorCodesMap.put(22,  "Color Layout - Params are NULL");
		libraryErrorCodesMap.put(23,  "Color Layout - Number of parameters is wrong");
		libraryErrorCodesMap.put(24,  "Color Layout - Wrong name for parameter");
		libraryErrorCodesMap.put(25,  "Color Layout - Wrong value for parameter");
		libraryErrorCodesMap.put(26,  "Color Layout - Could not find some dc nodes in XML");
		libraryErrorCodesMap.put(27,  "Color Layout - Could not find some ac nodes in XML");
		libraryErrorCodesMap.put(28,  "Color Layout - Wrong number of coefficients in XML attribute");
		libraryErrorCodesMap.put(29,  "Color Layout - Xmls have different coefficients number");
		libraryErrorCodesMap.put(30,  "Color Structure - Params are NULL");
		libraryErrorCodesMap.put(31,  "Color Structure - Number of parameters is wrong");
		libraryErrorCodesMap.put(32,  "Color Structure - Wrong name for parameter");
		libraryErrorCodesMap.put(33,  "Color Structure - Wrong value for parameter");
		libraryErrorCodesMap.put(34,  "Color Structure - Allocation error for method SetSize");
		libraryErrorCodesMap.put(35,  "Color Structure - Size error in SetSize function");
		libraryErrorCodesMap.put(36,  "Color Structure - Out of bounds  while setting element");
		libraryErrorCodesMap.put(37,  "Color Structure - Wrong element access");
		libraryErrorCodesMap.put(38,  "Color Structure - GetBinSize error");
		libraryErrorCodesMap.put(39,  "Color Structure - GetBinSize out of bounds");
		libraryErrorCodesMap.put(40,  "Color Structure - UnifyBins error");
		libraryErrorCodesMap.put(41,  "Color Structure - Distance is wrong");
		libraryErrorCodesMap.put(42,  "Color Structure - Wrong xml ColorQuantSize value");
		libraryErrorCodesMap.put(43,  "Color Structure - Cannot read value for <Values>");
		libraryErrorCodesMap.put(44,  "Color Structure - <Values> node not found in xml");
		libraryErrorCodesMap.put(45,  "Color Structure - Values coefficients wrong quantity");
		libraryErrorCodesMap.put(46,  "Color Structure - Descriptors size not equal while comparing");
		libraryErrorCodesMap.put(47,  "Color Structure - QuantAmplNonLinear totalLevels error");
		libraryErrorCodesMap.put(48,  "Color Structure - QuantAmplNonLinear normalize error");
		libraryErrorCodesMap.put(49,  "Color Structure - Error building transform table");
		libraryErrorCodesMap.put(50,  "Color Structure - Error in GetColorQuantSpace");
		libraryErrorCodesMap.put(51,  "Color Structure - Slide histogram allocation error");
		libraryErrorCodesMap.put(52,  "CTBrowsing - Params are NULL");
		libraryErrorCodesMap.put(53,  "CTBrowsing - XML cannot find <BrowsingCategory> element");
		libraryErrorCodesMap.put(54,  "CTBrowsing - XML unrecognized <BrowsingCategory> value");
		libraryErrorCodesMap.put(55,  "CTBrowsing - XML cannot find <SubRangeIndex> element");
		libraryErrorCodesMap.put(56,  "CTBrowsing - Wrong value for browsing category while generating XML");
		libraryErrorCodesMap.put(57,  "Dominant Color - Params are NULL");
		libraryErrorCodesMap.put(58,  "Dominant Color - Number of parameters is wrong");
		libraryErrorCodesMap.put(59,  "Dominant Color - Wrong name for parameter");
		libraryErrorCodesMap.put(60,  "Dominant Color - Wrong value for parameter");
		libraryErrorCodesMap.put(61,  "Dominant Color - <Percentage> element missing in XML");
		libraryErrorCodesMap.put(62,  "Dominant Color - <Index> element missing in XML");
		libraryErrorCodesMap.put(63,  "Dominant Color - Cannot read percentage value");
		libraryErrorCodesMap.put(64,  "Dominant Color - Cannot read index value");
		libraryErrorCodesMap.put(65,  "Dominant Color - Cannot read variance value");
		libraryErrorCodesMap.put(66,  "Scalable Color - Params are NULL");
		libraryErrorCodesMap.put(67,  "Scalable Color - Number of parameters is wrong");
		libraryErrorCodesMap.put(68,  "Scalable Color - Wrong name for parameter");
		libraryErrorCodesMap.put(69,  "Scalable Color - Wrong value for parameter");
		libraryErrorCodesMap.put(70,  "Scalable Color - Coefficients read/write and compare error");
		libraryErrorCodesMap.put(71,  "Scalable Color - NumberOfBitplanesDiscarded read/write error");
		libraryErrorCodesMap.put(72,  "Scalable Color - <Coefficients> node missing in XML");
		libraryErrorCodesMap.put(73,  "Scalable Color - Coefficients compare error when calculating distance (after first check)");
		libraryErrorCodesMap.put(74,  "Region Shape - Wrong number of coefficients in XML");
		libraryErrorCodesMap.put(75,  "Region Shape - <MagnitudeOfART> element missing from XML");
		libraryErrorCodesMap.put(76,  "Region Shape - GetElement wrong element access");
		libraryErrorCodesMap.put(77,  "Contour Shape - <GlobalCurvature> element missing in XML");
		libraryErrorCodesMap.put(78,  "Contour Shape - <HighestPeak> element missing from XML");
		libraryErrorCodesMap.put(79,  "Contour Shape - XML <GlobalCurvature> quantity of values wrong");
		libraryErrorCodesMap.put(80,  "Contour Shape - XML <PrototypeCurvature> quantity of values wrong");
		libraryErrorCodesMap.put(81,  "Contour Shape - Descriptor distance wrong value");
		libraryErrorCodesMap.put(82,  "Homogeneous Texture - Params are NULL");
		libraryErrorCodesMap.put(83,  "Homogeneous Texture - Number of parameters is wrong");
		libraryErrorCodesMap.put(84,  "Homogeneous Texture - Wrong name for parameter");
		libraryErrorCodesMap.put(85,  "Homogeneous Texture - Wrong value for parameter");
		libraryErrorCodesMap.put(86,  "Homogeneous Texture - Analyzed image is smaller than 128 x 128 pix");
		libraryErrorCodesMap.put(87,  "Homogeneous Texture - <StandardDeviation> element missing from XML");
		libraryErrorCodesMap.put(88,  "Homogeneous Texture - <Average> element missing from XML");
		libraryErrorCodesMap.put(89,  "Homogeneous Texture - <Energy> element missing from XML");
		libraryErrorCodesMap.put(90,  "Homogeneous Texture - <EnergyDeviation> missing from XML");
		libraryErrorCodesMap.put(91,  "Homogeneous Texture - Coefficients descriptor comparison error");
		libraryErrorCodesMap.put(92,  "Texture Browsing - Params are NULL");
		libraryErrorCodesMap.put(93,  "Texture Browsing - Wrong name for parameter");
		libraryErrorCodesMap.put(94,  "Texture Browsing - Wrong value for parameter");
		libraryErrorCodesMap.put(95,  "Texture Browsing - Wrong component value in XML generation");
		libraryErrorCodesMap.put(96,  "Texture Browsing - <Regularity> element missing from XML");
		libraryErrorCodesMap.put(97,  "Texture Browsing - <Direction> element missing from XML");
		libraryErrorCodesMap.put(98,  "Texture Browsing - <Scale> element missing from XML");
		libraryErrorCodesMap.put(99,  "Texture Browsing - Wrong values in XML");
		libraryErrorCodesMap.put(100, "Texture Browsing - Allocation matrix error");
		libraryErrorCodesMap.put(101, "Texture Browsing - Projection file read error");
		libraryErrorCodesMap.put(102, "Texture Browsing - Projection computation error");
		libraryErrorCodesMap.put(103, "Texture Browsing - Cannot open projection file");
		libraryErrorCodesMap.put(104, "Texture Browsing - Number of parameters is wrong");
		libraryErrorCodesMap.put(105, "Edge Histogram - BinCounts size in XML");
		libraryErrorCodesMap.put(106, "Edge Histogram - <BinCounts> element missing from XML");
	}
}