//-----------------------------------------------------------------------------
//    ____ _               
//   | ___| |   ___  _  _  
//   | ___| |  / __)\ \/ / 
//   | |  | |_| ___) |  |
//   |_|  \___|\___)/_/\_\ Image Library  
//		
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
// file :		FlexIO.h 
// author :		Scott Ettinger - scott.m.ettinger@intel.com
// description: Functions for reading and writing image data
//				
// modified :		Christian Bienia - cbienia@cs.princeton.edu (64-bit support for I/O)
//
// note :  THIS CODE MAY NOT BE FULLY PORTABLE.. SHOULD REWRITE
//
//-----------------------------------------------------------------------------

#ifndef FLEXIO_H
#define FLEXIO_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#if HAVE_STDINT_H
# include <stdint.h>
#endif //HAVE_STDINT_H

#include <stdio.h>

#include "FlexImage.h"
#include "FlexColor.h"

#pragma warning( disable : 4996)		//disable Microsoft compiler deprecation warnings 
#pragma warning( disable : 1786)		//disable Intel compiler deprecation warnings


//----------------------------- BMP I/O functions -----------------------------

//load BMP into any data type 1 or 3 channels (bitmap will be converted to proper number of channels if needed)
template<class T, int C>
bool FlexLoadBMP(const char *file, FlexImage<T,C> &img);

//Save image of any type 1 or 3 channel to a BMP file  (always saves as 24 bit image)
template<class T, int C>
bool FlexSaveBMP(const char *file, FlexImage<T,C> &img);


//---------------------------- Implementation ----------------------------------

#if HAVE_STDINT_H
typedef uint8_t             BYTE;
typedef uint16_t            WORD;
typedef uint32_t            DWORD;
typedef int32_t             LONG;
#else
//this is not 64-bit safe
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef long                LONG;
#endif //HAVE_STDINT_H


#define BFHSIZE 14
#define BIHSIZE 40

//#define BFHSIZE sizeof(BITMAPFILEHDR)
//#define BIHSIZE sizeof(BITMAPINFOHDR)

typedef struct tagBITMAPFILEHDR {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHDR;

typedef struct tagBITMAPINFOHDR{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHDR;

typedef struct tagRGBA {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBA;


//prototypes of byte order conversion functions
void ConvertBmfh(BITMAPFILEHDR *bmfh);
void ConvertBmih(BITMAPINFOHDR *bmih);


//prototypes of support functions
bool FlexLoadBMP8u(const char *file, FlexImage<Im8u,3> &img); 
bool FlexLoadBMP8u(const char *file, FlexImage<Im8u,1> &img); 

//Load BMP
template<class T, int C>
bool FlexLoadBMP(const char *file, FlexImage<T,C> &img)
{	
	FlexImage<Im8u,C> tmp;
	bool ok = FlexLoadBMP8u(file, tmp);
	img.ConvertFrom(tmp);
	return ok;
}

//Save either a 1 or 3 channel image to a BMP file  (always saves as 24 bit image)
template<int C>
bool FlexSaveBMP8u(const char *file, FlexImage<Im8u,C> &img)
{
	if(C != 1 && C != 3)				//must be 1 or 3 plane image
		return(false);

	//set up windows BMP headers
	BITMAPFILEHDR bmfh;
	BITMAPINFOHDR bmih;	
	bmfh.bfType = 19778;
	bmfh.bfSize = BIHSIZE + BFHSIZE + img.Width() * img.Height() * 3;
	bmfh.bfOffBits = BIHSIZE + BFHSIZE;
	bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
	bmih.biBitCount = 24;
	bmih.biSize = BIHSIZE;
	bmih.biWidth = img.Width();
	bmih.biHeight = img.Height();
	bmih.biPlanes = 1;
	bmih.biClrUsed = 0;
	bmih.biCompression = 0L;
	bmih.biSizeImage = img.Width() * img.Height() * 3;
	bmih.biYPelsPerMeter = bmih.biXPelsPerMeter = 0;
	bmih.biClrImportant = 0;

	//write headers
    FILE *f;
	f=fopen(file,"wb");
	if(f == NULL)
		return(false);
        ConvertBmfh(&bmfh);
        ConvertBmih(&bmih);
	fwrite(&bmfh.bfType, 2, 1, f);
	fwrite(&bmfh.bfSize, 4, 1, f);
	fwrite(&bmfh.bfReserved1, 2, 1, f);
	fwrite(&bmfh.bfReserved2, 2, 1, f);
	fwrite(&bmfh.bfOffBits, 4, 1, f);
	fwrite(&bmih, BIHSIZE, 1, f);

	//save image
	int line = img.Width() * 3;
	if((line % 4) != 0)
		line = int((img.Width() * 3) / 4) * 4 + 4;
	if(C == 3)														//24-bit image
		for(int i = img.Height() - 1; i >= 0; i--)
			fwrite(&img(0,i), line, 1, f);							//write each line of image (image is already padded)
	else															
		for(int y = img.Height() -1; y >= 0; y--)					//8-bit image
		{	for(int x = 0; x < img.Width(); x++)
				for(int i = 0; i < 3; i++)							//write pixel to each plane
					fwrite(&img(x,y), 1, 1, f);
			for(int i = 0; i < line - (img.Width() * 3); i++)	//pad to correct width 
				fwrite(&img(0,0), 1, 1, f);
		}

	fclose(f);
	return(true);

}

//Save image of any type to bmp of 1 or 3 channels
template<class T, int C>
bool FlexSaveBMP(const char *file, FlexImage<T,C> &img)
{	
	FlexImage<Im8u,C> tmp;
	tmp.ConvertFrom(img);
	return FlexSaveBMP8u(file, tmp);
}

#endif
