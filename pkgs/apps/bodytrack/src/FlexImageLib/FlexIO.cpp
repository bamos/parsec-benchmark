//-----------------------------------------------------------------------------
//    ____ _               
//   | ___| |   ___  _  _  
//   | ___| |  / __)\ \/ / 
//   | |  | |_| ___) |  |
//   |_|  \___|\___)/_/\_\ Image Library  Intel Corporation
//		
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
// file :		FlexIO.h 
// author :		Scott Ettinger - scott.m.ettinger@intel.com
// description: Functions for reading and writing image data
//				
// modified :		Christian Bienia - cbienia@cs.princeton.edu (Support for endianness)
//-----------------------------------------------------------------------------

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "FlexIO.h"

//Detect endianness of this machine
bool isLittleEndian() {
  union {
    WORD word;
    BYTE byte;
  } endian_test;

  endian_test.word = 0x00FF;
  return (endian_test.byte == 0xFF);
}

//Invert the byte order of a 16-bit word
WORD swap_16(WORD x) {
  union {
    WORD x_16;
    BYTE x_8[2];
  } mem_array;
  BYTE temp;

  mem_array.x_16 = x;
  temp = mem_array.x_8[0];
  mem_array.x_8[0] = mem_array.x_8[1];
  mem_array.x_8[1] = temp;

  return mem_array.x_16;
}

//Invert the byte order of a 32-bit word
DWORD swap_32(DWORD x) {
  union {
    DWORD x_32;
    WORD x_16[2];
    BYTE x_8[4];
  } mem_array;
  BYTE temp;

  mem_array.x_32 = x;
  //swap outer bytes
  temp = mem_array.x_8[0];
  mem_array.x_8[0] = mem_array.x_8[3];
  mem_array.x_8[3] = temp;
  //swap inner bytes
  temp = mem_array.x_8[1];
  mem_array.x_8[1] = mem_array.x_8[2];
  mem_array.x_8[2] = temp;

  return mem_array.x_32;
}

//Convert bitmap file header from file to memory byte order or back
void ConvertBmfh(BITMAPFILEHDR *bmfh) {
  if(!isLittleEndian()) {
    bmfh->bfType = swap_16(bmfh->bfType);
    bmfh->bfSize = swap_32(bmfh->bfSize);
    bmfh->bfReserved1 = swap_16(bmfh->bfReserved1);
    bmfh->bfReserved2 = swap_16(bmfh->bfReserved2);
    bmfh->bfOffBits = swap_32(bmfh->bfOffBits);
  }
}

//Convert bitmap info header from file to memory byte order or back
void ConvertBmih(BITMAPINFOHDR *bmih) {
  if(!isLittleEndian()) {
    bmih->biSize = swap_32(bmih->biSize);
    bmih->biWidth = swap_32(bmih->biWidth);
    bmih->biHeight = swap_32(bmih->biHeight);
    bmih->biPlanes = swap_16(bmih->biPlanes);
    bmih->biBitCount = swap_16(bmih->biBitCount);
    bmih->biCompression = swap_32(bmih->biCompression);
    bmih->biSizeImage = swap_32(bmih->biSizeImage);
    bmih->biXPelsPerMeter = swap_32(bmih->biXPelsPerMeter);
    bmih->biYPelsPerMeter = swap_32(bmih->biYPelsPerMeter);
    bmih->biClrUsed = swap_32(bmih->biClrUsed);
    bmih->biClrImportant = swap_32(bmih->biClrImportant);
  }
}


//Load an 8-bit grayscale .BMP file
bool FlexLoadBMPGray(const char *file, FlexImage<Im8u,1> &img) 
{
    FILE *in;
	BITMAPFILEHDR bmfh;
	BITMAPINFOHDR bmih;	

	in=fopen(file,"rb");
	if(in == NULL)
		return(false);
        //WARNING: Extra padding in bmfh causes erroneous reading into all but first field of structure
	fread(&bmfh,BFHSIZE,1,in);							//read BMP header
        ConvertBmfh(&bmfh);
	if(bmfh.bfType != 19778)							//check for valid BMP file
		return(false);
	fread(&bmih,BIHSIZE,1,in);							//read info header
        ConvertBmih(&bmih);
	if(bmih.biBitCount != 8) 
		return(false);									//only read 8 bit images
	img.Reallocate(bmih.biWidth, abs(bmih.biHeight));	//allocate image to size
	RGBA tmp;
	for(int i = 0; i < (int)bmih.biClrUsed; i++)		//skip color info
		fread(&tmp, sizeof(RGBA), 1, in);

	int padWidth = bmih.biWidth;
	while(padWidth%4) padWidth++;
	int dir = 1, yv = 0;
	if(bmih.biHeight > 0)
	{	dir = -1;
		yv = img.Height() - 1;
	}
	for(int y = 0; y < img.Height(); y++ )				//read in pixel data
	{	char tmp;
		fread(&img(0,yv), 1, img.Width(), in);
		yv += dir;
		for(int i = 0; i < padWidth - img.Width(); i++)	//skip over pad bytes
			fread(&tmp, 1, 1, in);
	}
	fclose(in);
	return(true);
}

