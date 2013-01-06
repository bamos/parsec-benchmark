//#####################################################################
// Copyright 2004-2006, Eran Guendelman, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DEBUG_UTILITIES
//#####################################################################
#ifndef __DEBUG_UTILITIES__
#define __DEBUG_UTILITIES__

#include <stdlib.h>
#include <assert.h>
#include <typeinfo>
#include <string>

#ifdef WIN32
#define NORETURN(declaration) __declspec(noreturn) declaration
#else
#define NORETURN(declaration) declaration __attribute__ ((noreturn))
#endif

#define WARN_IF_NOT_OVERRIDDEN() \
do{static bool __first_time__=true;if(__first_time__){Warn_If_Not_Overridden(__FUNCTION__,__FILE__,__LINE__,typeid(*this));__first_time__=false;}}while(0)

#define WARNING(message) \
do{static bool __first_time__=true;if(__first_time__){Warning((message),__FUNCTION__,__FILE__,__LINE__);__first_time__=false;}}while(0)

#define FUNCTION_IS_NOT_DEFINED() \
do{Function_Is_Not_Defined(__FUNCTION__,__FILE__,__LINE__,typeid(*this));exit(1);}while(0) // call exit explicitly since gcc seems to be ignoring noreturn

#define NOT_IMPLEMENTED() \
do{Not_Implemented(__FUNCTION__,__FILE__,__LINE__);exit(1);}while(0)

#define FATAL_ERROR() \
do{Fatal_Error(__FUNCTION__,__FILE__,__LINE__);exit(1);}while(0)

extern void Debug_Breakpoint();
extern void Warn_If_Not_Overridden (const char* function, const char* file, unsigned int line, const std::type_info& type);
extern void Warning (const std::string& message, const char* function, const char* file, unsigned int line);
extern void NORETURN (Function_Is_Not_Defined (const char* function, const char* file, unsigned int line, const std::type_info& type));
extern void NORETURN (Not_Implemented (const char* function, const char* file, unsigned int line));
extern void NORETURN (Fatal_Error (const char* function, const char* file, unsigned int line));

#endif
