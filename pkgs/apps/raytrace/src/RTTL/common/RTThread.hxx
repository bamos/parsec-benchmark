#ifndef RTTL_THREAD_HXX
#define RTTL_THREAD_HXX

#include "RTInclude.hxx"

#include <pthread.h>

#if !defined(_WIN32)

typedef volatile int atomic_t;

#if defined(__sparc__) || defined(__sparc) || defined(sparc) || defined(__SPARC__)
#define ASI_P	0x80

#define casa(rs1, rs2, rd) ({                                      \
        u_int __rd = (uint32_t)(rd);                               \
        __asm __volatile("casa [%2] %3, %4, %0"                    \
            : "+r" (__rd), "=m" (*rs1)                             \
            : "r" (rs1), "n" (ASI_P), "r" (rs2), "m" (*rs1));        \
        __rd;                                                      \
})

_INLINE int atomic_add(atomic_t *v, const int c) {
  atomic_t e, r, s;                                              
  for (e = *v;; e = r) {                    
            s = e + c;                                             
            r = casa(v, e, s);                        
            if (r == e)                                             
                    break;                                          
  }                                                              
  return e;
}
#else
_INLINE int atomic_add(atomic_t *v, const int c) {
  int i = c;
  int __i = i;
  __asm__ __volatile__(
                       "lock ; xaddl %0, %1;"
                       :"=r"(i)
                        :"m"(*v), "0"(i));

  return i + __i;
}
#endif

_INLINE int atomic_inc(atomic_t *v) {
  return atomic_add(v,1);
} 

_INLINE int atomic_dec(atomic_t *v) {
  return atomic_add(v,-1);
} 


#else

typedef volatile LONG atomic_t;

_INLINE int atomic_add(atomic_t *v, const int c) 
{
  LONG value = InterlockedExchangeAdd(v,(LONG)c);
  return (int)value;
}

_INLINE int atomic_inc(atomic_t *v)
{
  LONG value = atomic_add(v,1);
  return (int)value;
}

_INLINE int atomic_dec(atomic_t *v)
{
  LONG value = atomic_add(v,-1);
  return (int)value;
}


#endif

/* class should have the right alignment to prevent cache trashing */
class AtomicCounter
{
private:
  atomic_t m_counter;
  char dummy[64-sizeof(atomic_t)]; // (iw) to make sure it's the only
                                   // counter sitting in its
                                   // cacheline....
public:

  AtomicCounter() {
    reset();
  }

  AtomicCounter(const int v) {
    m_counter = v;
  }

  _INLINE void reset() {
#if defined(_WIN32)
    m_counter = 0;
#else
    m_counter = -1;
#endif
  }

  _INLINE int inc() {
    return atomic_inc(&m_counter);
  }

  _INLINE int dec() {
    return atomic_dec(&m_counter);
  }

  _INLINE int add(const int i) {
    return atomic_add(&m_counter,i);
  }


};

#define DBG_THREAD(x) 


#ifdef NEEDS_PTHREAD_BARRIER_T_WRAPPER
class Barrier
{
  int total;
  int waiting;
  pthread_cond_t m_cond;
  pthread_mutex_t m_mutex;

public:
  void init(int count)
  {
    pthread_mutex_init(&m_mutex,NULL);
    pthread_cond_init(&m_cond,NULL);
    total = count; 
    waiting = 0;
  }
  void wait()
  {
    pthread_mutex_lock(&m_mutex);
    ++waiting;
    if (waiting == total)
      {
        pthread_cond_broadcast(&m_cond);
        waiting = 0;
      }
    else
      {
        pthread_cond_wait(&m_cond,&m_mutex);
      }
    pthread_mutex_unlock(&m_mutex);
  }
};
#else
class Barrier
{
protected:
  pthread_barrier_t m_barrier;
public:
  void init(int count)
  {
    pthread_barrier_init(&m_barrier,NULL,count);
  }
  void wait()
  {
    pthread_barrier_wait(&m_barrier);
  }
};
#endif

