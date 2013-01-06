/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmCPackZIPGenerator.h,v $
  Language:  C++
  Date:      $Date: 2012/03/29 17:21:10 $
  Version:   $Revision: 1.1.1.1 $

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef cmCPackZIPGenerator_h
#define cmCPackZIPGenerator_h

#include "cmCPackGenerator.h"

class cmCPackZIPGeneratorForward;

/** \class cmCPackZIPGenerator
 * \brief A generator for ZIP files
 */
class cmCPackZIPGenerator : public cmCPackGenerator
{
public:
  friend class cmCPackZIPGeneratorForward;
  cmCPackTypeMacro(cmCPackZIPGenerator, cmCPackGenerator);

  /**
   * Construct generator
   */
  cmCPackZIPGenerator();
  virtual ~cmCPackZIPGenerator();

protected:
  virtual int InitializeInternal();
  int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetOutputExtension() { return ".zip"; }

  int ZipStyle;
};

#endif
