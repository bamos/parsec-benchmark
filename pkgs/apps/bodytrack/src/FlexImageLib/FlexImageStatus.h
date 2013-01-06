//-----------------------------------------------------------------------------
//    ____ _               
//   | ___| |   ___  _  _  
//   | ___| |  / __)\ \/ / 
//   | |  | |_| ___) |  |
//   |_|  \___|\___)/_/\_\ Image Library  Intel Corporation
//		
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
// file :		FlexImageStatus.h 
// author :		Scott Ettinger - scott.m.ettinger@intel.com
// description: Flex Image image status functions
//				
// modified : 
//-----------------------------------------------------------------------------

#ifndef FLEXIMAGESTATUS_H
#define FLEXIMAGESTATUS_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <string>

/*
///Error status codes
typedef enum
{	
	//Warnings
	FlexStsSubImageTruncated = 1,	//A sub-image was truncated to fit within source image size	
	FlexStsNoError = 0,				//No Error

	//Errors
	FlexStsAllocationFail = -1,		//Unable to successfully allocate memory for image
	FlexStsInvalidSubImage = -2		//An invalid rectangle was given as a sub-image 

}FlexImageStatus;

typedef struct {
	FlexImageStatus code;
	std::string text;
}StatusPair;

std::string StatusText(FlexImageStatus status);
*/

#endif