/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/** @file Image.h
* @brief Class for loading, storing and distributing image data for MPEG-7 descriptor extraction
*
* Table containg information, which channels are used by each descriptor extration algorithm:
*
* type of descriptor  | used channels
* ------------------- | -----------------------------------------------
* Color Layout        | R, G, B, A separated
* Color Structure     | R, G, B, A separated
* CT Browsing         | RGB single array
* Dominant Color      | RGB single array, A - separated
* Scalable Color      | RGB single array
* Region Shape        | Grayscale average single array
* Contour Shape       | Grayscale average single array
* Homogeneous Texture | Grayscale average single array, A - separated
* Edge Histogram      | R, G, B, A separated
* Texture Browsing    | Grayscale average single array, A - separated
*
* Images supported by libmpeg7pw:
*
* Gray, GrayAlpha, RGB, RGBA
*
* Where following assumptions are made:
*
*    type   | description
* --------- | -----------------------------------------------
* Gray      | image, which has 1 channel containing grayscale 8 bit image pixel values
* GrayAlpha | image, which has 2 channels containing grayscale 8 bit image pixel values and transparency values
* RGB       | 3 channel image with R, G and B  8 bit components
* RGBA      | 4 channel image with R, G, B and transparecy 8 bit components
* 
* OpenCV supports following file formats (as it is described in OpenCV 3.1.0 online documentation: \n
* http://docs.opencv.org/master/d4/da8/group__imgcodecs.html#gsc.tab=0.

* File type                 | extensions, comments
* ------------------------- | ----------------------------------------------------------------------------------------
* Windows bitmaps           | *.bmp, *.dib
* JPEG files                | *.jpeg, *.jpg, *.jpe - on Linux it is important to install "libjpeg-dev" library
* JPEG 2000 files           | *.jp2 - on Linux it is important to install "libjasper-dev" library
* Portable Network Graphics | *.png - on Linux it is important to install "libpng-dev" library
* WebP                      | *.webp
* Portable image format     | *.pbm, *.pgm, *.ppm *.pxm, *.pnm
* Sun rasters               | *.sr, *.ras
* TIFF files                | *.tiff, *.tif - on Linux it is important to install "libtiff-dev" library
* OpenEXR Image files       | *.exr
* Radiance HDR              | *.hdr, *.pic
* 
* File formats are detected by images content (not extension).
* When color images are loaded, they are stored in BGRA order (indices: 0, 1, 2, 3)
*
*  @author Krzysztof Lech Kucharski */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#pragma once

#include <iostream>
#include <memory>
#include <stdio.h>
#include <malloc.h>
#include <vector>

/** @brief Image load mode */
enum LoadMode {
	IMAGE_UNCHANGED = 1, //!< image is loaded in unchanged way
	IMAGE_COLOR     = 2, //!< image is loaded, as it has 3 channels (R,G,B)
	IMAGE_GRAYSCALE = 3, //!< image is loaded in grayscale, OpenCV is loading grayscale using luminosity method (not average)
};

/** @brief Image grayscale load mode \n\n
* MPEG7 XM library uses average grayscale mode.                \n
* It makes results slightly different, than luminosity method  \n
* In that case to get average grayscale data, image first is   \n
* loaded normally and transformed after that, when needed.     \n
* If an image is Grayscale (1 channel), returns index 0  */
enum GrayscaleMode {
    GRAYSCALE_LUMINOSITY = 1, //!< Luminosity method for accessing grayscale values
    GRAYSCALE_AVERAGE    = 2  //!< Average method for accessing grayscale values
};

class Image {
	private:
		unsigned char* imageData;

		int imageWidth;					  // Image width
		int imageHeight;			      // Image height
		int imageSize;					  // Image size (width * height)
		int totalImageSize;				  // Total image size (number of channels * imageSize)
		bool transparencyPresent = false; // Variable indicating presence of alpha (transparency) channel

		int maxAlpha = -1;				  // Maximum value of alpha channel
		int minAlpha = -1;				  // Minimum value of alpha channel

