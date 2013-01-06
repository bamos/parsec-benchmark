/*
 *    Implements dynamic task queues to provide load balancing
 *        Sanjeev Kumar --- December, 2004
 */

#ifndef __TASKQ_H__
#define __TASKQ_H__

#ifdef  __cplusplus
# define __BEGIN_DECLS  extern "C" {
# define __END_DECLS    }
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif


#define MAX_DIMENSION 3

__BEGIN_DECLS

//   An identifier that uniquely identifies a thread. The TaskQThreadId for the
//   main thread is 0. The TaskQThreadId ranges from 0 to "numThreads-1". So it
//   can be used to index arrays.

typedef long TaskQThreadId;

//   A task is a unit of work. A task has two componenets: A function (called
//   "task function" and a set of arguments (called "task arguments"). They
//   following are valid types for a task function.  When the task is executed
//   by the run time library, it executes the task function with the task's
//   arguments. In addition, it provides a threadId.
//   Note: The threadId that is provided to the task function (identity of the
//   thread executing the task) is not necessarily the same as the threadId
//   that was supplied to the taskQEnqueue function (which is the identity of
//   the thread enqueuing the task).

typedef void *TaskQTask; // Generic task type
typedef void (*TaskQTask1) (TaskQThreadId threadId, void *arg1);
typedef void (*TaskQTask2) (TaskQThreadId threadId, void *arg1, void *arg2);
typedef void (*TaskQTask3) (TaskQThreadId threadId, void *arg1, void *arg2, void *arg3);

//   These functions can be used to set the parameters in the task queue
//   implementations.  The different parameters affect different
//   implementations. Setting a parameter that is specific a particular
//   implementation is treated as a "nop" (as is therefore harmless) on the
//   other implementations.
//   These have to be called before TaskQInit to have an effect.
//   Usually you don't have to use this function---the default values will work
//   just fine.

enum TaskQParam { TaskQInvalidParam,
		  // parameters for Distributed implementations
		  TaskQThreadsPerQueue, // Number of threads per queue (default=1)
		  // End of parameters
		  TaskQNumParams
		};

void taskQSetParam (enum TaskQParam param, long value);
long taskQGetParam (enum TaskQParam param);

//   This function initializes the taskQ library and starts the worker threads.
//   This has to be called before calling any other taskQ functions (Except the
//   taskQ functions that set the various parameters).
//       numThreads: Total number of parallel threads (including the main
//           thread)
//       maxTasks: The maximum number of tasks that can be in the taskQ at any
//           point. Bad things happen when you specify this wrong (understate it).
//   This should be executed only once and that too by the main thread.

void taskQInit (int numThreads, int maxTasks);

//   These functions are used to enqueue a task. You have to call the
//   appropriate function depending on whether your task function takes 1, 2,
//   or 3 arguments.
//       taskFunction: The task function
//       threadId: This should be the TaskQThreadId of the thread that is
//           enqueuing the task (as stated above).  Note: It cannot be used to
//           specify the thread you want the task to executed on.
//       args[1..3]: The task arguments

void taskQEnqueueTask1 (TaskQTask1 taskFunction, TaskQThreadId threadId, void *arg1);
void taskQEnqueueTask2 (TaskQTask2 taskFunction, TaskQThreadId threadId, void *arg1, void *arg2);
void taskQEnqueueTask3 (TaskQTask3 taskFunction, TaskQThreadId threadId, void *arg1, void *arg2, void *arg3);

//   This function is used to enqueue a set of tasks at once. Suppose
//   you are operating on a grid (with at most MAX_DIMENSIONS). If you
//   want to break down the grid into tiles where each tile is an
//   independant task, you can enqueue all the tasks at once.
//        taskFunction: The task function. To satisfy the C type system, a type
//            cast should be used. For instance, if the numDimensions is 2, then
//            a task function (say taskFn2) of type TaskQTask2 should be defined
//            and cast into TaskQTask when it is supplied as an argument to
//            taskQEnqueueGrid.
//        numDimensions: The number of dimensions in the grid.
//        dimensionSize: Size of the grid in each dimension. Only indices 0 to
//            numDimensions-1 need to have valid values.
//        tileSize: Size of the tile in each dimension. Only indices 0 to
//            numDimensions-1 need to have valid values.

