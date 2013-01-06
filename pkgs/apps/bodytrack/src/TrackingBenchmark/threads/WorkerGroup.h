// (C) Copyright Christian Bienia 2007
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0.
//
//  file : WorkerGroup.h
//  author : Christian Bienia - cbienia@cs.princeton.edu
//  description : A class which can manage a group of worker threads

#ifndef WORKERGROUP_H
#define WORKERGROUP_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <vector>

#include "Thread.h"
#include "ThreadGroup.h"
#include "Mutex.h"
#include "Condition.h"
#include "Barrier.h"


namespace threads {
//type of a user command which can be sent to the threads
typedef unsigned short int thread_cmd_t;
//worker threads have a unique rank which which starts with 0
typedef unsigned int thread_rank_t;

//General group exception
class WorkerGroupException: public std::exception {
  public:
    virtual const char *what() const throw() {return "Unspecified worker group exception";}
};

//General group exception
class WorkerGroupCommandException: public WorkerGroupException {
  public:
    virtual const char *what() const throw() {return "Illegal command exception";}
};

//General group exception
class WorkerGroupCommandRangeException: public WorkerGroupCommandException {
  public:
    virtual const char *what() const throw() {return "Command out of range";}
};

class Threadable {
  public:
    virtual ~Threadable() {};
    //Thread objects will call the Exec() method and will pass the command and the rank of the thread
    virtual void Exec(thread_cmd_t, thread_rank_t) =0;
};


class WorkerGroup: protected ThreadGroup, protected Runnable {
  private:
    typedef int thread_internal_cmd_t;
    enum internal_cmds {
      THREADS_IDLE = -1,
      THREADS_SHUTDOWN = -2,
    };

    std::vector<Threadable *> cmds;
    thread_internal_cmd_t cmd;
    threads::Mutex workDispatch;                    //mutex controlling work dispatch
    threads::Condition workAvailable;               //condition to wait on for work
    threads::Barrier *workDoneBarrier;              //barrier to wait on for results
    threads::Barrier *poolReadyBarrier;             //work done, reset completed

    //Receive command with proper synchronization
    thread_internal_cmd_t RecvCmd();

    //Send a internal command to all worker threads
    void SendInternalCmd(thread_internal_cmd_t _cmd);

    //Acknowledge completion of command with proper synchronization
    void AckCmd();

  protected:
    void Run();

  public:
    //constructor
    WorkerGroup(int nThreads);
    //destructor
    ~WorkerGroup();

    //Add a new cmd -> object/function mapping to the pool
    void RegisterCmd(int _cmd, Threadable &obj);
    void RegisterCmd(thread_cmd_t _cmd, Threadable &obj);

    //Send a command to all worker threads
    void SendCmd(thread_cmd_t _cmd);

    //Terminate all threads in the group
    void JoinAll();

    //Number of workers in group
    int Size() const;
};

} //namespace threads

#endif //WORKERGROUP_H
