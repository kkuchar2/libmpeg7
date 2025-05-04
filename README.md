# libmpeg7

Library implementation of MPEG-7 digital image descriptors calculation and comparison algorithms. This project was created as part of an engineer's degree thesis.

## About MPEG-7

MPEG-7 is a multimedia content description standard that provides a rich set of standardized tools to describe multimedia content. This library focuses on the visual descriptors specified in the standard, which can be used for content-based image retrieval, image classification, and similarity-based image search.

## Supported Descriptors

The library implements the following MPEG-7 visual descriptors:

### Color Descriptors
- **Dominant Color (ID: 1)** - Provides a compact description of the representative colors in an image or region
- **Scalable Color (ID: 2)** - A color histogram encoded using the Haar transform for scalable representation
- **Color Layout (ID: 3)** - Captures the spatial distribution of colors in an image
- **Color Structure (ID: 4)** - Identifies localized color distributions using a structuring element
- **CT Browsing (ID: 5)** - Color-Texture browsing descriptor

### Texture Descriptors
- **Homogeneous Texture (ID: 6)** - Characterizes texture regions in an image using frequency domain statistics
- **Texture Browsing (ID: 7)** - Browsing type descriptor for texture
- **Edge Histogram (ID: 8)** - Represents the spatial distribution of edges in an image

### Shape Descriptors
- **Region Shape (ID: 9)** - Describes shapes using Angular Radial Transform (ART)
- **Contour Shape (ID: 10)** - Describes shape contours using curvature scale-space representation

Each descriptor can be configured with specific parameters to adjust its behavior. The descriptor IDs can be used with the command line application and API calls to specify which descriptor to extract.

## Building the Library

### Prerequisites
- C++14 compatible compiler
- CMake (minimum version 3.5)
- Java Development Kit (JDK) for JNI support

### Build Instructions

The project includes a build script that makes compilation straightforward:

```bash
# Clone the repository
git clone https://github.com/yourusername/libmpeg7.git
cd libmpeg7

# Make the build script executable
chmod +x build.sh

# Build both debug and release versions
./build.sh
```

The build script supports the following commands:

- `./build.sh` - Builds both debug and release versions
- `./build.sh debug` - Builds only the debug version
- `./build.sh release` - Builds only the release version

After a successful build, the libraries and executables can be found in:
- Debug version: `build/x64/Debug/` or `build/x32/Debug/` (depending on your architecture)
- Release version: `build/x64/Release/` or `build/x32/Release/`

## Usage

### Command Line Application

The library comes with a command-line application (`mpeg7_app`) that allows you to extract descriptors from images:

```bash
./build/x64/Release/mpeg7_app <descriptor_type> <image_path> [param_name param_value ...]
```

Where:
- `<descriptor_type>` is a number from 1 to 10 representing the descriptor type:
  1. Dominant Color
  2. Scalable Color
  3. Color Layout
  4. Color Structure
  5. CT Browsing
  6. Homogeneous Texture
  7. Texture Browsing
  8. Edge Histogram
  9. Region Shape
  10. Contour Shape
- `<image_path>` is the path to the image file
- Optional parameter name-value pairs can be provided depending on the descriptor type

Example:
```bash
./build/x64/Release/mpeg7_app 3 image.jpg NumberOfYCoeff 64 NumberOfCCoeff 64
```

### Library Integration

#### C++ Integration

The library can be integrated into your projects either as a static or shared library. Include the main header file in your code:

```cpp
#include "Mpeg7.h"
```

#### Java Integration (JNI)

The library includes JNI bindings that allow it to be used from Java applications. The following methods are exposed:

```java
// Extract descriptor from an image file
public native String extractDescriptor(int descriptorType, String imagePath, String[] parameters);

// Extract descriptor directly from image data
public native String extractDescriptorFromData(int descriptorType, byte[] imageData, String[] parameters);

// Calculate distance between two descriptors
public native String calculateDistance(String descriptorXml1, String descriptorXml2, String[] parameters);
```

To use the JNI interface:
1. Ensure the shared library (.so on Linux, .dll on Windows) is in your library path
2. Load the library in your Java application using `System.loadLibrary("mpeg7")`
3. Create an instance of the class that implements these native methods
4. Call the methods with appropriate parameters

## Reporting Issues

If you encounter any errors or issues while using the library, please create an issue in this GitHub repository. When reporting issues, please include:

1. Your operating system and compiler version
2. The command or code that triggered the error
3. Full error message or stack trace if available
4. Steps to reproduce the issue

## Author

Krzysztof Kucharski