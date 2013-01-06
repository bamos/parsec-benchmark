//#####################################################################
// Copyright 2004-2006, Geoffrey Irving, Frank Losasso, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class LOG
//#####################################################################
#ifndef __LOG__
#define __LOG__

#include <ostream>
namespace PhysBAM
{

class LOG_ENTRY;

class LOG
{
public:
	static std::ostream cout;
	static std::ostream cerr;
private:
	static LOG* instance;
	int timer_id;
	static int verbosity_level;
	static bool suppress_cout;
	static bool suppress_timing;
	LOG_ENTRY* root;
	LOG_ENTRY* current_entry;
	static FILE* log_file;
	static bool log_file_temporary;

	friend class LOG_ENTRY;
	friend class LOG_COUT_BUFFER;
	friend class LOG_CERR_BUFFER;

	LOG();
	~LOG();
public:

//#####################################################################
	static void Initialize_Logging (const bool suppress_cout_input = false, const bool suppress_timing_input = false, const int verbosity_level_input = 1 << 1, const bool cache_initial_output = false);
	static void Copy_Log_To_File (const std::string& filename, const bool append);
	static void Finish_Logging();
	static void Time (const char* format, ...);
	static void Stop_Time();
	static void Print (const char* format, ...);
	static void Push_Scope (const std::string& scope_identifier, const char* format_string, ...);
	static void Pop_Scope();
	static void Reset();
	static void Dump_Log();
	static void Dump_Log_XML (std::ostream& output);
//#####################################################################
};
}
#endif
