/*
 *    Implements dynamic task queues to provide load balancing
 *        Sanjeev Kumar --- December, 2004
 */


#include <time.h>
#include "config.h"

#ifdef ENABLE_PTHREADS
#include "alamere.h"
pthread_t _M4_threadsTable[MAX_THREADS];
int _M4_threadsTableAllocated[MAX_THREADS];
pthread_mutexattr_t _M4_normalMutexAttr;
#endif //ENABLE_PTHREADS

#include "taskQInternal.h"
#include "taskQList.h"

static volatile long          numThreads, numTaskQs, threadsPerTaskQ, maxTasks;
static volatile int           nextQ = 0;  // Just a hint. Not protected by locks.
static volatile int           parallelRegion = 0;
static volatile int           noMoreTasks = 0;

#if defined( TASKQ_DIST_GRID)
#include "taskQDistGrid.h"
#elif defined( TASKQ_DIST_LIST)
#include "taskQDistList.h"
#elif defined( TASKQ_DIST_FIXED)
#include "taskQDistFixed.h"
#else
#error "Missing Definition"
#endif

typedef struct {
#ifdef ENABLE_PTHREADS
    pthread_mutex_t lock;
#endif //ENABLE_PTHREADS
    long                   statEnqueued, statLocal, *statStolen;
    char                   padding1[CACHE_LINE_SIZE];
    TaskQDetails           q;
    char                   padding2[CACHE_LINE_SIZE];
} TaskQ;

typedef struct {
#ifdef ENABLE_PTHREADS
    pthread_mutex_t lock;
    pthread_cond_t taskAvail;
    pthread_cond_t tasksDone;
#endif //ENABLE_PTHREADS
    volatile long         threadCount;
} Sync;

TaskQ *taskQs;
Sync  sync;

#define MAX_STEAL 8
static inline int calculateNumSteal( int available) {
    int half = ( available == 1) ? 1 : available/2;
    return ( half > MAX_STEAL) ? MAX_STEAL : half;
}

#if defined( TASKQ_DIST_GRID)
#include "taskQDistGrid.c"
#elif defined( TASKQ_DIST_LIST)
#include "taskQDistList.c"
#elif defined( TASKQ_DIST_FIXED)
#include "taskQDistFixed.c"
#else
#error "Missing Definition"
#endif

static void waitForTasks( void) {
    TRACE;
#ifdef ENABLE_PTHREADS
    pthread_mutex_lock(&(sync.lock));;
    sync.threadCount++;
    if ( sync.threadCount == numThreads)
        pthread_cond_broadcast(&sync.tasksDone);;
    pthread_cond_wait(&sync.taskAvail,&sync.lock);  pthread_mutex_unlock(&sync.lock);;
#else
    sync.threadCount++;
#endif //ENABLE_PTHREADS
    TRACE;
}

static void signalTasks( void) {
    TRACE;
    if ( sync.threadCount == 0)    return;    // Unsafe
#ifdef ENABLE_PTHREADS
    pthread_mutex_lock(&(sync.lock));;
    sync.threadCount = 0;
    pthread_cond_broadcast(&sync.taskAvail);;
    pthread_cond_broadcast(&sync.tasksDone);;
    pthread_mutex_unlock(&(sync.lock));;    
#else
    sync.threadCount = 0;
#endif //ENABLE_PTHREADS
    TRACE;
}

static int waitForEnd( void) {
    int done;
    TRACE;
#ifdef ENABLE_PTHREADS
    pthread_mutex_lock(&(sync.lock));;
    sync.threadCount++;
    if ( sync.threadCount != numThreads)
        pthread_cond_wait(&sync.tasksDone,&sync.lock);;
    done = (sync.threadCount == numThreads);
    pthread_mutex_unlock(&(sync.lock));;
#else
    sync.threadCount++;
    done = (sync.threadCount == numThreads);
#endif //ENABLE_PTHREADS
    TRACE;

    return done;
}

static int doOwnTasks( long myThreadId, long myQ) {
    void *task[NUM_FIELDS];
    int executed = 0;

    TRACE;
    while ( getATaskFromHead( &taskQs[myQ], task)) {
        ( ( TaskQTask3)task[0])( myThreadId, task[1], task[2], task[3]);
        executed = 1;
    }
    TRACE;
    return executed;
}

