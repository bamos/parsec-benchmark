//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                  
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : TrackingModelPthread.h
//  author : Christian Bienia - cbienia@cs.princeton.edu
//			 Scott Ettinger - scott.m.ettinger@intel.com
//  description : Observation model for kinematic tree body 
//			tracking threaded with Pthreads.
//				  
//  modified : 
//--------------------------------------------------------------

#ifndef TRACKINGMODELPTHREAD_H
#define TRACKINGMODELPTHREAD_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "TrackingModel.h"
#include "threads/WorkerGroup.h"
#include "threads/TicketDispenser.h"
#include "threads/Barrier.h"
#include "WorkPoolPthread.h"
#include "AsyncIO.h"





class TrackingModelPthread : public TrackingModel, public threads::Threadable {
public:
	//constructor
	TrackingModelPthread(WorkPoolPthread &_workers);
	~TrackingModelPthread();

	//Generate an edge map from the original camera image
	void CreateEdgeMap(FlexImage8u &src, FlexImage8u &dst);

	//entry function for worker threads
	void Exec(threads::thread_cmd_t, threads::thread_rank_t);

	//set number of frames to be loaded
	void SetNumFrames(unsigned int n) {nFrames = n; };

	//Load and process new observation data from image files for a given time(frame).  Generates edge maps from the raw image files.
	virtual bool GetObservation(float timeval);

	//terminate IO thread
	void close() { IOthread->Join(); };

private:
	//the pool with the worker threads
	WorkPoolPthread &workers;
	threads::Barrier workInit;
	threads::TicketDispenser<int> loopTickets;
	AsyncImageLoader imageLoader;
	threads::Thread *IOthread;
	bool IOthreadStarted;
	unsigned int nFrames;

	//granularity of work unit for dynamic load balancing
	const int WORKUNIT_SIZE_FILTERROW;
	const int WORKUNIT_SIZE_FILTERCOLUMN;
	const int WORKUNIT_SIZE_GRADIENT;

	//inputs and outputs for GradientMagThresholdPthread
	struct {
		FlexImage8u *src;
		float threshold;
		FlexImage8u *gr;
	} GradientArgs;

	//inputs and outputs for FlexFilter* functions
	struct {
		FlexImage8u *src;
		FlexImage8u *dst;
		float *kernel;
		int kernelSize;
		bool allocate;
	} FilterArgs;

	//Generate an edge map from the original camera image
	//Separable 7x7 gaussian filter
	inline void GaussianBlurPthread(FlexImage8u *src, FlexImage8u *dst);
};

#endif //TRACKINGMODELPTHREAD_H

