from pympeg7 import *

url1 = "https://unsplash.it/200/300/?random"
url2 = "https://unsplash.it/200/300/?random"

empty_params = list([]);

params = list(["NumberOfCoefficients", "256", "NumberOfBitplanesDiscarded", "0"])

xml1 = extractDescriptor(DescriptorType.SCALABLE_COLOR_D, "results/force_awakens.jpg", params)

for i in range(0, 200):
    data = simpleDownload("https://unsplash.it/200/300/?random", chunk_size=256);
  
    try:
        xml2 = extractDescriptorFromData(DescriptorType.SCALABLE_COLOR_D, data, params)
        distance = getDistance(xml1, xml2, empty_params)

        print(distance)
    except (PyMpeg7Exception, LibraryException) as e:
        print("Exception: {} - code: {}" .format(str(e),str(e.errors)))
    if (distance < 500):
        filename = "results/" + str(i) + ".jpg"
        open(filename, 'wb').write(data)