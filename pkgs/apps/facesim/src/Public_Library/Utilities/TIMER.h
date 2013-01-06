//#####################################################################
// Copyright 2004, Frank Losasso.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TIMER
//#####################################################################
#ifndef __TIMER__
#define __TIMER__

#include <stdio.h>
double Get_Current_Time();
double Initialize_Timer();
namespace PhysBAM
{

class TIMER
{
private:
	double resolution;
	double* timer_start;
	double* timer_elapsed;
	int* free_timers;
	int number_of_free_timers;
	static TIMER* singleton_instance;

public:
	TIMER()
	{
		const int number_of_timers = 512;
		timer_start = new double[number_of_timers];
		timer_elapsed = new double[number_of_timers];
		free_timers = new int[number_of_timers];
		resolution = Initialize_Timer();
		timer_elapsed[0] = 0;
		timer_start[0] = 0;

		for (int i = 0; i < number_of_timers; i++) free_timers[i] = i;

		number_of_free_timers = number_of_timers;
	}

	~TIMER()
	{
		delete timer_start;
		delete timer_elapsed;
		delete free_timers;
	}

	static inline TIMER* Singleton()
	{
		if (singleton_instance == 0) singleton_instance = new TIMER();

		return singleton_instance;
	}

//#####################################################################
	double Get_Time();
	int Register_Timer();
	void Release_Timer (const int id);
	double Get_Total_Time_Since_Registration (const int id);
	double Peek_And_Reset_Time (const int id);
	void Reset_Time (const int id);
	double Peek_Time (const int id);
//#####################################################################
};
}
#endif