static int stealTasks( long myThreadId, long myQ) {
    int i, stolen = 0;

    TRACE;
    i = myQ + 1;
    while ( 1 ) {
        if ( i == numTaskQs)    i = 0;
        if ( i == myQ)    break;

        stolen = stealTasksSpecialized( &taskQs[myQ], &taskQs[i]);
        if( stolen) {
            IF_STATS(  {
#ifdef ENABLE_PTHREADS
                pthread_mutex_lock(&(taskQs[myQ].lock));;
                taskQs[myQ].statStolen[i] += stolen;
                pthread_mutex_unlock(&(taskQs[myQ].lock));;
#else
                taskQs[myQ].statStolen[i] += stolen;
#endif //ENABLE_PTHREADS
            });
            break;
        } else {
            i++;
        }
    }
    TRACE;
    return stolen;
}

static void *taskQIdleLoop( void *arg) {
    long index = ( long)arg;
    long myQ = index / threadsPerTaskQ;
    int i = 0;
    int stolen;

    while ( 1 ) {
        waitForTasks();
        if ( noMoreTasks)    return 0;
        doOwnTasks( index, myQ);
        while ( 1) {
            stolen = stealTasks( index, myQ);
            if ( stolen)
                doOwnTasks( index, myQ);
            else
                break;
        }
        i++;
    }
}

void taskQInit( int numOfThreads, int maxNumOfTasks) {
    int i;

#ifdef ENABLE_PTHREADS    
    ALAMERE_INIT(numOfThreads);
    ALAMERE_AFTER_CHECKPOINT();
    pthread_mutexattr_init( &_M4_normalMutexAttr);
    pthread_mutexattr_settype( &_M4_normalMutexAttr, PTHREAD_MUTEX_NORMAL);
    {
        int _M4_i;
        for ( _M4_i = 0; _M4_i < MAX_THREADS; _M4_i++) {
            _M4_threadsTableAllocated[_M4_i] = 0;
        }
    }
#endif //ENABLE_PTHREADS
;

    maxTasks = maxNumOfTasks;
    numThreads = numOfThreads;
    threadsPerTaskQ = taskQGetParam( TaskQThreadsPerQueue);
    DEBUG_ASSERT( ( numThreads >= 1) && ( threadsPerTaskQ >= 1));

    numTaskQs = (numOfThreads+threadsPerTaskQ-1)/threadsPerTaskQ;

    /*
    printf( "\n\n\t#####  Running TaskQ version Distributed %-15s with %ld threads and %ld queues  #####\n", 
            VERSION,  numThreads, numTaskQs);
    printf( "\t##### \t\t\t\t\t\t[ built on %s at %s ]  #####\n\n", __DATE__, __TIME__);
    printf( "\t\t TaskQ mutex address                 :  %ld\n", ( long)&sync.lock);
    printf( "\t\t TaskQ condition variable 1 address  :  %ld\n", ( long)&sync.taskAvail);
    printf( "\t\t TaskQ condition variable 2 address  :  %ld\n", ( long)&sync.tasksDone);
    printf( "\n\n");
    */
    DEBUG_ANNOUNCE;

    taskQs = ( TaskQ *)malloc( sizeof( TaskQ) * numTaskQs);
    for ( i = 0; i < numTaskQs; i++) {
#ifdef ENABLE_PTHREADS
        pthread_mutex_init(&(taskQs[i].lock), NULL);;
#endif //ENABLE_PTHREADS
        taskQs[i].statStolen = ( long *)malloc( sizeof(long) * numTaskQs);

        initTaskQDetails( &taskQs[i]);
    }
    taskQResetStats();

#ifdef ENABLE_PTHREADS
    pthread_mutex_init(&(sync.lock), NULL);;
    pthread_cond_init(&sync.taskAvail,NULL);;
    pthread_cond_init(&sync.tasksDone,NULL);;
#endif //ENABLE_PTHREADS
    sync.threadCount = 0;

#ifdef ENABLE_PTHREADS
    for ( i = 1; i < numThreads; i++)
    {
        int _M4_i;
        for ( _M4_i = 0; _M4_i < MAX_THREADS; _M4_i++) {
            if ( _M4_threadsTableAllocated[_M4_i] == 0)    break;
        }
        pthread_create(&_M4_threadsTable[_M4_i],NULL,(void *(*)(void *))taskQIdleLoop,(void *)( long)i);
        _M4_threadsTableAllocated[_M4_i] = 1;
    }
#endif //ENABLE_PTHREADS
;

    waitForEnd();
}

static inline int pickQueue( int threadId) { // Needs work
    if ( parallelRegion)    return threadId/threadsPerTaskQ;
    int q = nextQ;
    int p = q+1;
    nextQ = ( p >= numTaskQs) ? 0 : p;
    DEBUG_ASSERT( q < numTaskQs);
    return q;
}

