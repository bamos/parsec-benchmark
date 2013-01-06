//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                  
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : TBBTypes.h
//  author : Scott Ettinger
//  description : types for TBB pipelining
//				  
//  modified : 
//--------------------------------------------------------------

#ifndef TBBTYPES_H
#define TBBTYPES_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "ImageMeasurements.h"

//typedefs for convenient movement of image sets
typedef std::vector<FlexImage8u > ImageSet;			
typedef std::vector<BinaryImage> BinaryImageSet;

//structure containing edge maps and foreground maps to be sent between pipeline stages
struct ImageSetToken {

	ImageSet edgeMaps;
	BinaryImageSet FGmaps;

};


#endif

