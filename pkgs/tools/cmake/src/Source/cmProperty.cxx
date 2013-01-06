/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmProperty.cxx,v $
  Language:  C++
  Date:      $Date: 2012/03/29 17:21:08 $
  Version:   $Revision: 1.1.1.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmProperty.h"
#include "cmSystemTools.h"

void cmProperty::Set(const char *name, const char *value)
{
  this->Name = name;
  this->Value = value;
  this->ValueHasBeenSet = true;
}

void cmProperty::Append(const char *name, const char *value)
{
  this->Name = name;
  if(!this->Value.empty() && *value)
    {
    this->Value += ";";
    }
  this->Value += value;
  this->ValueHasBeenSet = true;
}

const char *cmProperty::GetValue() const
{
  if (this->ValueHasBeenSet)
    {
    return this->Value.c_str();
    }
  return 0;
}