void taskQEnqueueGrid (TaskQTask taskFuntion, TaskQThreadId threadId, long numDimensions,
		       long dimensionSize[MAX_DIMENSION], long tileSize[MAX_DIMENSION]);

//   This function is used to wait for all the tasks that have been enqueued to
//   be compeleted.
//   Implementation Note: Currently, the worker threads (as well as the main
//   thread) start executing the tasks only after the taskQWait function is
//   invoked by the main thread. This might change in the future and so a
//   program should not rely on this for correctness. However, this is stated
//   here because it is sometimes easier to understand the behavior and
//   performance of the program when you know this.
//   This should be executed only  by the main thread.

void taskQWait (void);

//   This function returns a rough estimate of the number of tasks
//   that are currently queued up in the task queues.
//   Note 1:  This is just a hint.
//   Note 2:  This is currently implemented only for the Central Queues.
//   Note 3:  This is an experimental feature and might disappear in the future.

int taskQPendingTasksHint (void);

//   This function performs clean up actions and should be the last taskQ
//   function to be executed by the program.
//   This should be executed only once and that too by the main thread.

void taskQEnd (void);

//   These functions reset and print some statistics collected by taskQ library.
//   This should be executed only  by the main thread.

void taskQResetStats (void);
void taskQPrnStats (void);

__END_DECLS

//    These macros can be used to eliminate task queueing code entirely from
//    the program. This is useful to measure the performance of the serial
//    version of the program without any task queuing overhead. These work only
//    for the Baseline and the Grid flavors of task queues.

#ifdef SERIAL_TASKQ

#define taskQInit(nt,mt) { if ( nt != 1) { printf( "ERROR: Serial version. numThreads should be 1\n"); exit(1); } }
#define taskQEnqueueTask1(tf,tId,a)      { (tf)(0,a); }
#define taskQEnqueueTask2(tf,tId,a,b)    { (tf)(0,a,b); }
#define taskQEnqueueTask3(tf,tId,a,b,c)  { (tf)(0,a,b,c); }
#define taskQWait()
#define taskQResetStats()
#define taskQPrintStats()
#define taskQEnd()

#define taskQEnqueueGrid(tf,tId,nd,max,step) {                                                 \
    if ( MAX_DIMENSION != 3) {                                                                 \
        printf( "ERROR: Serial version not implemented for MAX_DIMENSION != 3.\n");            \
        exit(1);                                                                               \
    }                                                                                          \
    long __TQ__i, __TQ__j[MAX_DIMENSION], __TQ__max[MAX_DIMENSION], __TQ__step[MAX_DIMENSION]; \
    for ( __TQ__i = 0; __TQ__i < MAX_DIMENSION; __TQ__i++) {                                   \
        if ( __TQ__i < nd) {                                                                   \
            __TQ__max[__TQ__i] = max[__TQ__i];                                                 \
            __TQ__step[__TQ__i] = step[__TQ__i];                                               \
        } else {                                                                               \
            __TQ__max[__TQ__i] = 1;                                                            \
            __TQ__step[__TQ__i] = 1;                                                           \
        }                                                                                      \
    }                                                                                          \
    for ( __TQ__j[0] = 0; __TQ__j[0] < __TQ__max[0]; __TQ__j[0] += __TQ__step[0]) {            \
        for ( __TQ__j[1] = 0; __TQ__j[1] < __TQ__max[1]; __TQ__j[1] += __TQ__step[1]) {        \
            for ( __TQ__j[2] = 0; __TQ__j[2] < __TQ__max[2]; __TQ__j[2] += __TQ__step[2]) {    \
                tf( 0, ( void *)__TQ__j[0], ( void *)__TQ__j[1], ( void *)__TQ__j[2]);         \
            }                                                                                  \
        }                                                                                      \
    }                                                                                          \
}

#endif

#endif

