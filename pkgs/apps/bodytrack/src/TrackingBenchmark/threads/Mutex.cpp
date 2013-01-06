// (C) Copyright Christian Bienia 2007
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0.
//
//  file : Mutex.cpp
//  author : Christian Bienia - cbienia@cs.princeton.edu
//  description : A mutex

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#if defined(HAVE_LIBPTHREAD)
# include <pthread.h>
# include <errno.h>
#else //default: winthreads
//# include <windows.h>
#endif //HAVE_LIBPTHREAD

#include <exception>

#include "Mutex.h"


namespace threads {

Mutex::Mutex() throw(MutexException) {
#if defined(HAVE_LIBPTHREAD)
  int rv;

  rv = pthread_mutex_init(&m, NULL);

  switch(rv) {
    case 0:
      break;
    case EAGAIN:
    case ENOMEM:
    {
      MutexResourceException e;
      throw e;
      break;
    }
    case EPERM:
    case EBUSY:
    case EINVAL:
    {
      MutexInitException e;
      throw e;
      break;
    }
    default:
    {
      MutexUnknownException e;
      throw e;
      break;
    }
  }
#else //default: winthreads
  InitializeCriticalSection(&m);
#endif //HAVE_LIBPTHREAD
}

Mutex::~Mutex() throw(MutexException) {
#if defined(HAVE_LIBPTHREAD)
  int rv;

  rv = pthread_mutex_destroy(&m);

  switch(rv) {
    case 0:
      break;
    case EBUSY:
    case EINVAL:
    {
      MutexDestroyException e;
      throw e;
      break;
    }
    default:
    {
      MutexUnknownException e;
      throw e;
      break;
    }
  }
#else //default: winthreads
  DeleteCriticalSection(&m);
#endif //HAVE_LIBPTHREAD
}

//Enter a critical region
void Mutex::Lock() throw(MutexException) {
#if defined(HAVE_LIBPTHREAD)
  int rv;

  rv = pthread_mutex_lock(&m);

  switch(rv) {
    case 0:
      //no error
      break;
    case EINVAL:
    case EAGAIN:
    {
      MutexLockingException e;
      throw e;
      break;
    }
    case EDEADLK:
    {
      MutexDeadlockException e;
      throw e;
      break;
    }
    default:
    {
      MutexUnknownException e;
      throw e;
      break;
    }
  }
#else //default: winthreads
  EnterCriticalSection(&m);
#endif //HAVE_LIBPTHREAD
}

//Try to acquire the lock, return true if successful
bool Mutex::TryLock() throw(MutexException) {
#if defined(HAVE_LIBPTHREAD)
  int rv;

  rv = pthread_mutex_trylock(&m);

  switch(rv) {
    case 0:
      //no error
      break;
    case EBUSY:
      //not an error, lock acquisition expected to fail sometimes
      return false;
      break;
    case EINVAL:
    case EAGAIN:
    {
      MutexLockingException e;
      throw e;
      break;
    }
    default:
    {
      MutexUnknownException e;
      throw e;
      break;
    }
  }

  return true;
#else //default: winthreads
  return TryEnterCriticalSection(&m);
#endif //HAVE_LIBPTHREAD
}

//Leave a critical region
void Mutex::Unlock() throw(MutexException) {
#if defined(HAVE_LIBPTHREAD)
  int rv;

  rv = pthread_mutex_unlock(&m);

  switch(rv) {
    case 0:
      //no error
      break;
    case EINVAL:
    case EAGAIN:
    case EPERM:
    {
      MutexLockingException e;
      throw e;
      break;
    }
    default:
    {
      MutexUnknownException e;
      throw e;
      break;
    }
  }
#else //default: winthreads
  LeaveCriticalSection(&m);
#endif //HAVE_LIBPTHREAD
}

} //namespace threads
