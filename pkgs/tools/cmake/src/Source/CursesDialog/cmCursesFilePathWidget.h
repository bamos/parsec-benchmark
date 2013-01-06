/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmCursesFilePathWidget.h,v $
  Language:  C++
  Date:      $Date: 2012/03/29 17:21:09 $
  Version:   $Revision: 1.1.1.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __cmCursesFilePathWidget_h
#define __cmCursesFilePathWidget_h

#include "cmCursesPathWidget.h"

class cmCursesFilePathWidget : public cmCursesPathWidget
{
public:
  cmCursesFilePathWidget(int width, int height, int left, int top);

protected:
  cmCursesFilePathWidget(const cmCursesFilePathWidget& from);
  void operator=(const cmCursesFilePathWidget&);

};

#endif // __cmCursesFilePathWidget_h
