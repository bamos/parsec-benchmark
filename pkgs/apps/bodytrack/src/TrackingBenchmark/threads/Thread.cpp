// (C) Copyright Christian Bienia 2007
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0.
//
//  file : Thread.cpp
//  author : Christian Bienia - cbienia@cs.princeton.edu
//  description : A C++ thread

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#if defined(HAVE_LIBPTHREAD)
# include <pthread.h>
#else //default: winthreads
# include <windows.h>
# include <process.h>
#endif //HAVE_LIBPTHREAD

#include <typeinfo>
#include "Thread.h"


namespace threads {

#if defined(HAVE_LIBPTHREAD)
// Unfortunately, thread libraries such as pthreads which use the C
// calling convention are incompatible with C++ member functions.
// To provide an object-oriented thread interface despite this obstacle,
// we make use of a helper function which will wrap the member function.
extern "C" {
  static void *thread_entry(void *arg) {
    Runnable *tobj = static_cast<Runnable *>(arg);
    tobj->Run();

    return NULL;
  }
}
#else //default: winthreads
extern "C" {
  unsigned __stdcall thread_entry(void *arg) {
    Runnable *tobj = static_cast<Runnable *>(arg);
    tobj->Run();
	return NULL;
  }
}
#endif //HAVE_LIBPTHREAD


//Constructor, expects a threadable object as argument
Thread::Thread(Runnable &_tobj) throw(ThreadCreationException) : tobj(_tobj) {
#if defined(HAVE_LIBPTHREAD)
  if(pthread_create(&t, NULL, &thread_entry, (void *)&tobj)) {
    ThreadCreationException e;
    throw e;
  }
#else //default: winthreads
  t = (void *)_beginthreadex(NULL, 0, &thread_entry, (void *)&tobj, 0, &t_id);
  if(!t) {
    ThreadCreationException e;
    throw e;
  }
#endif //HAVE_LIBPTHREAD
}

//Wait until Thread object has finished
void Thread::Join() {
  Stoppable *_tobj;
  bool isStoppable = true;

  //call Stop() function if implemented
  try {
    _tobj = &dynamic_cast<Stoppable &>(tobj);
  } catch(std::bad_cast e) {
    isStoppable = false;
  }
  if(isStoppable) {
    _tobj->Stop();
  }

#if defined(HAVE_LIBPTHREAD)
  pthread_join(t, NULL);
#else //default: winthreads
  WaitForSingleObject(t, INFINITE);
  CloseHandle(t);
#endif //HAVE_LIBPTHREAD
}

} //namespace threads
