//#####################################################################
// Copyright 2004-2005, Eran Guendelman, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Namespace STRING_UTILITIES
//#####################################################################
#include "STRING_UTILITIES.h"
#include <stdio.h>
#include <ctype.h>
#include <cstring>
#include "../Arrays/LIST_ARRAY.h"
namespace PhysBAM
{
namespace STRING_UTILITIES
{

#ifdef _WIN32
#define vsnprintf _vsnprintf
#endif

std::string Stripped_Whitespace (const std::string& str)
{
	int first = 0, last = (int) str.length() - 1;

	while (first <= last && isspace (str[first])) first++;

	while (last >= 0 && isspace (str[last])) last--;

	return str.substr (first, last - first + 1);
}

bool Is_Number (const std::string& str) // integer or floating point
{
	const char *nptr = str.c_str(), *last_character = nptr + strlen (nptr);
	char *endptr;
	strtod (nptr, &endptr);
	return endptr == last_character;
}

int Compare_Strings (const std::string &str1, const std::string &str2, bool case_sensitive)
{
	if (case_sensitive) return str1.compare (str2); // use the built-in comparison
	else
	{
		std::string::const_iterator iter1, iter2;

		for (iter1 = str1.begin(), iter2 = str2.begin(); iter1 != str1.end() && iter2 != str2.end(); iter1++, iter2++)
			if (::toupper (*iter1) != ::toupper (*iter2)) return ( (::toupper (*iter1) < ::toupper (*iter2)) ? -1 : 1);

		std::string::size_type length1 = str1.length(), length2 = str2.length();
		return (length1 == length2) ? 0 : ( (length1 < length2) ? -1 : 1);
	}
}

std::string toupper (const std::string& str)
{
	std::string str_copy = str;

	for (std::string::iterator iter = str_copy.begin(); iter != str_copy.end(); iter++) *iter =::toupper (*iter);

	return str_copy;
}

std::string string_sprintf (const char* format, ...)
{
	va_list marker;
	va_start (marker, format);
	std::string result = string_vsprintf (format, marker);
	va_end (marker);
	return result;
}

// may truncate string for safety
std::string string_vsprintf (const char* format, va_list ap)
{
	static char buffer[2048];
	vsnprintf (buffer, sizeof (buffer) - 1, format, ap);
	return buffer;
}

bool Parse_Integer_Range (const std::string& str, LIST_ARRAY<int>& integer_list)
{
	int start_val, end_val;
	char* c_str = new char[str.length() + 1];
	strcpy (c_str, str.c_str());
	char* endptr = c_str;
	start_val = (int) strtol (c_str, &endptr, 10);

	if (endptr == c_str) return false;

	while (isspace (*endptr)) endptr++; // skip whitespace

	if (*endptr == '\0')
	{
		integer_list.Append_Element (start_val);        // single value
		return true;
	}
	else if (*endptr == '-')
	{
		char* nextptr = endptr + 1;
		end_val = (int) strtol (nextptr, &endptr, 10);

		if (endptr == nextptr) return false;

		while (isspace (*endptr)) endptr++; // skip whitespace

		if (*endptr == '\0')
		{
			for (int i = start_val; i <= end_val; i++) integer_list.Append_Element (i);

			return true;
		}
		else return false;
	}
	else return false;
}

// integer list format: [range],[range],[range],... where each [range] is either a single <number> or <number>-<number>, e.g. "1-3,4,7,10-11"
bool Parse_Integer_List (const std::string& str, LIST_ARRAY<int>& integer_list)
{
	integer_list.Reset_Current_Size_To_Zero();
	std::string remaining_string = str;

	while (!str.empty())
	{
		std::string token;
		std::string::size_type comma_pos = remaining_string.find (",");

		if (comma_pos != std::string::npos)
		{
			token = remaining_string.substr (0, comma_pos);
			remaining_string = remaining_string.substr (comma_pos + 1);
		}
		else
		{
			token = remaining_string;
			remaining_string = "";
		}

		if (!Parse_Integer_Range (token, integer_list)) return false;
	}

	return true;
}

//#####################################################################
}
}
