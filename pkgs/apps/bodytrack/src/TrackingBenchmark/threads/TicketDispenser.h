// (C) Copyright Christian Bienia 2007
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0.
//
//  file : TicketDispenser.h
//  author : Christian Bienia - cbienia@cs.princeton.edu
//  description : A synchronized counter used to issue tickets

#ifndef TICKETDISPENSER_H
#define TICKETDISPENSER_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "LockTypes.h"
#include "Mutex.h"


namespace threads {

template <typename T>
class TicketDispenser {
  public:
    //dispenser with Mutex, starting with (T)0, incrementing in steps of (T)1
    TicketDispenser();
    //dispenser with Mutex, starting with (T)0, custom increment steps
    TicketDispenser(T);
    //dispenser with Mutex, custom start value, custom increment steps
    TicketDispenser(T, T);
    //dispenser with custom lock, custom start value, custom increment steps
    TicketDispenser(T, T, LockType &);
    //destructor
    ~TicketDispenser();

    //get a ticket and increment counter (in that order)
    T getTicket();
    //reset internal counter to last start value
    void resetDispenser();
    //reset internal counter and use given value as new increment step
    void resetDispenser(T);
    //reset internal counter and use given values as new start value and increment step
    void resetDispenser(T, T);

  private:
    T init;
    T inc;
    T value;
    LockType *l;
};

//dispenser with Mutex, starting with (T)0, incrementing in steps of (T)1
template <typename T>
TicketDispenser<T>::TicketDispenser() {
  init = (T)0;
  inc = (T)1;
  l = new Mutex;
  value = init;
}

//dispenser with Mutex, starting with (T)0, custom increment steps
template <typename T>
TicketDispenser<T>::TicketDispenser(T _inc) {
  init = (T)0;
  inc = _inc;
  l = new Mutex;
  value = init;
}

//dispenser with Mutex, custom start value, custom increment steps
template <typename T>
TicketDispenser<T>::TicketDispenser(T _init, T _inc) {
  init = _init;
  inc = _inc;
  l = new Mutex;
  value = init;
}

//dispenser with custom lock, custom start value, custom increment steps
template <typename T>
TicketDispenser<T>::TicketDispenser(T _init, T _inc, LockType &_l) {
  init = _init;
  inc = _inc;
  l = &_l;
  value = init;
}

//destructor
template <typename T>
TicketDispenser<T>::~TicketDispenser() {
  delete l;
}

//get a ticket and increment counter (in that order)
template <typename T>
T TicketDispenser<T>::getTicket() {
  T rv;

  l->Lock();
  rv = value;
  value += inc;
  l->Unlock();

  return rv;
}

//reset internal counter to last start value
template <typename T>
void TicketDispenser<T>::resetDispenser() {
  l->Lock();
  value = init;
  l->Unlock();
}

//reset internal counter and use given value as new increment step
template <typename T>
void TicketDispenser<T>::resetDispenser(T _inc) {
  l->Lock();
  inc = _inc;
  value = init;
  l->Unlock();
}

//reset internal counter and use given values as new start value and increment step
template <typename T>
void TicketDispenser<T>::resetDispenser(T _init, T _inc) {
  l->Lock();
  init = _init;
  inc = _inc;
  value = init;
  l->Unlock();
}

} //namespace threads

#endif //TICKETDISPENSER_H
