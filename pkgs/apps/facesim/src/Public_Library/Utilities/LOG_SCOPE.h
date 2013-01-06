//#####################################################################
// Copyright 2004, Geoffrey Irving, Frank Losasso, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class LOG_SCOPE
//#####################################################################
#ifndef __LOG_SCOPE__
#define __LOG_SCOPE__

#include "LOG_ENTRY.h"
#include <string>
#include "../Arrays/LIST_ARRAY.h"
#include "../Data_Structures/SPLAY_TREE.h"
#include "TIMER.h"
namespace PhysBAM
{

class LOG_SCOPE: public LOG_ENTRY
{
public:
	SPLAY_TREE<std::string, int> entries;
	LIST_ARRAY<LOG_ENTRY*> children;
	std::string scope_identifier;

	LOG_SCOPE (LOG_ENTRY* parent_input, int depth_input, int timer_id_input, const std::string& scope_identifier_input, const std::string& name_input, int& verbosity_level_input)
		: LOG_ENTRY (parent_input, depth_input, timer_id_input, name_input, verbosity_level_input), scope_identifier (scope_identifier_input)
	{}

	virtual LOG_ENTRY* Get_Stop_Time()
	{
		return this;
	}

	LOG_ENTRY* Get_New_Scope (const std::string& new_scope_identifier, const std::string& new_name)
	{
		int entry;
		end_on_separate_line = true;
		log_file_end_on_separate_line = true;

		if (entries.Find (new_scope_identifier, entry))
		{
			children (entry)->name = new_name;
			return children (entry);
		}

		LOG_ENTRY* new_entry = new LOG_SCOPE (this, depth + 1, timer_id, new_scope_identifier, new_name, verbosity_level);
		children.Append_Element (new_entry);
		entries.Set (new_scope_identifier, children.m);
		return new_entry;
	}

	LOG_ENTRY* Get_New_Item (const std::string& new_name)
	{
		int entry;
		end_on_separate_line = true;
		log_file_end_on_separate_line = true;

		if (entries.Find (new_name, entry)) return children (entry);

		LOG_ENTRY* new_entry = new LOG_ENTRY (this, depth + 1, timer_id, new_name, verbosity_level);
		children.Append_Element (new_entry);
		entries.Set (new_name, children.m);
		return new_entry;
	}

	LOG_ENTRY* Get_Pop_Scope()
	{
		Stop();
		return parent;
	}

	void Dump_Log (FILE* output)
	{
		fprintf (output, "%*s%-*s%8.4f\n", 2 * depth, "", 50 - 2 * depth, scope_identifier.c_str(), time);
		fflush (output);

		for (int i = 1; i <= children.m; i++) children (i)->Dump_Log (output);
	}

	void Dump_Log_XML (std::ostream& output)
	{
		output << STRING_UTILITIES::string_sprintf ("%*s", 2 * depth, "") << "<scope name=\"" << scope_identifier << "\" time=\"" << time << "\">" << std::endl;

		for (int i = 1; i <= children.m; i++) children (i)->Dump_Log_XML (output);

		output << STRING_UTILITIES::string_sprintf ("%*s", 2 * depth, "") << "</scope>" << std::endl;
	}

	void Dump_Names (FILE* output)
	{
		LOG_ENTRY::Dump_Names (output);

		for (int i = 1; i <= children.m; i++) children (i)->Dump_Names (output);
	}

//#####################################################################
};
}
#endif
