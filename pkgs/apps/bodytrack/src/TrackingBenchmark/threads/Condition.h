// (C) Copyright Christian Bienia 2007
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0.
//
//  file : Condition.h
//  author : Christian Bienia - cbienia@cs.princeton.edu
//  description : A condition variable

#ifndef CONDITION_H
#define CONDITION_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#pragma warning( disable : 4290)		//disable Microsoft compiler exception warning

#if defined(HAVE_LIBPTHREAD)
# include <pthread.h>
#else //default: winthreads
# include <windows.h>
#endif //HAVE_LIBPTHREAD

#include <exception>

#include "Mutex.h"


namespace threads {

//General condition exception
class CondException: public std::exception {
  public:
    virtual const char *what() const throw() {return "Unspecified condition error";}
};

//Condition initialization error
class CondInitException: public CondException {
  public:
    virtual const char *what() const throw() {return "Unspecified error while initializing condition";}
};

//Condition destruction error
class CondDestroyException: public CondException {
  public:
    virtual const char *what() const throw() {return "Unspecified error while destroying condition";}
};

//Resources exhausted
class CondResourceException: public CondException {
  public:
    virtual const char *what() const throw() {return "Insufficient resources";}
};

//Unknown error
class CondUnknownException: public CondException {
  public:
    virtual const char *what() const throw() {return "Unknown error";}
};


//A standard condition variable
class Condition {
  public:
    Condition(Mutex &) throw(CondException);
    ~Condition() throw(CondException);

    //Wake up exactly one thread, return number of threads currently waiting (before wakeup)
    //If no more threads are waiting, the notification is lost
    int NotifyOne() throw(CondException);
    //Wake up all threads, return number of threads currently waiting (before wakeup)
    int NotifyAll() throw(CondException);

    //Wait until either NotifyOne() or NotifyAll() is executed
    void Wait() throw(CondException, MutexException);
  private:
    //the mutex associated with the condition variable
    Mutex *M;
    //number of waiting threads (with either no wakeup permission or pending wakeup)
    int nWaiting;
    //threads allowed to wake up (i.e. pending wakeups), nWakeupTickets <= nWaiting
    //notifications increase the number of tickets, successful wakeups decrease them
    int nWakeupTickets;
#if defined(HAVE_LIBPTHREAD)
    pthread_cond_t c;
#else //default: winthreads
    //generation counter to improve fairness
    unsigned int genCounter;
    HANDLE c;
#endif //HAVE_LIBPTHREAD

};

} //namespace threads

#endif //CONDITION_H
