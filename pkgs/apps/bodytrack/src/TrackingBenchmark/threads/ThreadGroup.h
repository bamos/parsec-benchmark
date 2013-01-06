// (C) Copyright Christian Bienia 2007
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0.
//
//  file : ThreadGroup.h
//  author : Christian Bienia - cbienia@cs.princeton.edu
//  description : A class which can manage a group of threads

#ifndef THREADGROUP_H
#define THREADGROUP_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <list>

#include "Thread.h"


namespace threads {

//Class to manage a group of threads
class ThreadGroup {
  private:
    std::list<Thread *> threads;

  public:
    ThreadGroup();
    ~ThreadGroup();

    //Create a new thread and add it to the group
    void CreateThread(Runnable &);
    //Create multiple new threads which use the same start function and add them to the group
    void CreateThreads(int, Runnable &);
    //Add an already existing thread
    void AddThread(Thread *);
    //Remove a thread from the group
    void RemoveThread(Thread *);

    //Join all threads
    void JoinAll();
    //Number of threads in this group
    int Size() const;
};

} //namespace threads

#endif //THREADGROUP_H
