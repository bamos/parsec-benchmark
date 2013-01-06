// (C) Copyright Christian Bienia 2007
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0.
//
//  file : Thread.h
//  author : Christian Bienia - cbienia@cs.princeton.edu
//  description : A C++ thread

#ifndef THREAD_H
#define THREAD_H

#pragma warning( disable : 4290)		//disable Microsoft compiler exception warning

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#if defined(HAVE_LIBPTHREAD)
# include <pthread.h>
#endif //HAVE_LIBPTHREAD

#include <exception>


namespace threads {

//Abstract class which has to be implemented to make a class thread-capable
//The thread class constructor requires a threadable object to instantiate a thread object
class Runnable {
  public:
    virtual ~Runnable() {};
    //Thread objects will call the Run() method of its associated Runnable class
    virtual void Run() =0;
};

//Abstract class which has to be implemented if a thread requires extra steps to stop
//Join() will call the Stop() method of a stoppable class before it waits for the termination of Run()
class Stoppable {
  public:
    virtual ~Stoppable() {};
    //Join call the Stop() method
    virtual void Stop() =0;
};

//Exception which gets thrown if thread creation fails
class ThreadCreationException: public std::exception {
  public:
    virtual const char *what() const throw() {return "Error creating thread";}
};

//A thread
class Thread {
  private:
    Runnable &tobj;
#if defined(HAVE_LIBPTHREAD)
    pthread_t t;
#else //default: winthreads
    void *t;
    unsigned int t_id;
#endif //HAVE_LIBPTHREAD
  public:
    Thread(Runnable &) throw(ThreadCreationException);

    //Wait until Thread object has finished
    void Join();
};

} //namespace threads

#endif //THREAD_H