		int channels;
		int depth;
	public:
		Image();		                  // Constructor creating empty image object				  

        /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
        /* Image loading
        /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

		// Method loading image decoding from 'data' array with mode specified in decode_mode
        void load(unsigned char * data, int size, LoadMode decode_mode);
        // Method loading image located at 'filename' path with mode specified in load_mode
        void load(const char * filename, LoadMode load_mode);
		
        /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
        /* Basic functions
        /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

        int getChannels();

        /** @return int - image width property in pixels */
		int getWidth();

        /** @return int - image height property in pixels */
		int getHeight();

        /** @return int - image size (Width * height) property in pixels */
		int getSize();

        /** @return int - image total size (Width * height * number of channels) in pixels */
		int getTotalSize();

        /** @return bool - image transparency presence indicator */
		bool getTransparencyPresent();

        /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
		/* Channels distribution for descriptor extraction methods:
		/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

        /** @brief
        * Method returning G (green mask) channel for an image.       \n\n
        * If an image is RGBA (4 channels), returns index 3           \n
        * If an image is RGB (3 channels), returns index 2            \n
        * If an image is GrayscaleAlpha (2 channels), returns index 1 \n
        * If an image is Grayscale (1 channel), returns index 0       \n
        *
        * @return unsigned char * - pointer to mask array */

		unsigned char * getChannel_R();

        /** @brief
        * Method returning G (green mask) channel for an image.       \n\n
        * If an image is RGBA (4 channels), returns index 2           \n
        * If an image is RGB (3 channels), returns index 1            \n
        * If an image is GrayscaleAlpha (2 channels), returns index 1 \n
        * If an image is Grayscale (1 channel), returns index 0       \n
        *
        * @return unsigned char * - pointer to mask array */

		unsigned char * getChannel_G();

        /** @brief
        * Method returning B (blue mask) channel for an image.        \n\n
        * If an image is RGBA (4 channels), returns index 1           \n
        * If an image is RGB (3 channels), returns index 0            \n
        * If an image is GrayscaleAlpha (2 channels), returns index 0 \n
        * If an image is Grayscale (1 channel), returns index 0       \n
        * 
        * @return unsigned char * - pointer to mask array */

		unsigned char * getChannel_B();

        /** @brief
        * Method returning A (transparency mask) channel for an image. \n\n
        * If an image is RGBA (4 channels), return index 0             \n
        * If an image is GrayscaleAlpha (2 channels), returns index 0  \n
        *
        * @return unsigned char * - pointer to mask array */

		unsigned char * getChannel_A();

        /** @brief
        * Default method returning Gray channel for an image.                     \n
		If an image is RGBA or RGB, returns luminosity method gray calculated     \n
        from r g b components. Otherwise - gray channel from 2 channel AlphaGray  \n
		or gray channel values from 1 channel Grayscale.                          \n
        *
        * @return unsigned char * - pointer to grayscale array */

        unsigned char * getGray();

        /** @brief
        * Method returning grayscale channel based on algorithm mode. . \n\n
        * GRAYSCALE_LUMINOSITY - luminosity method                      \n
        * GRAYSCALE_AVERAGE - average mathod                            \n
        *
        * @return unsigned char * - pointer to mask array */

		unsigned char * getGray(GrayscaleMode mode);

        /** @brief
        * Method returning RGB array (continous) for an image. \n\n
        * If an image is RGB, returns all indices
        * If an image is RGBA (4 channels), returns first 3 indices
        *
        * @return unsigned char * - pointer to array */

		unsigned char * getRGB();

        /** @brief
        * Method returning RGBA array (continous) for an image. \n\n
        * If an image is RGBA returns all indices                \n
        * Otherwise: NULL
        *
        * @return unsigned char * - pointer to array */

		unsigned char * getRGBA();

		/** @brief
		* Method returning RGB array (continous) for an image. \n\n
		* If an image is RGB, returns all indices
		* If an image is RGBA (4 channels), returns first 3 indices
		*
		* @return unsigned char * - pointer to array */
		unsigned char * getData();

		// Descructor
		~Image();
};

#pragma once
