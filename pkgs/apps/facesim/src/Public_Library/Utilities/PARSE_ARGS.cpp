//#####################################################################
// Copyright 2004, Eran Guendelman, Geoffrey Irving, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class PARSE_ARGS
//#####################################################################
#include <string>
#include "PARSE_ARGS.h"
using namespace PhysBAM;
using namespace std;
//#####################################################################
// Function Use_Help_Option
//#####################################################################
void PARSE_ARGS::Use_Help_Option (bool use_it)
{
	use_help_option = use_it;
}
//#####################################################################
// Function Add_Option_Argument
//#####################################################################
void PARSE_ARGS::
Add_Option_Argument (const std::string& arg_str, const std::string& desc)
{
	arg_data_list.Append_Element (ARG_DATA (arg_str, desc));
}
//#####################################################################
// Function Add_Integer_Argument
//#####################################################################
void PARSE_ARGS::
Add_Integer_Argument (const std::string& arg_str, int default_value, const std::string& val_name, const std::string& desc)
{
	arg_data_list.Append_Element (ARG_DATA (arg_str, val_name, desc, default_value));
}
//#####################################################################
// Function Add_Double_Argument
//#####################################################################
void PARSE_ARGS::
Add_Double_Argument (const std::string& arg_str, double default_value, const std::string& val_name, const std::string& desc)
{
	arg_data_list.Append_Element (ARG_DATA (arg_str, val_name, desc, default_value));
}
//#####################################################################
// Function Add_Vector2_Argument
//#####################################################################
void PARSE_ARGS::
Add_Vector2_Argument (const std::string& arg_str, double default_value[2], const std::string& val_name, const std::string& desc)
{
	arg_data_list.Append_Element (ARG_DATA (arg_str, val_name, desc, default_value, 2));
}
//#####################################################################
// Function Add_Vector3_Argument
//#####################################################################
void PARSE_ARGS::
Add_Vector3_Argument (const std::string& arg_str, double default_value[3], const std::string& val_name, const std::string& desc)
{
	arg_data_list.Append_Element (ARG_DATA (arg_str, val_name, desc, default_value, 3));
}
//#####################################################################
// Function Add_String_Argument
//#####################################################################
void PARSE_ARGS::
Add_String_Argument (const std::string& arg_str, const std::string& default_value, const std::string& val_name, const std::string& desc)
{
	arg_data_list.Append_Element (ARG_DATA (arg_str, val_name, desc, default_value));
}
//#####################################################################
// Function Add_Vector_2D_Argument
//#####################################################################
void PARSE_ARGS::
Add_Vector_2D_Argument (const std::string& arg_str, const VECTOR_2D<double> &default_value, const std::string& val_name, const std::string& desc)
{
	double vec[2] = {default_value.x, default_value.y};
	Add_Vector2_Argument (arg_str, vec, val_name, desc);
}
//#####################################################################
// Function Add_Vector_3D_Argument
//#####################################################################
void PARSE_ARGS::
Add_Vector_3D_Argument (const std::string& arg_str, const VECTOR_3D<double> &default_value, const std::string& val_name, const std::string& desc)
{
	double vec[3] = {default_value.x, default_value.y, default_value.z};
	Add_Vector3_Argument (arg_str, vec, val_name, desc);
}
//#####################################################################
// Function Set_Extra_Arguments
//#####################################################################
void PARSE_ARGS::
Set_Extra_Arguments (int num, const std::string& synopsis, const std::string& desc) // num=-1 for arbitrary extra rguments
{
	num_expected_extra_args = num;

	if (synopsis.length()) extra_args_synopsis = synopsis;

	if (desc.length()) extra_args_desc = desc;
}
//#####################################################################
// Function Set_Extra_Usage_Callback
//#####################################################################
void PARSE_ARGS::
Set_Extra_Usage_Callback (void (*extra_usage_callback_input) ())
{
	extra_usage_callback = extra_usage_callback_input;
}
//#####################################################################
// Function Parse
//#####################################################################
int PARSE_ARGS::
Parse (int argc, char* argv[])
{
	program_name = argv[0];
	int current_arg = 1;

	while (current_arg < argc)
	{
		if (use_help_option && !strcmp (argv[current_arg], "--help")) Print_Usage (true); // print help

		int match = Find_Match (argv[current_arg]);

		if (match < 0)
		{
			if (argv[current_arg][0] == '-') Print_Usage (true);
			else break;
		}

		if (!arg_data_list (match).Parse_Value (argc, argv, current_arg)) Print_Usage (true);
		else arg_data_list (match).value_set = true;
	}

	int num_remaining_args = argc - current_arg;

	if (num_expected_extra_args != -1 && num_remaining_args != num_expected_extra_args) Print_Usage (true); // didn't get the expected number of extra args

	extra_arg_list.Remove_All_Entries();

	for (int i = current_arg; i < argc; i++) extra_arg_list.Append_Element (argv[i]);

	return current_arg;
}
//#####################################################################
// Function Parse
//#####################################################################
int PARSE_ARGS::
Parse (int argc, const char* argv[])
{
	return Parse (argc, (char**) argv);
}
//#####################################################################
// Function Get_Option_Value
//#####################################################################
bool PARSE_ARGS::
Get_Option_Value (const std::string& arg_str) const
{
	int match = Find_Match (arg_str);
	assert (match >= 0);
	assert (arg_data_list (match).type == ARG_DATA::OPTION);
	return arg_data_list (match).boolean_value;
}
//#####################################################################
// Function Get_Integer_Value
//#####################################################################
int PARSE_ARGS::
Get_Integer_Value (const std::string& arg_str) const
{
	int match = Find_Match (arg_str);
	assert (match >= 0);
	assert (arg_data_list (match).type == ARG_DATA::INTEGER);
	return arg_data_list (match).integer_value;
}
//#####################################################################
// Function Get_Double_Value
//#####################################################################
double PARSE_ARGS::
Get_Double_Value (const std::string& arg_str) const
{
	int match = Find_Match (arg_str);
	assert (match >= 0);
	assert (arg_data_list (match).type == ARG_DATA::DOUBLE);
	return arg_data_list (match).double_value;
}
//#####################################################################
// Function Get_Vector2_Value
//#####################################################################
const double* PARSE_ARGS::
Get_Vector2_Value (const std::string& arg_str) const
{
	int match = Find_Match (arg_str);
	assert (match >= 0);
	assert (arg_data_list (match).type == ARG_DATA::VECTOR2);
	return arg_data_list (match).vector_value;
}
//#####################################################################
// Function Get_Vector2_Value
//#####################################################################
void PARSE_ARGS::
Get_Vector2_Value (const std::string& arg_str, double* value) const
{
	int match = Find_Match (arg_str);
	assert (match >= 0);
	assert (arg_data_list (match).type == ARG_DATA::VECTOR2);
	memcpy (value, arg_data_list (match).vector_value, 2 * sizeof (double));
}
//#####################################################################
// Function Get_Vector_2D_Value
//#####################################################################
VECTOR_2D<double> PARSE_ARGS::
Get_Vector_2D_Value (const std::string& arg_str) const
{
	const double* vec = Get_Vector2_Value (arg_str);
	VECTOR_2D<double> vector (vec[0], vec[1]);
	return vector;
}
//#####################################################################
// Function Get_Vector3_Value
//#####################################################################
const double* PARSE_ARGS::
Get_Vector3_Value (const std::string& arg_str) const
{
	int match = Find_Match (arg_str);
	assert (match >= 0);
	assert (arg_data_list (match).type == ARG_DATA::VECTOR3);
	return arg_data_list (match).vector_value;
}
//#####################################################################
// Function Get_Vector_3D_Value
//#####################################################################
VECTOR_3D<double> PARSE_ARGS::
Get_Vector_3D_Value (const std::string& arg_str) const
{
	const double* vec = Get_Vector3_Value (arg_str);
	VECTOR_3D<double> vector (vec[0], vec[1], vec[2]);
	return vector;
}
//#####################################################################
// Function Get_Vector3_Value
//#####################################################################
void PARSE_ARGS::
Get_Vector3_Value (const std::string& arg_str, double* value) const
{
	int match = Find_Match (arg_str);
	assert (match >= 0);
	assert (arg_data_list (match).type == ARG_DATA::VECTOR3);
	memcpy (value, arg_data_list (match).vector_value, 3 * sizeof (double));
}
//#####################################################################
// Function Get_String_Value
//#####################################################################
const std::string& PARSE_ARGS::
Get_String_Value (const std::string& arg_str) const
{
	int match = Find_Match (arg_str);
	assert (match >= 0);
	assert (arg_data_list (match).type == ARG_DATA::STRING);
	return arg_data_list (match).string_value;
}
//#####################################################################
// Function Is_Value_Set
//#####################################################################
bool PARSE_ARGS::
Is_Value_Set (const std::string& arg_str) const
{
	int match = Find_Match (arg_str);
	assert (match >= 0);
	return arg_data_list (match).value_set;
}
//#####################################################################
// Function Find_Match
//#####################################################################
int PARSE_ARGS::
Find_Match (const std::string& str) const
{
	for (int i = 1; i <= arg_data_list.m; i++) if (arg_data_list (i).str == str) return i;

	return -1;
}
//#####################################################################
// Function Num_Extra_Args
//#####################################################################
int PARSE_ARGS::
Num_Extra_Args() const
{
	return extra_arg_list.m;
}
//#####################################################################
// Function Extra_Arg
//#####################################################################
const std::string& PARSE_ARGS::
Extra_Arg (int i) const
{
	return extra_arg_list (i);
}
//#####################################################################
// Function Get_Program_Name
//#####################################################################
const std::string& PARSE_ARGS::
Get_Program_Name() const
{
	return program_name;
}
//#####################################################################
// Function Print_Usage
//#####################################################################
void PARSE_ARGS::
Print_Usage (bool do_exit) const
{
	int i;
	cerr << "Usage: " << program_name << " ";

	for (i = 1; i <= arg_data_list.m; i++)
	{
		arg_data_list (i).Print_Synopsis (cerr);
		cerr << " ";
	}

	cerr << extra_args_synopsis << std::endl;
	int width = 0;

	for (i = 1; i <= arg_data_list.m; i++)
	{
		int len = (int) arg_data_list (i).str.length();

		if (len > width) width = len;
	}

	for (i = 1; i <= arg_data_list.m; i++)
	{
		arg_data_list (i).Print_Description (cerr, width + 2);
		cerr << endl;
	}

	cerr << extra_args_desc << endl;

	if (extra_usage_callback) extra_usage_callback();

	if (do_exit) exit (-1);
}
//#####################################################################
// Function Find_And_Remove
//#####################################################################
bool PARSE_ARGS::
Find_And_Remove (const char *str, int& argc, char** argv)
{
	int i;

	for (i = 0; i < argc; i++) if (!strcmp (str, argv[i])) break;

	if (i < argc)
	{
		for (; i < argc - 1; i++) argv[i] = argv[i + 1];

		argc--;
		return true;
	}

	return false;
}
//#####################################################################
