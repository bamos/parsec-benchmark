//------------------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                           
//	  2006, Christian Bienia, licensed under Apache 2.0 
//
//  file :	WorkPoolPthread.h
//  author :	Christian Bienia - cbienia@cs.princeton.edu
//
//  description : Work assignment and management class for pthreads
//
//  modified : 
//--------------------------------------------------------------------------


#ifndef WORKPOOLPTHREAD_H
#define WORKPOOLPTHREAD_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "threads/WorkerGroup.h"



class WorkPoolPthread: public threads::WorkerGroup {
public:
	//constants encoding commands from boss to worker threads
	enum {
		THREADS_CMD_PARTICLEWEIGHTS,
		THREADS_CMD_NEWPARTICLES,
		THREADS_CMD_FILTERROW,
		THREADS_CMD_FILTERCOLUMN,
		THREADS_CMD_GRADIENT
	};

	
	//constructor
	WorkPoolPthread(int nThreads) : threads::WorkerGroup(nThreads) {}
};

#endif //WORKPOOLPTHREAD_H