bool FlexLoadBMPColor(const char *file, FlexImage<Im8u,3> &img) 
{
    FILE *in;
	BITMAPFILEHDR bmfh;
	BITMAPINFOHDR bmih;	

	in=fopen(file,"rb");
	if(in == NULL)
		return(false);	
        //WARNING: Extra padding in bmfh causes erroneous reading into all but first field of structure
	fread(&bmfh,BFHSIZE,1,in);							//read BMP header
        ConvertBmfh(&bmfh);
	if(bmfh.bfType != 19778)							//check for valid BMP file
		return(false);
	fread(&bmih,BIHSIZE,1,in);							//read info header
        ConvertBmih(&bmih);
	if(bmih.biBitCount != 24) 
		return(false);									//only read 8 bit images
	img.Reallocate(bmih.biWidth, abs(bmih.biHeight));	//allocate image to size

	RGBA tmp;
	for(int i = 0; i < (int)bmih.biClrUsed; i++)		//skip color info
		fread(&tmp, sizeof(RGBA), 1, in);

	int padWidth = 3 * bmih.biWidth;
	while(padWidth%4) padWidth++;						
	int dir = 1, yv = 0;
	if(bmih.biHeight > 0)
	{	dir = -1;
		yv = img.Height() - 1;
	}
	for(int y = 0; y < img.Height(); y++ )				//read in pixel data
	{	char tmp;
		fread(&img(0,yv), 1, img.Width() * 3, in);
		yv += dir;
		for(int i = 0; i < padWidth - img.Width() * 3; i++)	//skip over pad bytes
			fread(&tmp, 1, 1, in);
	}
	fclose(in);
	return(true);
}

bool FlexLoadBMP8u(const char *file, FlexImage<Im8u,3> &img) 
{
    FILE *in;
	BITMAPFILEHDR bmfh;
	BITMAPINFOHDR bmih;	

	in=fopen(file,"rb");
	if(in == NULL)
		return(false);
        //WARNING: Extra padding in bmfh causes erroneous reading into all but first field of structure
	fread(&bmfh,BFHSIZE,1,in);							//read BMP header
        ConvertBmfh(&bmfh);
	if(bmfh.bfType != 19778)							//check for valid BMP file
		return(false);
	fread(&bmih,BIHSIZE,1,in);							//read info header
        ConvertBmih(&bmih);
	if(bmih.biBitCount == 8)	
	{	fclose(in);										//load as grayscale
		FlexImage<Im8u,1> tmp;
		FlexLoadBMPGray(file, tmp);
		img.ReallocateNE(tmp.Width(), tmp.Height());
		FlexCopyC1CM(tmp, img, 0);						//duplicate to all 3 image planes
		FlexCopyC1CM(tmp, img, 1);
		FlexCopyC1CM(tmp, img, 2);
		return(true);
	}
	if(bmih.biBitCount == 24)
	{	fclose(in);
		FlexLoadBMPColor(file, img);
		return(true);
	}
	fclose(in);
	return(false);										//only read 8 bit images
}

bool FlexLoadBMP8u(const char *file, FlexImage<Im8u,1> &img) 
{
    FILE *in;
	BITMAPFILEHDR bmfh;
	BITMAPINFOHDR bmih;	

	in=fopen(file,"rb");
	if(in == NULL)
		return(false);
        //WARNING: Extra padding in bmfh causes erroneous reading into all but first field of structure
	fread(&bmfh,BFHSIZE,1,in);							//read BMP header
        ConvertBmfh(&bmfh);
	if(bmfh.bfType != 19778)							//check for valid BMP file
		return(false);
	fread(&bmih,BIHSIZE,1,in);							//read info header
        ConvertBmih(&bmih);
	if(bmih.biBitCount == 8)	
	{	fclose(in);										//load as grayscale
		return FlexLoadBMPGray(file, img);
	}
	if(bmih.biBitCount == 24)
	{	fclose(in);
		FlexImage<Im8u,3> tmp;
		bool ok = FlexLoadBMPColor(file, tmp);			//load as color image
		if(ok)
		{	img.ReallocateNE(tmp.Width(), tmp.Height());
			FlexRGBToGray(tmp, img, false);					//convert to grayscale
		}
		return ok;
	}
	fclose(in);
	return(false);										//only read 8 bit images
}

