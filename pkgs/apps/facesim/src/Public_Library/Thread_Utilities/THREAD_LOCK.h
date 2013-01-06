//#####################################################################
// Copyright 2005, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#ifndef __THREAD_LOCK__
#define __THREAD_LOCK__

#include <cstdio>
#include <cstdlib>
#include <cassert>

#ifdef ENABLE_PTHREADS
#if defined(ALAMERE_PTHREADS)
#   include "alamere.h"
#else
#    include <pthread.h>
#endif
#endif //ENABLE_PTHREADS

namespace PhysBAM
{

#ifdef ENABLE_PTHREADS
class THREAD_LOCK
{
private:
	pthread_mutex_t mutex;

public:
	THREAD_LOCK()
	{
		if (pthread_mutex_init (&mutex, 0) != 0)
		{
			Error();
		}
	}

	~THREAD_LOCK()
	{
		if (pthread_mutex_destroy (&mutex) != 0)
		{
			Error();
		}
	}

	void Lock()
	{
		if (pthread_mutex_lock (&mutex) != 0)
		{
			Error();
		}
	}

	void Unlock()
	{
		if (pthread_mutex_unlock (&mutex) != 0)
		{
			Error();
		}
	}

	void Error()
	{
		perror ("pthread");
		assert (0);
	}

	friend class THREAD_CONDITION;
};
#else
class THREAD_LOCK
{

public:
	THREAD_LOCK()
	{}

	~THREAD_LOCK()
	{}

	void Lock()
	{}

	void Unlock()
	{}

	void Error()
	{
		assert (0);
	}

	friend class THREAD_CONDITION;
};
#endif //ENABLE_PTHREADS
}
#endif

