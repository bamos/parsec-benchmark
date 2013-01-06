// (C) Copyright Christian Bienia 2007
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0.
//
//  file : LockTypes.h
//  author : Christian Bienia - cbienia@cs.princeton.edu
//  description : Several abstract lock types

#ifndef LOCKTYPES_H
#define LOCKTYPES_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif


namespace threads {

//A generic lock
class LockType {
  public:
    virtual ~LockType() {};

    //Enter a critical region
    virtual void Lock() =0;
    //Leave a critical region
    virtual void Unlock() =0;
    //Try to acquire the lock, return true if successful
    virtual bool TryLock() =0;
};

//A read-write lock
class RWLockType: public LockType {
  public:
    virtual ~RWLockType() {};

    //Enter a critical region for reading
    virtual void ReadLock() =0;
    //Try to acquire the lock for reading, return true if successful
    virtual bool TryReadLock() =0;
    //Enter a critical region for writing
    virtual void WriteLock() =0;
    //Try to acquire the lock for writing, return true if successful
    virtual bool TryWriteLock() =0;
    //Leave a critical region
    virtual void Unlock() =0;

    //Using the inherited methods of LockType is equivalent to using the
    //*WriteLock methods and will result in the same behavior
    void Lock() {WriteLock();}
    bool TryLock() {return TryWriteLock();}
};




} //namespace threads

#endif //LOCKTYPES_H
