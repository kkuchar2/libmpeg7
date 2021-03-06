import java.io.IOException;

import com.ztv.mpeg7.lib.DescriptorType;
import com.ztv.mpeg7.lib.LibraryException;
import com.ztv.mpeg7.lib.javampeg7pw;

public class TestClass {
	public static void main(String[] args) {
		byte[] data1 = null;
		byte[] data2 = null;
		String libPathProperty = System.getProperty("java.library.path");
        System.out.println(libPathProperty);
		try {
			data1 = javampeg7pw.simpleDownload("http://img.lum.dolimg.com/v1/images/rey_bddd0f27.jpeg?region=0%2C24%2C1560%2C876&width=768", true);
			data2 = javampeg7pw.simpleDownload("http://i.onionstatic.com/avclub/5535/60/16x9/960.jpg", true);
		} catch (IOException e) {
			e.printStackTrace();
		}

		String xml1 = null;
		String xml2 = null;
		
		try {
			xml1 = javampeg7pw.extractDescriptorFromData(DescriptorType.SCALABLE_COLOR_D, data1, 
					new String[]{"NumberOfCoefficients", "256", "NumberOfBitplanesDiscarded", "0"});
			xml2 = javampeg7pw.extractDescriptorFromData(DescriptorType.SCALABLE_COLOR_D, data2, 
					new String[]{"NumberOfCoefficients", "256", "NumberOfBitplanesDiscarded", "0"});
		} catch (LibraryException e) {
			e.printStackTrace();
		}

		double distance = -1;

		try {
			distance = javampeg7pw.calculateDistance(xml1, xml2, new String[]{});
		} catch (LibraryException e) {
			e.printStackTrace();
		}
		System.out.println(xml1);
		System.out.println(xml2);
		System.out.println(distance);
	}
}
