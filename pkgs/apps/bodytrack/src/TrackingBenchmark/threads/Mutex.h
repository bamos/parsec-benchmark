// (C) Copyright Christian Bienia 2007
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0.
//
//  file : Mutex.h
//  author : Christian Bienia - cbienia@cs.princeton.edu
//  description : A mutex

#ifndef MUTEX_H
#define MUTEX_H

#pragma warning( disable : 4290)		//disable Microsoft compiler exception warning

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#if defined(HAVE_LIBPTHREAD)
# include <pthread.h>
#else //default: winthreads
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x400
#endif	//_WIN32_WINNT
# include <windows.h>
#endif //HAVE_LIBPTHREAD

#include <exception>

#include "LockTypes.h"


namespace threads {

//General mutex exception
class MutexException: public std::exception {
  public:
    virtual const char *what() const throw() {return "Unspecified mutex error";}
};

//Mutex initialization error
class MutexInitException: public MutexException {
  public:
    virtual const char *what() const throw() {return "Unspecified error while initializing mutex";}
};

//Mutex destruction error
class MutexDestroyException: public MutexException {
  public:
    virtual const char *what() const throw() {return "Unspecified error while destroying mutex";}
};

//Resources exhausted
class MutexResourceException: public MutexException {
  public:
    virtual const char *what() const throw() {return "Insufficient resources";}
};

//General locking error
class MutexLockingException: public MutexException {
  public:
    virtual const char *what() const throw() {return "Unspecified locking error";}
};

//Deadlock detected
class MutexDeadlockException: public MutexLockingException {
  public:
    virtual const char *what() const throw() {return "Deadlock detected";}
};

//Unknown error
class MutexUnknownException: public MutexException {
  public:
    virtual const char *what() const throw() {return "Unknown error";}
};

//Condition class requires access to mutex, forward declaration for friendship
class Condition;

//A standard mutex
class Mutex: public LockType {
  //Condition class requires access to mutex
  friend class Condition;

  public:
    Mutex() throw(MutexException);
    ~Mutex() throw(MutexException);

    //Enter a critical region
    void Lock() throw(MutexException);
    //Leave a critical region
    void Unlock() throw(MutexException);
    //Try to acquire the lock, return true if successful
    bool TryLock() throw(MutexException);

  private:
#if defined(HAVE_LIBPTHREAD)
    pthread_mutex_t m;
#else //default: winthreads
    CRITICAL_SECTION m;
#endif //HAVE_LIBPTHREAD
};

} //namespace threads

#endif //MUTEX_H
