//-----------------------------------------------------------------------------
//    ____ _               
//   | ___| |   ___  _  _  
//   | ___| |  / __)\ \/ / 
//   | |  | |_| ___) |  |
//   |_|  \___|\___)/_/\_\ Image Library
//	
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
// file :		FlexDefs.h 
// author :		Scott Ettinger - scott.m.ettinger@intel.com
// description: Flex Image image library basic definitions
//				
// modified : 
//-----------------------------------------------------------------------------

#ifndef FLEXDEFS_H
#define FLEXDEFS_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif
#include <stdlib.h>
#include <cstdlib>
#include <cstring>

//------------------------------------- Data Types ---------------------------------------

#define Im8u    unsigned char
#define Im8s    char
#define Im16u   unsigned short int
#define Im16s   short int
#define Im32u   unsigned int
#define Im32s   int
#define Im32f	float
#define Im64f   double

//----------------------------- primitive structure definitions ---------------------------

class FIPoint {						//simple Point in integer space
public:
	int x;
    int y;
};

class FIPointf {					//Point in continuous 2d space
public:
	float x;
    float y;
};

class FISize {						//2D size 
public:
    int width;
    int height;
};

class FIRect {						//Rectangle
public:
	int x;
    int y;
    int width;
    int height;
};

//----------------------------- Error Status Codes -------------------------------

#define FIStatus int

typedef enum
{	
	//Warnings
	FlexStsSubImageTruncated = 1,	//A sub-image was truncated to fit within source image size	
	FlexStsNoError = 0,				//No Error

	//Errors
	FlexStsAllocationFail = -1,		//Unable to successfully allocate memory for image
	FlexStsInvalidSubImage = -2		//An invalid rectangle was given as a sub-image 

}FlexImageStatus;

#endif

