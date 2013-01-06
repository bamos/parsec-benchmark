//#####################################################################
// Copyright 2004, Frank Losasso.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TIMER
//#####################################################################
#include "TIMER.h"
#include <iostream>
using namespace PhysBAM;

#ifdef _WIN32 // we are running on windows
#pragma comment(lib, "winmm.lib")
#include <windows.h>
double Get_Current_Time()
{
	__int64 time;
	QueryPerformanceCounter ( (LARGE_INTEGER*) &time);
	return (double) time;
}
double Initialize_Timer()
{
	__int64 frequency;
	QueryPerformanceFrequency ( (LARGE_INTEGER*) &frequency);
	return 1000. / (double) frequency;
}
#elif defined __linux__ || defined __unix__ // we are running on linux/unix
#include <sys/time.h>
#include <sys/resource.h>
double Get_Current_Time()
{
	timeval tv;
	gettimeofday (&tv, 0);
	return tv.tv_sec + 1e-6 * tv.tv_usec;
}
double Initialize_Timer()
{
	return 1e3;       // to convert seconds to milliseconds
}
#else
#error No implementation for Get_Current_Time!
#endif

TIMER* TIMER::singleton_instance = 0;
//####################################################################
// Function Register_Timer
//#####################################################################
int TIMER::
Register_Timer()
{
	if (number_of_free_timers <= 0)
	{
		std::cerr << "No more timers available, timing information may be incorrect" << std::endl;
		return 0;
	}

	int return_id = free_timers[number_of_free_timers - 1];
	number_of_free_timers--;

	double time = Get_Current_Time();
	timer_start[return_id] = time;
	timer_elapsed[return_id] = time;
	return return_id;
}
//#####################################################################
// Function Release_Timer
//#####################################################################
void TIMER::
Release_Timer (const int id)
{
	free_timers[number_of_free_timers++] = id;
}
//#####################################################################
// Function Get_Time
//#####################################################################
double TIMER::
Get_Time()
{
	return Get_Current_Time() * resolution;
}
//#####################################################################
// Function Get_Total_Time_Since_Registration
//#####################################################################
double TIMER::
Get_Total_Time_Since_Registration (const int id)
{
	return (Get_Current_Time() - timer_start[id]) * resolution;
}
//#####################################################################
// Function Peek_And_Reset_Time
//#####################################################################
double TIMER::
Peek_And_Reset_Time (const int id)
{
	double time = Get_Current_Time();
	double time_elapsed = (time - timer_elapsed[id]) * resolution;
	timer_elapsed[id] = time;
	return time_elapsed;
}
//#####################################################################
// Function Reset_Time
//#####################################################################
void TIMER::
Reset_Time (const int id)
{
	double time = Get_Current_Time();
	timer_elapsed[id] = time;
}
//#####################################################################
// Function Peek_Time
//#####################################################################
double TIMER::
Peek_Time (const int id)
{
	return (Get_Current_Time() - timer_elapsed[id]) * resolution;
}
//#####################################################################