void taskQEnqueueGrid( TaskQTask taskFunction, TaskQThreadId threadId, long numOfDimensions, 
                       long dimensionSize[MAX_DIMENSION], long tileSize[MAX_DIMENSION]) { 
    taskQEnqueueGridSpecialized( ( TaskQTask3)taskFunction, threadId, numOfDimensions, tileSize);
    enqueueGridHelper( assignTasks, ( TaskQTask3)taskFunction, numOfDimensions, numTaskQs, dimensionSize, tileSize); 
    if ( parallelRegion)    signalTasks();
}


static inline void taskQEnqueueTaskHelper( int threadId, void *task[NUM_FIELDS]) {
    TRACE;
    int queueNo = pickQueue( threadId);
    // if ( !parallelRegion)  printf( "%30ld %20ld\n", ( long)task[1], ( long)queueNo);
    taskQEnqueueTaskSpecialized( &taskQs[queueNo], task);
    if ( parallelRegion)    signalTasks();
}

void taskQEnqueueTask1( TaskQTask1 taskFunction, TaskQThreadId threadId, void *arg1) {
    TRACE;
    void *task[NUM_FIELDS];
    copyArgs1( task, taskFunction, arg1);
    taskQEnqueueTaskHelper( threadId, task);
}

void taskQEnqueueTask2( TaskQTask2 taskFunction, TaskQThreadId threadId, void *arg1, void *arg2) {
    TRACE;
    void *task[NUM_FIELDS];
    copyArgs2( task, taskFunction, arg1, arg2);
    taskQEnqueueTaskHelper( threadId, task);
}

void taskQEnqueueTask3( TaskQTask3 taskFunction, TaskQThreadId threadId, void *arg1, void *arg2, void *arg3) {
    TRACE;
    void *task[NUM_FIELDS];
    copyArgs3( task, taskFunction, arg1, arg2, arg3);
    taskQEnqueueTaskHelper( threadId, task);
}

void taskQWait( void) {
    static int i = 0;
    int done;
    parallelRegion = 1;
    signalTasks();
    TRACE;
    do {
        doOwnTasks( 0, 0);
        stealTasks( 0, 0);
        doOwnTasks( 0, 0);
        done = waitForEnd();
    } while (!done);
    parallelRegion = 0;
    TRACE;
    i++;
}

void taskQResetStats() {
    IF_STATS( {
        int i; int j;
        for ( i = 0; i < numTaskQs; i++) {
            taskQs[i].statEnqueued = 0;
            taskQs[i].statLocal = 0;
            for ( j = 0; j < numTaskQs; j++) {
                taskQs[i].statStolen[j] = 0;
            }
        }
    });
}

void taskQPrnStats() {
    IF_STATS( {
        long j; long i; long total1 = 0; long total2 = 0;
        printf( "\n\n\t#####  Cumulative statistics from Task Queues #####\n\n");
        printf( "\t\t%-10s    %-10s %-10s %-10s %-10s\n\n", "Queue", "Enqueued", "Local", "Stolen", "Executed");
        for ( j = 0; j < numTaskQs; j++) {
            long totalStolen = 0;
            for ( i = 0; i < numTaskQs; i++)    totalStolen += taskQs[j].statStolen[i];
            printf( "\t\t%5ld    %10ld %10ld %10ld %10ld\n", j, taskQs[j].statEnqueued, taskQs[j].statLocal-totalStolen, totalStolen, taskQs[j].statLocal);
            total1 += taskQs[j].statEnqueued;
            total2 += taskQs[j].statLocal;
        }
        printf( "\t\t%5s    %10ld %10s %10s %10ld\n", "Total", total1, "", "", total2);
        printf( "\n\n");

        printf( "\tBreakdown of Task Stealing\n\n\t%10s", "");
        for ( i = 0; i < numTaskQs; i++)  { printf( "%8ld Q", i); }
        printf( "\n\n");
        for ( j = 0; j < numTaskQs; j++) {
            printf( "\t%8ld T", j);
            for ( i = 0; i < numTaskQs; i++) {
                printf( "%10ld", taskQs[j].statStolen[i]);
            }
            printf( "\n");
        }
        printf( "\n\n");
    });
}

void taskQEnd( void) {
    noMoreTasks = 1;
    signalTasks();

#ifdef ENABLE_PTHREADS
    ALAMERE_END();;
#endif //ENABLE_PTHREADS
}
