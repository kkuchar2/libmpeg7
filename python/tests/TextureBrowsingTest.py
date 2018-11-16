from pympeg7 import *

################################### LOCAL TEST ################################### 

try:
    xml1     = extractDescriptor(DescriptorType.TEXTURE_BROWSING_D, "images/texture1.pgm", list(["layer", "1"]))
    xml2     = extractDescriptor(DescriptorType.TEXTURE_BROWSING_D, "images/texture1.pgm", list(["layer", "1"]))
    distance = getDistance(xml1, xml2, list([]))
    
except (PyMpeg7Exception, LibraryException) as e:
    print("Exception: {} - code: {}" .format(str(e),str(e.errors)))

print(xml1 + "\n")
print(xml2 + "\n")
print("Distance: %f \n" % distance)