class MultiThreadedTaskQueueServer; /// Actually executes tasks
class MultiThreadedScheduler;       /// Allows suspending/resuming threads
class MultiThreadedTaskQueue        /// Externally visible
{
public:

  friend class MultiThreadedTaskQueueServer;

  enum {
    THREAD_EXIT,
    THREAD_RUNNING
  };

  /// Allows defining jobs in derived classes.
  virtual int task(int jobID, int threadID) {
    /* nothing to do */
    //PING;
    //cout << "THIS SHOULD NOT BE CALLED" << endl;
    return THREAD_RUNNING;
  }

  MultiThreadedTaskQueue() : m_threads(0) {}
  explicit MultiThreadedTaskQueue(int nt) : m_threads(nt) { if (nt) createThreads(nt); }
  ~MultiThreadedTaskQueue() {}

  /// Client differentiator: by default it is mapped to this pointer.
  /// Derived classes could use it to implement
  /// multi-phase processing.
  virtual long long int tag() const {
    return (long long int)this;
  }

  /// Set max # of threads handled by m_server;
  /// actual number executed inside this class (as defined by createThreads)
  /// may be different.
  static void setMaxNumberOfThreads(int threads);
  static int  maxNumberOfThreads();

  /// Request nthreads from server and execute task() in each
  /// (only after startThreads() is called).
  /// startThreads/waitForAllThreads pairs could be executed multiple times.
  void createThreads(int nthreads);
  /// Go!
  virtual void startThreads();
  /// Wait...
  virtual void waitForAllThreads();
  /// Start & wait
  void executeAllThreads() {
    startThreads();
    waitForAllThreads();
  }

  _INLINE int numberOfThreads() const { return m_threads; }

protected:

  volatile int m_assigned_jobs; // # of assigned jobs for client, not used for server
  volatile int m_finished_jobs; // # of completed jobs for client, marker for server
  int m_threads;                // # of jobs; fo reach job task() will be called with job id

  static MultiThreadedTaskQueueServer m_server;
  static MultiThreadedTaskQueue**     m_client;
  static MultiThreadedScheduler*      m_scheduler;

};

/// This class just allows suspending/resuming threads;
/// it is implemented on top of pthreads.
class MultiThreadedSyncPrimitive
{
public:
  MultiThreadedSyncPrimitive() {
    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_cond, NULL);
  }
  ~MultiThreadedSyncPrimitive() {
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);
  }

  // http://node1.yo-linux.com/cgi-bin/man2html?cgi_command=pthread_mutex_lock
  #if 1
  _INLINE int  lock()      { return pthread_mutex_lock(&m_mutex);         }
  _INLINE int  trylock()   { return pthread_mutex_trylock(&m_mutex);      }
  _INLINE int  unlock()    { return pthread_mutex_unlock(&m_mutex);       }
  _INLINE int  suspend()   { return pthread_cond_wait(&m_cond, &m_mutex); }
  _INLINE void resume()    { pthread_cond_signal(&m_cond);                }
  _INLINE void resumeAll() { pthread_cond_broadcast(&m_cond);             }
  #else
  _INLINE int  lock()      { cout << "L" << flush; return pthread_mutex_lock(&m_mutex);         }
  _INLINE int  trylock()   { cout << "T" << flush; return pthread_mutex_trylock(&m_mutex);      }
  _INLINE int  unlock()    { cout << "U" << flush; return pthread_mutex_unlock(&m_mutex);       }
  _INLINE int  suspend()   { cout << "S" << flush; return pthread_cond_wait(&m_cond, &m_mutex); }
  _INLINE void resume()    { cout << "R" << flush; pthread_cond_signal(&m_cond);                }
  _INLINE void resumeAll() { cout << "A" << flush; pthread_cond_broadcast(&m_cond);             }
  #endif

protected:
  pthread_mutex_t m_mutex;
  pthread_cond_t  m_cond;
};

#endif
