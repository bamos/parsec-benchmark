//#####################################################################
// Copyright 2004-2005, Eran Guendelman, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Namespace STRING_UTILITIES
//#####################################################################
#ifndef __STRING_UTILITIES__
#define __STRING_UTILITIES__

#include <string>
#include <sstream>
#include <stdarg.h>
namespace PhysBAM
{

template<class T> class LIST_ARRAY;

namespace STRING_UTILITIES
{

std::string Stripped_Whitespace (const std::string& str);

inline void Strip_Whitespace (std::string& str)
{
	str = Stripped_Whitespace (str);
}

bool Is_Number (const std::string& str); // integer or floating point
int Compare_Strings (const std::string &str1, const std::string &str2, bool case_sensitive = true);
std::string toupper (const std::string& str);
std::string string_sprintf (const char* format, ...);
std::string string_vsprintf (const char* format, va_list ap);
bool Parse_Integer_Range (const std::string& str, LIST_ARRAY<int>& integer_list);

// integer list format: [range],[range],[range],... where each [range] is either a single <number> or <number>-<number>, e.g. "1-3,4,7,10-11"
bool Parse_Integer_List (const std::string& str, LIST_ARRAY<int>& integer_list);

template<class T> inline bool String_To_Value (const std::string& str, T& value) // assumes operator>> defined
{
	std::istringstream string_stream (str);
	return (string_stream >> value) != 0;
}

//#####################################################################
}
}
#endif
