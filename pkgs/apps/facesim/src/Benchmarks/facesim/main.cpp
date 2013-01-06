//#####################################################################
// Copyright 2004, Igor Neverov, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "../../Public_Library/Utilities/PARSE_ARGS.h"
#include "../../Public_Library/Thread_Utilities/THREAD_POOL.h"
#include "../../Public_Library/Thread_Utilities/THREAD_DIVISION_PARAMETERS.h"

#include "FACE_DRIVER.h"
#include "Storytelling/STORYTELLING_EXAMPLE.h"
#include "../../Public_Library/Utilities/LOG.h"

#ifdef ENABLE_PARSEC_HOOKS
#include <hooks.h>
#endif

using namespace PhysBAM;

#ifdef ENABLE_PTHREADS
//Use serial code
bool PHYSBAM_THREADED_RUN = true;
# else
//Use multi-threaded code
bool PHYSBAM_THREADED_RUN = false;
#endif //ENABLE_PTHREADS

int main (int argc, char* argv[])
{
#ifdef PARSEC_VERSION
#define __PARSEC_STRING(x) #x
#define __PARSEC_XSTRING(x) __PARSEC_STRING(x)
	printf ("PARSEC Benchmark Suite Version "__PARSEC_XSTRING (PARSEC_VERSION) "\n");
	fflush (NULL);
#else
	printf ("PARSEC Benchmark Suite\n");
	fflush (NULL);
#endif //PARSEC_VERSION
#ifdef ENABLE_PARSEC_HOOKS
	__parsec_bench_begin (__parsec_facesim);
#endif

	PARSE_ARGS parse_args;
	parse_args.Add_Integer_Argument ("-restart", 0);
	parse_args.Add_Integer_Argument ("-lastframe", 300);
	parse_args.Add_Integer_Argument ("-threads", 1);
	parse_args.Add_Option_Argument ("-timing");
	parse_args.Parse (argc, argv);

	STORYTELLING_EXAMPLE<float, float> example;

	FILE_UTILITIES::Create_Directory (example.output_directory);
	LOG::Copy_Log_To_File (example.output_directory + "/log.txt", example.restart);

	if (parse_args.Is_Value_Set ("-threads"))
	{
		static char tmp_buf[255];
		sprintf (tmp_buf, "PHYSBAM_THREADS=%d", parse_args.Get_Integer_Value ("-threads"));

		if (putenv (tmp_buf) < 0) perror ("putenv");
	}

	if (parse_args.Is_Value_Set ("-restart"))
	{
		example.restart = true;
		example.restart_frame = parse_args.Get_Integer_Value ("-restart");
	}

	if (parse_args.Is_Value_Set ("-lastframe"))
	{
		example.last_frame = parse_args.Get_Integer_Value ("-lastframe");
	}

	if (parse_args.Is_Value_Set ("-timing"))
	{
		example.write_output_files = false;
		example.verbose = false;
	}

	if (PHYSBAM_THREADED_RUN == false && parse_args.Get_Integer_Value ("-threads") > 1)
	{
		printf ("Error: Number of threads cannot be greater than 1 for serial runs\n");
		exit (1);
	}

	THREAD_DIVISION_PARAMETERS<float>& parameters = *THREAD_DIVISION_PARAMETERS<float>::Singleton();
	parameters.grid_divisions_3d = VECTOR_3D<int> (5, 5, 5);

	FACE_DRIVER<float, float> driver (example);

	driver.Execute_Main_Program();

	delete (THREAD_POOL::Singleton());

#ifdef ENABLE_PARSEC_HOOKS
	__parsec_bench_end();
#endif

	return 0;
}
