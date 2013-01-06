//#####################################################################
// Copyright 2004, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include <iostream>
#include "THREAD_POOL.h"
using namespace PhysBAM;

#include <cstdlib>

#ifdef USE_ALAMERE_TASKQ
extern "C" {
#include "taskQ.h"
}

static int startedEnqueuing = 0;

THREAD_POOL* THREAD_POOL::singleton_instance = 0;
//#####################################################################
// Function THREAD_POOL
//#####################################################################
THREAD_POOL::
THREAD_POOL()
{
	//std::cout<<"THREAD_POOL_ALAMERE: Initializing..."<<std::endl;
	char* threads_environment = getenv ("PHYSBAM_THREADS");

	if (threads_environment) number_of_threads = atoi (threads_environment);
	else number_of_threads = 1;

	//std::cout<<"THREAD_POOL_ALAMERE thread count: " << number_of_threads << std::endl;
	taskQInit (number_of_threads, 65536);
}
//#####################################################################
// Function ~THREAD_POOL
//#####################################################################
THREAD_POOL::
~THREAD_POOL()
{
	taskQEnd();
}
//#####################################################################
// Function Add_Task
//#####################################################################
void THREAD_POOL::
Add_Task (CALLBACK callback_input, void* data_input)
{
	taskQEnqueueTask1 (callback_input, 0, data_input);
}
//#####################################################################
// Function Add_TaskGrid
//#####################################################################
void THREAD_POOL::
Add_TaskGrid (CALLBACK callback_input, int numTasks)
{
	if (!startedEnqueuing)
	{
		startedEnqueuing = 1;
	}

#define MAX_DIMENSION 3
	long int dimensionSize[MAX_DIMENSION], tileSize[MAX_DIMENSION];
	dimensionSize[0] = numTasks;
	tileSize[0] = 1;
	taskQEnqueueGrid ( (TaskQTask) callback_input, 0, 1, dimensionSize, tileSize);
#undef MAX_DIMENSION
}
//#####################################################################
// Function Wait_For_Completion
//#####################################################################
void THREAD_POOL::
Wait_For_Completion()
{
	taskQWait();
}
//#####################################################################
#endif
