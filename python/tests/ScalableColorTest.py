from pympeg7 import *

################################### NETWORK TEST ################################### 

url1 = "http://orig08.deviantart.net/9595/f/2008/167/f/3/a_somewhat_strange_cat_by_brando94.jpg"
url2 = "http://www-huber.embl.de/EBImage/ExampleImages/lena-color.jpg"

data1 = simpleDownload(url1, chunk_size=512);
data2 = simpleDownload(url2, chunk_size=512);

print()

try:
    xml1     = extractDescriptorFromData(DescriptorType.SCALABLE_COLOR_D, data1, list(["NumberOfCoefficients", "256", "NumberOfBitplanesDiscarded", "0"]))
    xml2     = extractDescriptorFromData(DescriptorType.SCALABLE_COLOR_D, data2, list(["NumberOfCoefficients", "256", "NumberOfBitplanesDiscarded", "0"]))
    distance = getDistance(xml1, xml2, list())
    
except (PyMpeg7Exception, LibraryException) as e:
    print("Exception: {} - code: {}" .format(str(e),str(e.errors)))

print(xml1 + "\n")
print(xml2 + "\n")
print("Distance: %f \n" % distance)

################################### LOCAL TEST ################################### 

try:
    xml1     = extractDescriptor(DescriptorType.SCALABLE_COLOR_D, "images/lena_color.jpg", list(["NumberOfCoefficients", "256", "NumberOfBitplanesDiscarded", "0"]))
    xml2     = extractDescriptor(DescriptorType.SCALABLE_COLOR_D, "images/HanSolo.jpg", list(["NumberOfCoefficients", "256", "NumberOfBitplanesDiscarded", "0"]))
    distance = getDistance(xml1, xml2, list([]))
    
except (PyMpeg7Exception, LibraryException) as e:
    print("Exception: {} - code: {}" .format(str(e),str(e.errors)))

print(xml1 + "\n")
print(xml2 + "\n")
print("Distance: %f \n" % distance)