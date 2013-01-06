//#####################################################################
// Copyright 2002-2004, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Igor Neverov, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Namespace FILE_UTILITIES
//#####################################################################
#ifndef __FILE_UTILITIES__
#define __FILE_UTILITIES__
#include <fstream>

#include <cstdlib>

#include "READ_WRITE_FUNCTIONS.h"
#include "../Utilities/STRING_UTILITIES.h"
namespace PhysBAM
{
namespace FILE_UTILITIES
{

//#####################################################################
// ADD NEW FILE EXTENSIONS HERE
// The enumeration should match the file_extensions array, and
// UNKNOWN_FILE should be matched with a 0 in the array.
enum FILE_TYPE {RGD_FILE, RGD2D_FILE, TRI_FILE, PHI_FILE, PHI2D_FILE, OCT_FILE, PLY_FILE, PLY2D_FILE, RGB_FILE, TRI2D_FILE, CURVE_FILE, CURVE2D_FILE, TET_FILE, HEX_FILE, UNKNOWN_FILE};
static const char *file_extensions[] = {"rgd", "rgd2d", "tri", "phi", "phi2d", "oct", "ply", "ply2d", "rgb", "tri2d", "curve", "curve2d", "tet", "hex", 0};
//#####################################################################

//###################################################################
// Platform Specific Function Definitions
//###################################################################
bool Directory_Exists (const std::string& dirname);
bool Create_Directory (const std::string& dirname, bool exit_on_fail = true);
std::string Real_Path (const std::string& path);
int Compare_File_Times_Ignoring_Compression_Suffix (const std::string& filename1, const std::string& filename2);
std::istream* Safe_Open_Input (const std::string& filename, bool binary = true, bool exit_on_fail = true);
std::ostream* Safe_Open_Output (const std::string& filename, bool binary = true, bool exit_on_fail = true);
//###################################################################
// Platform Compare_File_Times
//###################################################################
int Compare_File_Times (const std::string& filename1, const std::string& filename2);
//###################################################################
// Platform Non-Specific Function Definitions
//###################################################################
inline std::string Get_File_Extension_Ignoring_Compression_Suffix (const std::string &filename)
{
	std::string::size_type lastperiod = filename.rfind ('.'), lastslash = filename.rfind ('/');

	if (lastperiod != std::string::npos && (lastslash == std::string::npos || lastperiod > lastslash)) return filename.substr (lastperiod + 1);
	else return "";
}

inline std::string Get_Basename_Ignoring_Compression_Suffix (const std::string& filename)
{
	std::string::size_type lastperiod = filename.rfind (".");
	std::string::size_type lastslash = filename.rfind ("/");

	if (lastperiod != std::string::npos && (lastslash == std::string::npos || lastperiod > lastslash)) return filename.substr (0, lastperiod);
	else return filename;
}

inline bool File_Extension_Matches_Ignoring_Compression_Suffix (const std::string& filename, const std::string& ext, const bool case_sensitive = true)
{
	return !STRING_UTILITIES::Compare_Strings (Get_File_Extension_Ignoring_Compression_Suffix (filename), ext, case_sensitive);
}

inline bool File_Is_Compressed (const std::string& filename)
{
	return File_Extension_Matches_Ignoring_Compression_Suffix (filename, "gz");
}

inline bool File_Exists_Ignoring_Compression_Suffix (const std::string& filename)
{
	return std::ifstream (filename.c_str()) != 0;
}

inline bool File_Exists (const std::string& filename)
{
	return File_Exists_Ignoring_Compression_Suffix (filename) || (!File_Is_Compressed (filename) && File_Exists_Ignoring_Compression_Suffix (filename + ".gz"));
}

inline bool File_Writable_Ignoring_Compression_Suffix (const std::string& filename)
{
	return std::ofstream (filename.c_str()) != 0;
}

inline bool File_Writable (const std::string& filename)
{
	return File_Writable_Ignoring_Compression_Suffix (filename) || (!File_Is_Compressed (filename) && File_Writable_Ignoring_Compression_Suffix (filename + ".gz"));
}

inline std::string Strip_Compression_Suffix (const std::string& filename)
{
	if (File_Is_Compressed (filename)) return Get_Basename_Ignoring_Compression_Suffix (filename);
	else return filename;
}

inline std::string Real_File (const std::string& filename)
{
	if (File_Exists_Ignoring_Compression_Suffix (filename)) return filename;
	else if (!File_Is_Compressed (filename) && File_Exists_Ignoring_Compression_Suffix (filename + ".gz")) return filename + ".gz";
	else return "";
}

inline bool File_Extension_Matches (const std::string& filename, const std::string& ext, const bool case_sensitive = true)
{
	return File_Extension_Matches_Ignoring_Compression_Suffix (Strip_Compression_Suffix (filename), ext, case_sensitive);
}

inline FILE_TYPE Get_File_Type_Ignoring_Compression_Suffix (const std::string& filename)
{
	std::string ext = Get_File_Extension_Ignoring_Compression_Suffix (filename);

	for (int i = 0; file_extensions[i]; i++) if (ext == file_extensions[i]) return (FILE_TYPE) i;

	return UNKNOWN_FILE;
}

inline FILE_TYPE Get_File_Type (const std::string& filename)
{
	return Get_File_Type_Ignoring_Compression_Suffix (Strip_Compression_Suffix (filename));
}

inline bool File_Type_Matches_Ignoring_Compression_Suffix (const std::string& filename, FILE_TYPE type)
{
	return File_Extension_Matches_Ignoring_Compression_Suffix (filename, file_extensions[ (int) type]);
}

inline bool File_Type_Matches (const std::string& filename, FILE_TYPE type)
{
	return File_Type_Matches_Ignoring_Compression_Suffix (Strip_Compression_Suffix (filename), type);
}

inline bool Is_Rgd_File (const std::string& filename)
{
	return File_Type_Matches (filename, RGD_FILE);
}

inline bool Is_Rgd2D_File (const std::string& filename)
{
	return File_Type_Matches (filename, RGD2D_FILE);
}

inline bool Is_Tri_File (const std::string& filename)
{
	return File_Type_Matches (filename, TRI_FILE);
}

inline bool Is_Tet_File (const std::string& filename)
{
	return File_Type_Matches (filename, TET_FILE);
}

inline bool Is_Hex_File (const std::string& filename)
{
	return File_Type_Matches (filename, HEX_FILE);
}

inline bool Is_Phi_File (const std::string& filename)
{
	return File_Type_Matches (filename, PHI_FILE);
}

inline bool Is_Phi2D_File (const std::string& filename)
{
	return File_Type_Matches (filename, PHI2D_FILE);
}

inline bool Is_Oct_File (const std::string& filename)
{
	return File_Type_Matches (filename, OCT_FILE);
}

inline bool Is_Ply_File (const std::string& filename)
{
	return File_Type_Matches (filename, PLY_FILE);
}

inline bool Is_Ply2D_File (const std::string& filename)
{
	return File_Type_Matches (filename, PLY2D_FILE);
}

inline bool Is_Rgb_File (const std::string& filename)
{
	return File_Type_Matches (filename, RGB_FILE);
}

inline bool Is_Curve_File (const std::string& filename)
{
	return File_Type_Matches (filename, CURVE_FILE);
}

inline bool Is_Curve2D_File (const std::string& filename)
{
	return File_Type_Matches (filename, CURVE2D_FILE);
}

inline std::string Get_File_Extension (const std::string &filename)
{
	return Get_File_Extension_Ignoring_Compression_Suffix (Strip_Compression_Suffix (filename));
}

inline std::string Get_Basename (const std::string& filename)
{
	return Get_Basename_Ignoring_Compression_Suffix (Strip_Compression_Suffix (filename));
}

inline std::string Get_Base_Directory_Name (const std::string& path)
{
	size_t lastslash = path.rfind ('/');

	if (lastslash == std::string::npos) return ".";
	else return path.substr (0, lastslash);
}

inline std::string Number_To_String (const int i)
{
	return STRING_UTILITIES::string_sprintf ("%d", i);
}

inline std::string Find_First_Nonexistent_File_In_Sequence (std::string filename_base, std::string filename_suffix, const int id_start = 0, int* id_result = 0)
{
	int i = id_start;

	while (File_Exists ( (filename_base + Number_To_String (i) + filename_suffix))) ++i;

	if (id_result) *id_result = i;

	return filename_base + Number_To_String (i) + filename_suffix;
}

inline std::string Find_First_Nonexistent_Directory_In_Sequence (std::string directory_name_base, const int id_start = 0, int* id_final = 0)
{
	int id = id_start;

	while (Directory_Exists ( (directory_name_base + Number_To_String (id)))) ++id;

	if (id_final) *id_final = id;

	return directory_name_base + Number_To_String (id);
}

inline std::string Make_First_Nonexistent_Directory_In_Sequence (std::string directory_name_base, const int id_start = 0, int* id_final = 0)
{
	std::string output_directory = Find_First_Nonexistent_Directory_In_Sequence (directory_name_base, id_start, id_final);

	if (!Create_Directory (output_directory, false))
	{
		std::cerr << "Can't create directory " << output_directory << std::endl;
		exit (1);
	}
	else std::cout << "Created directory " << output_directory << std::endl;

	return output_directory;
}
//###################################################################
// Utilities for "animated" files (files with %d for frame number)
//###################################################################
inline bool Is_Animated (const std::string &filename)
{
	return filename.find ("%d") != std::string::npos;
}

inline std::string Get_Frame_Filename (const std::string &filename, int frame)
{
	return Is_Animated (filename) ? STRING_UTILITIES::string_sprintf (filename.c_str(), frame) : filename;
}

inline bool Frame_File_Exists (const std::string &filename, int frame)
{
	return File_Exists (Get_Frame_Filename (filename, frame));
}
//#####################################################################
// Read_From_File
//#####################################################################
// Convenience functions
template<class RW, class T1>
inline void Read_From_File (const std::string& filename, T1& d1)
{
	std::istream* input = Safe_Open_Input (filename);
	Read_Binary<RW> (*input, d1);
	delete input;
}
template<class RW, class T1, class T2>
inline void Read_From_File (const std::string& filename, T1& d1, T2& d2)
{
	std::istream* input = Safe_Open_Input (filename);
	Read_Binary<RW> (*input, d1, d2);
	delete input;
}
template<class RW, class T1, class T2, class T3>
inline void Read_From_File (const std::string& filename, T1& d1, T2& d2, T3& d3)
{
	std::istream* input = Safe_Open_Input (filename);
	Read_Binary<RW> (*input, d1, d2, d3);
	delete input;
}
template<class RW, class T1, class T2, class T3, class T4>
inline void Read_From_File (const std::string& filename, T1& d1, T2& d2, T3& d3, T4& d4)
{
	std::istream* input = Safe_Open_Input (filename);
	Read_Binary<RW> (*input, d1, d2, d3, d4);
	delete input;
}
template<class RW, class T1, class T2, class T3, class T4, class T5>
inline void Read_From_File (const std::string& filename, T1& d1, T2& d2, T3& d3, T4& d4, T5& d5)
{
	std::istream* input = Safe_Open_Input (filename);
	Read_Binary<RW> (*input, d1, d2, d3, d4, d5);
	delete input;
}
template<class RW, class T1, class T2, class T3, class T4, class T5, class T6>
inline void Read_From_File (const std::string& filename, T1& d1, T2& d2, T3& d3, T4& d4, T5& d5, T6& d6)
{
	std::istream* input = Safe_Open_Input (filename);
	Read_Binary<RW> (*input, d1, d2, d3, d4, d5, d6);
	delete input;
}
//#####################################################################
// Write_To_File
//#####################################################################
// Convenience functions
template<class RW, class T1>
inline void Write_To_File (const std::string& filename, const T1& d1)
{
	std::ostream* output = Safe_Open_Output (filename);
	Write_Binary<RW> (*output, d1);
	delete output;
}
template<class RW, class T1, class T2>
inline void Write_To_File (const std::string& filename, const T1& d1, const T2& d2)
{
	std::ostream* output = Safe_Open_Output (filename);
	Write_Binary<RW> (*output, d1, d2);
	delete output;
}
template<class RW, class T1, class T2, class T3>
inline void Write_To_File (const std::string& filename, const T1& d1, const T2& d2, const T3& d3)
{
	std::ostream* output = Safe_Open_Output (filename);
	Write_Binary<RW> (*output, d1, d2, d3);
	delete output;
}
template<class RW, class T1, class T2, class T3, class T4>
inline void Write_To_File (const std::string& filename, const T1& d1, const T2& d2, const T3& d3, const T4& d4)
{
	std::ostream* output = Safe_Open_Output (filename);
	Write_Binary<RW> (*output, d1, d2, d3, d4);
	delete output;
}
template<class RW, class T1, class T2, class T3, class T4, class T5>
inline void Write_To_File (const std::string& filename, const T1& d1, const T2& d2, const T3& d3, const T4& d4, const T5& d5)
{
	std::ostream* output = Safe_Open_Output (filename);
	Write_Binary<RW> (*output, d1, d2, d3, d4, d5);
	delete output;
}
template<class RW, class T1, class T2, class T3, class T4, class T5, class T6>
inline void Write_To_File (const std::string& filename, const T1& d1, const T2& d2, const T3& d3, const T4& d4, const T5& d5, const T6& d6)
{
	std::ostream* output = Safe_Open_Output (filename);
	Write_Binary<RW> (*output, d1, d2, d3, d4, d5, d6);
	delete output;
}
//#####################################################################
// Read_From_Text_File
//#####################################################################
// Convenience function
template<class T1>
inline void Read_From_Text_File (const std::string& filename, T1& d1)
{
	std::istream* input = Safe_Open_Input (filename, false);
	*input >> d1;
	delete input;
}
//#####################################################################
// Write_To_Text_File
//#####################################################################
// Convenience function
template<class T1>
inline void Write_To_Text_File (const std::string& filename, const T1& d1)
{
	std::ostream* output = Safe_Open_Output (filename, false);
	*output << d1;
	delete output;
}
//#####################################################################
// Create_From_File
//#####################################################################
template<class RW, class T>
inline void Create_From_File (const std::string& filename, T*& d)
{
	d = T::Create();
	Read_From_File<RW> (filename, *d);
}
//#####################################################################
}
}
#endif
