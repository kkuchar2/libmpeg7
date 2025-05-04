#pragma once

#include <iostream>
#include <memory>
#include <stdio.h>
#include <malloc.h>
#include <vector>

enum LoadMode {
	IMAGE_UNCHANGED = 1,
	IMAGE_COLOR     = 2,
	IMAGE_GRAYSCALE = 3,
};

enum GrayscaleMode {
    GRAYSCALE_LUMINOSITY = 1,
    GRAYSCALE_AVERAGE    = 2
};

class Image {
	private:
		unsigned char* imageData;

		int imageWidth;
		int imageHeight;
		int imageSize;
		int totalImageSize;
		bool transparencyPresent = false;

		int maxAlpha = -1;
		int minAlpha = -1;

		int channels;
		int depth;
	public:
		Image();

        void load(unsigned char * data, int size, LoadMode decode_mode);
        void load(const char * filename, LoadMode load_mode);

        int getChannels();
		int getWidth();
		int getHeight();
		int getSize();
		int getTotalSize();
		bool getTransparencyPresent();

		unsigned char * getChannel_R();
		unsigned char * getChannel_G();
		unsigned char * getChannel_B();
		unsigned char * getChannel_A();
        unsigned char * getGray();
		unsigned char * getGray(GrayscaleMode mode);
		unsigned char * getRGB();
		unsigned char * getRGBA();
		unsigned char * getData();

		~Image();
};
