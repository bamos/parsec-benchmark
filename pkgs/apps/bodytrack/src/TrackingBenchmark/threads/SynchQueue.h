// (C) Copyright Christian Bienia 2007
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0.
//
//  file : SynchQueue.h
//  author : Christian Bienia - cbienia@cs.princeton.edu
//  description : A synchronized queue

#ifndef SYNCHQUEUE_H
#define SYNCHQUEUE_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <pthread.h>
#include <queue>

#include "Mutex.h"
#include "Condition.h"


namespace threads {

//General queue exception
class SynchQueueException: public std::exception {
  public:
    const char *what() {return "Unspecified synchronization queue error";}
};

//capacity constant for queues with no maximum capacity
#define SYNCHQUEUE_NOCAPACITY -1

template <typename T>
class SynchQueue {
  public:
    //In addition to the default (unbounded) queue behavior, synchronized queues support a maximum capacity
    SynchQueue();
    SynchQueue(int);
    ~SynchQueue();

    bool isEmpty() const;
    bool isFull() const;
    int Size() const;
    const int Capacity() const;
    void Enqueue(const T&);
    const T &Dequeue();

  private:
    std::queue<T> q;
    int cap;
    Mutex *M;
    Condition *notEmpty;
    Condition *notFull;
};

template <typename T>
SynchQueue<T>::SynchQueue() {
  cap = SYNCHQUEUE_NOCAPACITY;
  M = new Mutex;
  notEmpty = new Condition(M);
  notFull = new Condition(M);
}

template <typename T>
SynchQueue<T>::SynchQueue(int _cap) {
  if(_cap < 1) {
    SynchQueueException e;
    throw e;
  }

  cap = _cap;
  M = new Mutex;
  notEmpty = new Condition(*M);
  notFull = new Condition(*M);
}

template <typename T>
SynchQueue<T>::~SynchQueue() {
  delete notFull;
  delete notEmpty;
  delete M;
}

template <typename T>
bool SynchQueue<T>::isEmpty() const {
  bool rv;

  M->Lock();
  rv = q.empty();
  M->Unlock();

  return rv;
}

template <typename T>
bool SynchQueue<T>::isFull() const {
  int s;

  if(cap == SYNCHQUEUE_NOCAPACITY) {
    return false;
  }

  M->Lock();
  s = q.size();
  M->Unlock();

  return (cap == s);
}

template <typename T>
int SynchQueue<T>::Size() const {
  int s;

  M->Lock();
  s = q.size();
  M->Unlock();

  return s;
}

template <typename T>
const int SynchQueue<T>::Capacity() const {
  return cap;
}

template <typename T>
void SynchQueue<T>::Enqueue(const T &x) {
  M->Lock();
  while(q.size() >= cap && cap != SYNCHQUEUE_NOCAPACITY) {
    notFull->Wait();
  }
  q.push(x);
  notEmpty->NotifyOne();
  M->Unlock();
}

template <typename T>
const T &SynchQueue<T>::Dequeue() {
  M->Lock();
  while(q.empty()) {
    notEmpty->Wait();
  }
  T &x = q.front();
  q.pop();
  notFull->NotifyOne();
  M->Unlock();

  return x;
}

} //namespace threads

#endif //SYNCHQUEUE_H
