//#####################################################################
// Copyright 2004-2006, Eran Guendelman, Geoffrey Irving, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DEBUG_UTILITIES
//#####################################################################
#include "DEBUG_UTILITIES.h"
#include "../Utilities/LOG.h"
#include <stdexcept>
#include <sstream>
using namespace PhysBAM;
//#####################################################################
// Function Debug_Breakpoint Linux
//#####################################################################
#if defined(__linux__) || defined(__CYGWIN__) || defined(__DARWIN__)
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

void Debug_Breakpoint()
{
	kill (getpid(), SIGSTOP); // if you use this you need to step out of the signal handler to get a non-corrupt stack
}
#else
//#####################################################################
// Function Debug_Breakpoint Default
//#####################################################################
void Debug_Breakpoint()
{
	assert (false);
}
#endif
//#####################################################################
// Function Warn_If_Not_Overridden
//#####################################################################
void Warn_If_Not_Overridden (const char* function, const char* file, unsigned int line, const std::type_info& type)
{
	LOG::cerr << "*** WARNING: base function '" << function << "' in file '" << file << "' line " << line << " not overridden by " << type.name() << "!!!" << std::endl;
}
//#####################################################################
// Function Warning
//#####################################################################
void Warning (const std::string& message, const char* function, const char* file, unsigned int line)
{
	LOG::cerr << "*** WARNING: '" << message << "' in function '" << function << "' in file '" << file << "' line " << line << "!!!" << std::endl;
}
//#####################################################################
// Function Function_Is_Not_Defined
//#####################################################################
void Function_Is_Not_Defined (const char* function, const char* file, unsigned int line, const std::type_info& type)
{
	std::ostringstream string_stream;
	string_stream << "*** ERROR: The function '" << function << "' in file '" << file << "' line " << line << " is not defined by " << type.name() << "!!!";
	LOG::cerr << string_stream.str() << std::endl;
	throw std::runtime_error (string_stream.str());
	assert (false);
	exit (1);
}
//#####################################################################
// Function Not_Implemented
//#####################################################################
void Not_Implemented (const char* function, const char* file, unsigned int line)
{
	std::ostringstream string_stream;
	string_stream << "*** ERROR: Something in function '" << function << "' in file '" << file << "' line " << line << " is not implemented!!!";
	LOG::cerr << string_stream.str() << std::endl;
	throw std::runtime_error (string_stream.str());
	assert (false);
	exit (1);
}
//#####################################################################
// Function Fatal_Error
//#####################################################################
void Fatal_Error (const char* function, const char* file, unsigned int line)
{
	std::ostringstream string_stream;
	string_stream << "*** ERROR: Fatal error in function '" << function << "' in file '" << file << "' line " << line << "!!!";
	LOG::cerr << string_stream.str() << std::endl;
	throw std::runtime_error (string_stream.str());
	assert (false);
	exit (1);
}
//#####################################################################
