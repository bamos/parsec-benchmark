//#####################################################################
// Copyright 2004, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include <iostream>
#include "THREAD_POOL.h"
using namespace PhysBAM;

#ifdef USE_THREAD_POOL_SINGLE
THREAD_POOL* THREAD_POOL::singleton_instance = 0;
//#####################################################################
// Function THREAD_POOL
//#####################################################################
THREAD_POOL::
THREAD_POOL()
{
	number_of_threads = 1;
}
//#####################################################################
// Function ~THREAD_POOL
//#####################################################################
THREAD_POOL::
~THREAD_POOL()
{}
//#####################################################################
// Function Add_Task
//#####################################################################
void THREAD_POOL::
Add_Task (CALLBACK callback_input, void* data_input)
{
	callback_input (0, data_input);
}
//#####################################################################
// Function Wait_For_Completion
//#####################################################################
void THREAD_POOL::
Wait_For_Completion()
{}
//#####################################################################
#endif
