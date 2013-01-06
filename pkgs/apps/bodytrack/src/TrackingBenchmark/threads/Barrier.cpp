// (C) Copyright Christian Bienia 2007
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0.
//
//  file : Barrier.cpp
//  author : Christian Bienia - cbienia@cs.princeton.edu
//  description : A barrier

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#if defined(HAVE_LIBPTHREAD)
# include <pthread.h>
# include <errno.h>
#else
# include "Mutex.h"
# include "Condition.h"
#endif //HAVE_LIBPTHREAD

#include <exception>

#include "Barrier.h"


namespace threads {

Barrier::Barrier(int _n) throw(BarrierException) {
#if defined(HAVE_LIBPTHREAD)
  int rv;

  n = _n;
  rv = pthread_barrier_init(&b, NULL, n);

  switch(rv) {
    case 0:
      break;
    case EINVAL:
    case EBUSY:
    {
      BarrierInitException e;
      throw e;
      break;
    }
    case EAGAIN:
    case ENOMEM:
    {
      BarrierResourceException e;
      throw e;
      break;
    }
    default:
    {
      BarrierUnknownException e;
      throw e;
      break;
    }
  }
#else
  n = _n;
  countSleep = 0;
  countReset = 0;
  M = new Mutex;
  CSleep = new Condition(*M);
  CReset = new Condition(*M);
#endif //HAVE_LIBPTHREAD
}

Barrier::~Barrier() throw(BarrierException) {
#if defined(HAVE_LIBPTHREAD)
  int rv;

  rv = pthread_barrier_destroy(&b);

  switch(rv) {
    case 0:
      break;
    case EINVAL:
    case EBUSY:
    {
      BarrierDestroyException e;
      throw e;
      break;
    }
    default:
    {
      BarrierUnknownException e;
      throw e;
      break;
    }
  }
#else
  delete CReset;
  delete CSleep;
  delete M;
#endif //HAVE_LIBPTHREAD
}

//Wait at a barrier
bool Barrier::Wait() throw(BarrierException) {
#if defined(HAVE_LIBPTHREAD)
  int rv;

  rv = pthread_barrier_wait(&b);

  switch(rv) {
    case 0:
      break;
    case PTHREAD_BARRIER_SERIAL_THREAD:
      return true;
      break;
    case EINVAL:
    {
      BarrierException e;
      throw e;
      break;
    }
    default:
    {
      BarrierUnknownException e;
      throw e;
      break;
    }
  }

  return false;
#else
  bool master;

  M->Lock();

  //Make sure no more than n threads have entered the barrier yet, otherwise wait for reset
  while(countSleep >= n) CReset->Wait();

  //Enter barrier, pick a thread as master
  master = (countSleep == 0);
  countSleep++;

  //Sleep until designated number of threads have entered barrier
  if(countSleep < n) {
    //Wait() must be free of spurious wakeups
    CSleep->Wait();
  } else {
    countReset = 0;  //prepare for synchronized reset
    CSleep->NotifyAll();
  }

  //Leave barrier
  countReset++;

  //Wait until all threads have left barrier, then execute reset
  if(countReset < n) {
    //Wait() must be free of spurious wakeups
    CReset->Wait();
  } else {
    countSleep = 0;
    CReset->NotifyAll();
  }

  M->Unlock();

  return master;
#endif //HAVE_LIBPTHREAD
}

const int Barrier::nThreads() const {
  return n;
}

};
