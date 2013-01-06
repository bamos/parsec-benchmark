//#####################################################################
// Copyright 2004, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#ifndef __THREAD_POOL__
#define __THREAD_POOL__
namespace PhysBAM
{

class THREAD_POOL
{
private:
	static THREAD_POOL* singleton_instance;
public:
	int number_of_threads;

	typedef void (*CALLBACK) (long, void*);

	static inline THREAD_POOL* Singleton()
	{
		if (!singleton_instance) singleton_instance = new THREAD_POOL();

		return singleton_instance;
	}

//#####################################################################
	THREAD_POOL();
	~THREAD_POOL();
	void Add_Task (CALLBACK callback_input, void* data_input);
	void Add_TaskGrid (CALLBACK callback_input, int numTasks);
	void Wait_For_Completion();
//#####################################################################
};
}
#endif
