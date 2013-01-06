//#####################################################################
// Copyright 2002-2004 Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Igor Neverov, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Namespace FILE_UTILITIES
//#####################################################################
#include "FILE_UTILITIES.h"
#ifdef WIN32
#include <windows.h>
#endif
#if defined(__linux__) || defined(__unix__)
#include <sys/stat.h>
#endif
#ifdef USE_COMPRESSION
#include "../../External_Libraries/gzstream/gzstream.h"
#endif
namespace PhysBAM
{
namespace FILE_UTILITIES
{

//###################################################################
// Win32 Specific Function Definitions
//###################################################################
#if defined(WIN32)

bool Directory_Exists (const std::string& dirname)
{
	DWORD attr = GetFileAttributes (dirname.c_str());
	return ( (attr != -1) && (attr & FILE_ATTRIBUTE_DIRECTORY));
}

bool Create_Directory (const std::string& dirname, bool exit_on_fail)
{
	if (!Directory_Exists (dirname))
	{
		std::cerr << "Creating directory using CreateDirectory...";
		CreateDirectory (dirname.c_str(), 0);

		if (!Directory_Exists (dirname))
		{
			std::cerr << "Failed!" << std::endl;
			std::cerr << "Create directory manually and re-run" << std::endl;

			if (exit_on_fail) exit (-1);
			else return false;
		}
		else
		{
			std::cerr << "Successful!" << std::endl;
			return true;
		}
	}

	return true;
}

std::string Real_Path (const std::string& path) //TODO: Implement this for windows
{
	std::cerr << "Real_Path not implemented on windows" << std::endl;
	return std::string();
}

int Compare_File_Times_Ignoring_Compression_Suffix (const std::string& filename1, const std::string& filename2)
{
	HANDLE handle1 = CreateFile (filename1.c_str(), 0, 0, 0, OPEN_EXISTING, 0, 0);

	if (handle1 == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Compare_File_Times: can't CreateFile " << filename1 << std::endl;
		return 0;
	}

	HANDLE handle2 = CreateFile (filename2.c_str(), 0, 0, 0, OPEN_EXISTING, 0, 0);

	if (handle2 == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Compare_File_Times: can't CreateFile " << filename2 << std::endl;
		return 0;
	}

	FILETIME time1, time2;

	if (!GetFileTime (handle1, 0, &time1, 0) || !GetFileTime (handle2, 0, &time2, 0))
	{
		std::cerr << "Compare_File_Times: error with GetFileTime" << std::endl;
		return 0;
	}

	CloseHandle (handle1);
	CloseHandle (handle2);
	return CompareFileTime (&time1, &time2);
}
//###################################################################
// Linux Specific Function Definitions
//###################################################################
#elif defined(__linux__) || defined(__unix__)

bool Directory_Exists (const std::string& dirname)
{
	return std::ifstream (dirname.c_str()) != 0;
}

bool Create_Directory (const std::string& dirname, bool exit_on_fail)
{
	if (!Directory_Exists (dirname))
	{
		std::string command = STRING_UTILITIES::string_sprintf ("mkdir -p %s", dirname.c_str());
		std::cerr << "Creating directory using system(\"" << command << "\")...";
		system (command.c_str());

		if (!Directory_Exists (dirname))
		{
			std::cerr << "Failed!" << std::endl;
			std::cerr << "Create directory manually and re-run" << std::endl;

			if (exit_on_fail) exit (-1);
			else return false;
		}
		else
		{
			std::cerr << "Successful!" << std::endl;
			return true;
		}
	}

	return true;
}

std::string Real_Path (const std::string& path)
{
	char real_path[512];
	realpath (path.c_str(), real_path);
	return real_path;
}

int Compare_File_Times_Ignoring_Compression_Suffix (const std::string& filename1, const std::string& filename2)
{
	struct stat stat1, stat2;

	if (stat (filename1.c_str(), &stat1) != 0)
	{
		std::cerr << "Compare_File_Times: can't stat " << filename1 << std::endl;
	}

	if (stat (filename2.c_str(), &stat2) != 0)
	{
		std::cerr << "Compare_File_Times: can't stat " << filename2 << std::endl;
	}

	if (stat1.st_mtime < stat2.st_mtime) return -1;
	else if (stat1.st_mtime > stat2.st_mtime) return 1;
	else return 0;
}
//###################################################################
// Default (Unimplemented) Function Definitions
//###################################################################
#else

bool Directory_Exists (const std::string& dirname)
{
	return true;       // always return true on unsupported platforms
}

bool Create_Directory (const std::string& dirname, bool exit_on_fail)
{
	std::cerr << "Create_Directory not supported, assuming directory exists...";
	return true;
}

std::string Real_Path (const std::string& path)
{
	std::cerr << "Real_Path unsupported..." << std::endl;
	return std::string();
}

int Compare_File_Times_Ignoring_Compression_Suffix (const std::string& filename1, const std::string& filename2)
{
	std::cerr << "Compare_File_Times unsupported..." << std::endl;
	return 0;
}

#endif
//###################################################################
// File open without compression capability
//###################################################################
#ifndef USE_COMPRESSION

std::istream* Safe_Open_Input (const std::string& filename, bool binary, bool exit_on_fail)
{
	if (File_Is_Compressed (filename) || File_Exists_Ignoring_Compression_Suffix (filename + ".gz"))
	{
		std::cerr << "Can't open " << filename << " (compression handling not enabled)\n";

		if (exit_on_fail)
		{
			assert (false);
			exit (1);
		}
		else return 0;
	}
	else
	{
		std::ios_base::openmode flags = std::ios::in;

		if (binary) flags |= std::ios::binary;

		std::istream* input = new std::ifstream (filename.c_str(), flags);

		if (!*input) delete input;
		else return input;

		std::cerr << "Can't open " << filename << " for read " << (binary ? "(binary)" : "") << std::endl;

		if (exit_on_fail)
		{
			assert (false);
			exit (1);
		}
		else return 0;
	}
}

std::ostream* Safe_Open_Output (const std::string& filename, bool binary, bool exit_on_fail)
{
	if (File_Is_Compressed (filename))
	{
		std::cerr << "Can't open " << filename << " (compression handling not enabled)\n";

		if (exit_on_fail)
		{
			assert (false);
			exit (1);
		}
		else return 0;
	}
	else if (File_Exists_Ignoring_Compression_Suffix (filename + ".gz"))
	{
		std::cerr << "Refusing to write " << filename << " uncompressed when compressed version already exists\n";

		if (exit_on_fail)
		{
			assert (false);
			exit (1);
		}
		else return 0;
	}
	else
	{
		std::ios_base::openmode flags = std::ios::out;

		if (binary) flags |= std::ios::binary;

		std::ostream* output = new std::ofstream (filename.c_str(), flags);

		if (!*output) delete output;
		else return output;

		std::cerr << "Can't open " << filename << " for write " << (binary ? "(binary)" : "") << std::endl;

		if (exit_on_fail)
		{
			assert (false);
			exit (1);
		}
		else return 0;
	}
}
//###################################################################
// File open with compression capability
//###################################################################
#else

std::istream* Safe_Open_Input (const std::string& filename, bool binary, bool exit_on_fail)
{
	bool compressed = File_Is_Compressed (filename);
	std::ios_base::openmode flags = std::ios::in;

	if (binary) flags |= std::ios::binary;

	std::string filename_compressed = compressed ? filename : filename + ".gz";
	std::istream* input;

	if (File_Exists (filename_compressed)) return new ::igzstream (filename_compressed.c_str(), flags);

	if (!compressed)
	{
		std::string filename_uncompressed = compressed ? Strip_Compression_Suffix (filename) : filename;
		input = new std::ifstream (filename_uncompressed.c_str(), flags);

		if (!*input) delete input;
		else return input;

		filename_compressed = filename_uncompressed + "(.gz)";
	}

	std::cerr << "Can't open " << filename_compressed << " for read " << (binary ? "(binary)" : "") << std::endl;

	if (exit_on_fail)
	{
		assert (false);
		exit (1);
	}
	else return 0;
}

std::ostream* Safe_Open_Output (const std::string& filename, bool binary, bool exit_on_fail)
{
	bool compressed = File_Is_Compressed (filename);
	std::ios_base::openmode flags = std::ios::out;

	if (binary) flags |= std::ios::binary;

	if (compressed && !binary)
	{
		std::cerr << "Refusing to open compressed file " << filename << "in text mode\n";

		if (exit_on_fail)
		{
			assert (false);
			exit (1);
		}
		else return 0;
	}

	std::string actual_filename;
	std::ostream* output;

	if (binary)
	{
		actual_filename = compressed ? filename : filename + ".gz";

		if (File_Writable (actual_filename)) return new ::ogzstream (actual_filename.c_str(), flags);
	}
	else
	{
		actual_filename = compressed ? Strip_Compression_Suffix (filename) : filename;
		output = new std::ofstream (actual_filename.c_str(), flags);

		if (!*output) delete output;
		else return output;
	}

	std::cerr << "Can't open " << actual_filename << " for write " << (binary ? "(binary)" : "") << std::endl;

	if (exit_on_fail)
	{
		assert (false);
		exit (1);
	}
	else return 0;
}

#endif
//###################################################################
// Compare_File_Times
//###################################################################
int Compare_File_Times (const std::string& filename1, const std::string& filename2)
{
	return Compare_File_Times_Ignoring_Compression_Suffix (Real_File (filename1), Real_File (filename2));
}
//#####################################################################

} // namespace FILE_UTILITIES

#ifdef __sparc__
void sparc_seg_fault_prevent_dummy (void *ptr)
{
}
#endif

} // namespace PhysBAM{
