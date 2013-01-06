/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmDocumentationFormatterText.h,v $
  Language:  C++
  Date:      $Date: 2012/03/29 17:21:09 $
  Version:   $Revision: 1.1.1.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _cmDocumentationFormatterText_h
#define _cmDocumentationFormatterText_h

#include "cmStandardIncludes.h"

#include "cmDocumentationFormatter.h"

/** Class to print the documentation as plain text.  */
class cmDocumentationFormatterText : public cmDocumentationFormatter
{
public:
  cmDocumentationFormatterText();

  virtual cmDocumentationEnums::Form GetForm() const
                                      { return cmDocumentationEnums::TextForm;}

  virtual void PrintSection(std::ostream& os,
                    const cmDocumentationSection& section,
                    const char* name);
  virtual void PrintPreformatted(std::ostream& os, const char* text);
  virtual void PrintParagraph(std::ostream& os, const char* text);
  void PrintColumn(std::ostream& os, const char* text);
  void SetIndent(const char* indent);
protected:
  int TextWidth;
  const char* TextIndent;
};

#endif
