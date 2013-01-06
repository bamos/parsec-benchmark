//#####################################################################
// Copyright 2004-2006, Geoffrey Irving, Frank Losasso, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "LOG.h"
#include <stdarg.h>
#include <sstream>
#include "TIMER.h"
#include "LOG_ENTRY.h"
#include "LOG_SCOPE.h"
#include "STRING_UTILITIES.h"
namespace PhysBAM
{
//#####################################################################
std::ostream LOG::cout (std::cout.rdbuf());
std::ostream LOG::cerr (std::cerr.rdbuf());
LOG* LOG::instance = 0;
int LOG::verbosity_level = (1 << 30) - 1;
bool LOG::suppress_cout = false;
bool LOG::suppress_timing = false;
FILE* LOG::log_file = 0;
bool LOG::log_file_temporary = false;
//#####################################################################
// Class LOG_COUT_BUFFER
//#####################################################################
class LOG_COUT_BUFFER: public std::stringbuf
{
	int sync()
	{
		if (!LOG::suppress_cout && LOG::instance->current_entry->depth < LOG::instance->verbosity_level)
		{
			if (LOG_ENTRY::start_on_separate_line) putchar ('\n');

			std::string buffer = str();

			for (std::string::size_type start = 0, end; start < buffer.length();)
			{
				end = buffer.find ('\n', start);

				if (LOG_ENTRY::needs_indent)
				{
					printf ("%*s", 2 * LOG::instance->current_entry->depth + 2, "");
					LOG_ENTRY::needs_indent = false;
				}

				fputs (buffer.substr (start, end - start).c_str(), stdout);

				if (end != std::string::npos)
				{
					putchar ('\n');
					LOG_ENTRY::needs_indent = true;
					start = end + 1;
				}
				else break;
			}

			LOG_ENTRY::start_on_separate_line = false;
			LOG::instance->current_entry->end_on_separate_line = true;
			fflush (stdout);
		}

		if (LOG::log_file)
		{
			if (LOG_ENTRY::log_file_start_on_separate_line) putc ('\n', LOG::log_file);

			std::string buffer = str();

			for (std::string::size_type start = 0, end; start < buffer.length();)
			{
				end = buffer.find ('\n', start);

				if (LOG_ENTRY::log_file_needs_indent)
				{
					fprintf (LOG::log_file, "%*s", 2 * LOG::instance->current_entry->depth + 2, "");
					LOG_ENTRY::log_file_needs_indent = false;
				}

				fputs (buffer.substr (start, end - start).c_str(), LOG::log_file);

				if (end != std::string::npos)
				{
					putc ('\n', LOG::log_file);
					LOG_ENTRY::log_file_needs_indent = true;
					start = end + 1;
				}
				else break;
			}

			LOG_ENTRY::log_file_start_on_separate_line = false;
			LOG::instance->current_entry->log_file_end_on_separate_line = true;
			fflush (LOG::log_file);
		}

		str ("");
		return std::stringbuf::sync();
	}
};
//#####################################################################
// Class LOG_CERR_BUFFER
//#####################################################################
class LOG_CERR_BUFFER: public std::stringbuf
{
	int sync()
	{
		if (LOG_ENTRY::start_on_separate_line) putchar ('\n');

		LOG_ENTRY::start_on_separate_line = false;
		fputs (str().c_str(), stderr);

		if (LOG::log_file)
		{
			if (LOG_ENTRY::log_file_start_on_separate_line) putc ('\n', LOG::log_file);

			LOG_ENTRY::log_file_start_on_separate_line = false;
			fputs (str().c_str(), LOG::log_file);
		}

		str ("");
		return std::stringbuf::sync();
	}
};
//#####################################################################
// Constructor
//#####################################################################
LOG::LOG()
{
	timer_id = TIMER::Singleton()->Register_Timer();
	cout.rdbuf (new LOG_COUT_BUFFER);
	cerr.rdbuf (new LOG_CERR_BUFFER);
	root = new LOG_SCOPE (0, 0, timer_id, "SIMULATION", "Simulation", verbosity_level);
	current_entry = root;
	root->Start();
}
//#####################################################################
// Destructor
//#####################################################################
LOG::~LOG()
{
	while (current_entry != 0) instance->current_entry = instance->current_entry->Get_Pop_Scope();

	Dump_Log();

	if (log_file) fclose (log_file);

	std::streambuf* cout_buffer = cout.rdbuf();
	cout.rdbuf (std::cout.rdbuf());
	delete cout_buffer;
	std::streambuf* cerr_buffer = cerr.rdbuf();
	cerr.rdbuf (std::cerr.rdbuf());
	delete cerr_buffer;
	TIMER::Singleton()->Release_Timer (timer_id);
}
//#####################################################################
// Function Initialize_Logging
//#####################################################################
void LOG::
Initialize_Logging (const bool suppress_cout_input, const bool suppress_timing_input, const int verbosity_level_input, const bool cache_initial_output)
{
	if (instance) delete instance;

	LOG::suppress_cout = suppress_cout_input;
	LOG::suppress_timing = suppress_timing_input;
	LOG::verbosity_level = verbosity_level_input - 1;

	if (cache_initial_output)
	{
		log_file = tmpfile();

		if (!log_file)
		{
			LOG::cerr << "Couldn't create temporary log file" << std::endl;
			exit (1);
		}

		log_file_temporary = true;
	}

	instance = new LOG();
}
//#####################################################################
// Function Copy_Log_To_File
//#####################################################################
void LOG::
Copy_Log_To_File (const std::string& filename, const bool append)
{
	if (!instance) Initialize_Logging();

	FILE* temporary_file = 0;

	if (log_file && log_file_temporary)
	{
		temporary_file = log_file;
		log_file = 0;
	}

	if (log_file)
	{
		if (LOG_ENTRY::log_file_start_on_separate_line) putc ('\n', log_file);

		instance->root->Dump_Log (log_file);
		fclose (log_file);
		log_file = 0;
	}

	if (!filename.empty())
	{
		if (append)
		{
			log_file = fopen (filename.c_str(), "a");

			if (!log_file)
			{
				LOG::cerr << "Can't open log file " << filename << " for append" << std::endl;
				exit (1);
			}

			putc ('\n', log_file);
		}
		else
		{
			log_file = fopen (filename.c_str(), "w");

			if (!log_file)
			{
				LOG::cerr << "Can't open log file " << filename << " for writing" << std::endl;
				exit (1);
			}
		}

		if (!temporary_file)
		{
			instance->root->Dump_Names (log_file);
			LOG_ENTRY::log_file_start_on_separate_line = LOG_ENTRY::log_file_needs_indent = instance->current_entry->log_file_end_on_separate_line = true;
		}
		else
		{
			fflush (temporary_file);
			fseek (temporary_file, 0, SEEK_SET);
			ARRAY<char> buffer (4096, false);

			for (;;)
			{
				int n = (int) fread (buffer.Get_Array_Pointer(), sizeof (char), buffer.m, temporary_file);
				fwrite (buffer.Get_Array_Pointer(), sizeof (char), n, log_file);

				if (n < buffer.m) break;
			}

			fflush (log_file);
		}
	}

	if (temporary_file) fclose (temporary_file);

	log_file_temporary = false;
}
//#####################################################################
// Function Finish_Logging
//#####################################################################
void LOG::
Finish_Logging()
{
	if (instance) delete instance;
}
//#####################################################################
// Function Time
//#####################################################################
void LOG::
Time (const char* format, ...)
{
	if (!instance) Initialize_Logging();

	if (instance->suppress_timing) return;

	va_list marker;
	va_start (marker, format);
	instance->current_entry = instance->current_entry->Get_New_Item (STRING_UTILITIES::string_vsprintf (format, marker));
	va_end (marker);
	instance->current_entry->Start();
}
//#####################################################################
// Function Stop_Time
//#####################################################################
void LOG::
Stop_Time()
{
	if (!instance) Initialize_Logging();

	if (instance->suppress_timing) return;

	instance->current_entry = instance->current_entry->Get_Stop_Time();
}
//#####################################################################
// Function Print
//#####################################################################
void LOG::
Print (const char* format, ...)
{
	if (!instance) Initialize_Logging();

	if (instance->suppress_timing) return;

	if (instance->current_entry->depth < instance->verbosity_level)
	{
		if (LOG_ENTRY::start_on_separate_line) putchar ('\n');

		if (LOG_ENTRY::needs_indent) printf ("%*s", 2 * instance->current_entry->depth + 2, "");

		va_list marker;
		va_start (marker, format);
		vprintf (format, marker);
		va_end (marker);
		LOG_ENTRY::start_on_separate_line = false;
		LOG_ENTRY::needs_indent = instance->current_entry->end_on_separate_line = true;
	}

	if (log_file)
	{
		if (LOG_ENTRY::log_file_start_on_separate_line) putc ('\n', log_file);

		if (LOG_ENTRY::log_file_needs_indent) fprintf (log_file, "%*s", 2 * instance->current_entry->depth + 2, "");

		va_list marker;
		va_start (marker, format);
		vfprintf (log_file, format, marker);
		va_end (marker);
		LOG_ENTRY::log_file_start_on_separate_line = false;
		LOG_ENTRY::log_file_needs_indent = instance->current_entry->log_file_end_on_separate_line = true;
	}
}
//#####################################################################
// Function Push_Scope
//#####################################################################
void LOG::
Push_Scope (const std::string& scope_identifier, const char* format, ...)
{
	if (!instance) Initialize_Logging();

	if (instance->suppress_timing) return;

	va_list marker;
	va_start (marker, format);
	instance->current_entry = instance->current_entry->Get_New_Scope (scope_identifier, STRING_UTILITIES::string_vsprintf (format, marker));
	va_end (marker);
	instance->current_entry->Start();
}
//#####################################################################
// Function Pop_Scope
//#####################################################################
void LOG::
Pop_Scope()
{
	if (!instance) Initialize_Logging();

	if (instance->suppress_timing) return;

	instance->current_entry = instance->current_entry->Get_Pop_Scope();

	if (instance->current_entry == 0)
	{
		LOG::cerr << "Could not pop scope. Defaulting to root" << std::endl;
		instance->current_entry = instance->root;
	}
}
//#####################################################################
// Function Reset
//#####################################################################
void LOG::
Reset()
{
	if (!instance) Initialize_Logging();

	if (instance->suppress_timing) return;

	delete instance->root;
	instance->root = new LOG_SCOPE (0, 0, instance->timer_id, "SIMULATION", "Simulation", instance->verbosity_level);
	instance->current_entry = instance->root;
}
//#####################################################################
// Function Dump_Log
//#####################################################################
void LOG::
Dump_Log()
{
	if (!instance) Initialize_Logging();

	if (instance->suppress_timing) return;

	if (LOG_ENTRY::start_on_separate_line)
	{
		putchar ('\n');
		LOG_ENTRY::start_on_separate_line = false;
	}

	if (!suppress_cout) instance->root->Dump_Log (stdout);

	if (log_file)
	{
		if (LOG_ENTRY::log_file_start_on_separate_line)
		{
			putc ('\n', log_file);
			LOG_ENTRY::log_file_start_on_separate_line = false;
		}

		instance->root->Dump_Log (log_file);
	}
}
//#####################################################################
// Function Dump_Log_XML
//#####################################################################
void LOG::
Dump_Log_XML (std::ostream& output)
{
	if (!instance) Initialize_Logging();

	if (instance->suppress_timing) return;

	if (LOG_ENTRY::start_on_separate_line) output << std::endl;

	LOG_ENTRY::start_on_separate_line = false;
	instance->root->Dump_Log_XML (output);
}
//#####################################################################
}
