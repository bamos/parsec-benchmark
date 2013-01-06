/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmDocumentationFormatterUsage.h,v $
  Language:  C++
  Date:      $Date: 2012/03/29 17:21:09 $
  Version:   $Revision: 1.1.1.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _cmDocumentationFormatterUsage_h
#define _cmDocumentationFormatterUsage_h

#include "cmDocumentationFormatterText.h"

/** Class to print the documentation as usage on the command line.  */
class cmDocumentationFormatterUsage : public cmDocumentationFormatterText
{
public:
  cmDocumentationFormatterUsage();

  virtual cmDocumentationEnums::Form GetForm() const
                                     { return cmDocumentationEnums::UsageForm;}

  virtual void PrintSection(std::ostream& os,
                    const cmDocumentationSection& section,
                    const char* name);
};

#endif
