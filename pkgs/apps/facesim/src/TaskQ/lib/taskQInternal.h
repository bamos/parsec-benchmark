/*
 *    Implements dynamic task queues to provide load balancing
 *        Sanjeev Kumar --- December, 2004
 */

#ifndef __TASKQ_INTERNAL_H__
#define __TASKQ_INTERNAL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef MAXFLOW
#include <taskQMaxflow.h>
#else
#include "../include/taskQ.h"
#endif

#ifdef USE_ATHREADS                               // Alamere threads
#include "taskQAThread.h"
#endif

#define CACHE_LINE_SIZE 256          /* Largest cache line size */

// #define HERE printf( ">>>  at  %s : %d\n", __FILE__, __LINE__)
// #define HERE printf( "%ld  $$$  at  %s : %d\n", ( long)alt_index(), __FILE__, __LINE__)
#define HERE

// #define TRACE printf( "%ld  ###  at  %s : %d\n", ( long)alt_index(), __FILE__, __LINE__); fflush( stdout);
// #define TRACE printf( ">>>  at  %s : %d\n", __FILE__, __LINE__)
// #define TRACE printf( ">>> %10d at  %s : %d\n", pthread_self(), __FILE__, __LINE__)
#define TRACE

#define ERROR { printf( "Error in TaskQ: This function should not have been called at  %s : %d\n", __FILE__, __LINE__); exit(0); }

// #define IF_STATS(s) s
#define IF_STATS(s)

#define TQ_ASSERT(v) {                                                   \
    if ( !(v)) {                                                         \
        printf( "TQ Assertion failed at %s:%d\n", __FILE__, __LINE__);   \
        exit(1);                                                         \
    }                                                                    \
}


#define UNDEFINED_VALUE ( ( void *)-999)

// #define DEBUG_TASKQ

#ifdef DEBUG_TASKQ
#define DEBUG_ASSERT(v) TQ_ASSERT(v)
#define IF_DEBUG(v) v
#define DEBUG_ANNOUNCE { printf( "\n\n>>>>>>>>>>>>>>>>>>>>>>   Running the DEBUG version of the TaskQ   <<<<<<<<<<<<<<<<<<<\n\n\n"); }
#else
#define DEBUG_ASSERT(v)
#define IF_DEBUG(v)
#define DEBUG_ANNOUNCE
#endif


#define NUM_FIELDS (MAX_DIMENSION+1)

static inline void copyTask (void *taskDest[NUM_FIELDS], void *taskSrc[NUM_FIELDS])
{
	int i;

	for (i = 0; i < NUM_FIELDS; i++)
		taskDest[i] = taskSrc[i];
}

static inline void copyArgs1 (void *task[NUM_FIELDS], TaskQTask1 taskFunction, void *arg1)
{
	TQ_ASSERT (NUM_FIELDS >= 2);
	task[0] = (void *) taskFunction;
	task[1] = arg1;
	IF_DEBUG ( { int i; for (i = 2; i < NUM_FIELDS; i++)    task[i] = UNDEFINED_VALUE; });
}

static inline void copyArgs2 (void *task[NUM_FIELDS], TaskQTask2 taskFunction, void *arg1, void *arg2)
{
	TQ_ASSERT (NUM_FIELDS >= 3);
	task[0] = (void *) taskFunction;
	task[1] = arg1;
	task[2] = arg2;
	IF_DEBUG ( { int i; for (i = 3; i < NUM_FIELDS; i++)    task[i] = UNDEFINED_VALUE; });
}

static inline void copyArgs3 (void *task[NUM_FIELDS], TaskQTask3 taskFunction, void *arg1, void *arg2, void *arg3)
{
	TQ_ASSERT (NUM_FIELDS >= 4);
	task[0] = (void *) taskFunction;
	task[1] = arg1;
	task[2] = arg2;
	task[3] = arg3;
	IF_DEBUG ( { int i; for (i = 4; i < NUM_FIELDS; i++)    task[i] = UNDEFINED_VALUE; });
}

// The following functions are used to enqueue a grid of tasks

typedef void (*AssignTasksFn) (TaskQTask3 taskFn, int numDimensions, int queueNo,
			       long min[MAX_DIMENSION], long max[MAX_DIMENSION], long step[MAX_DIMENSION]);

void standardize (int n, long min1[MAX_DIMENSION], long max1[MAX_DIMENSION], long step1[MAX_DIMENSION],
		  long min2[MAX_DIMENSION], long max2[MAX_DIMENSION], long step2[MAX_DIMENSION])
{
	long k;
	DEBUG_ASSERT (MAX_DIMENSION == 3);

	for (k = 0; k < MAX_DIMENSION; k++)
	{
		if (k < n)
		{
			min1[k] = min2[k];
			max1[k] = max2[k];
			step1[k] = step2[k];
		}
		else
		{
			min1[k] = 0;
			max1[k] = 1;
			step1[k] = 1;
		}
	}
}

static inline void assignTasksStandardized (AssignTasksFn fn, TaskQTask3 taskFn, int numDimensions, int queueNo,
		long min[MAX_DIMENSION], long max[MAX_DIMENSION], long step[MAX_DIMENSION])
{
	long min1[MAX_DIMENSION], max1[MAX_DIMENSION], step1[MAX_DIMENSION];
	standardize (numDimensions, min1, max1, step1, min, max, step);
	fn (taskFn, numDimensions, queueNo, min1, max1, step1);
}

static void enqueueGridRec (AssignTasksFn fn, TaskQTask3 taskFn,
			    int numDimensions, int startQueue, int totalQueues, int currentDimension,
			    long min[MAX_DIMENSION], long max[MAX_DIMENSION], long step[MAX_DIMENSION])
{
	if (totalQueues == 1)
	{
		assignTasksStandardized (fn, taskFn, numDimensions, startQueue, min, max, step);
	}
	else
	{
		long j, k;

		for (j = 0, k = currentDimension; j < numDimensions; j++, k = (k + 1) % numDimensions)
			if (max[k] - min[k] > 1)    break;

		if (max[k] - min[k] == 1)
		{
			// Only one task left. Put it in the first queue
			assignTasksStandardized (fn, taskFn, numDimensions, startQueue, min, max, step);

			// The rest get nothing
			for (j = 0; j < numDimensions; j++)
				max[j] = min[j];

			for (j = 1; j < totalQueues; j++)
				assignTasksStandardized (fn, taskFn, numDimensions, startQueue + j, min, max, step);
		}
		else     // Split it into two halfs and give half to each
		{
			long next, middle, half, alt[MAX_DIMENSION];
			middle = min[k] + (max[k] - min[k]) / 2;
			next = (k + 1) % numDimensions;
			half = totalQueues / 2;

			// First half
			for (j = 0; j < numDimensions; j++)
				if (j == k)
					alt[j] = middle;
				else
					alt[j] = max[j];

			enqueueGridRec (fn, taskFn, numDimensions, startQueue, half, next, min, alt, step);

			// Seconf half
			for (j = 0; j < numDimensions; j++)
				if (j == k)
					alt[j] = middle;
				else
					alt[j] = min[j];

			enqueueGridRec (fn, taskFn, numDimensions, startQueue + half, totalQueues - half, next, alt, max, step);
		}
	}
}

long countTasks (int n, long max[MAX_DIMENSION], long step[MAX_DIMENSION])
{
	long k, count = 1;

	for (k = 0; k < n; k++)    count *= max[k] / step[k];

	return count;
}

static inline void enqueueGridHelper (AssignTasksFn assignFunction, TaskQTask3 taskFunction, int numDimensions, int numTaskQs,
				      long dimensionSize[MAX_DIMENSION], long tileSize[MAX_DIMENSION])
{
	long i, min[MAX_DIMENSION], max[MAX_DIMENSION];

	for (i = 0; i < numDimensions; i++)
	{
		DEBUG_ASSERT (dimensionSize[i] > 0);
		TQ_ASSERT (dimensionSize[i] % tileSize[i] == 0);
		min[i] = 0;
		max[i] = dimensionSize[i] / tileSize[i];
	}

	enqueueGridRec (assignFunction, taskFunction, numDimensions, 0, numTaskQs, 0, min, max, tileSize);
}


//  These are used to get/set task queue parameters

static long paramValues[] = { (long) UNDEFINED_VALUE, 1, (long) UNDEFINED_VALUE };

void taskQSetParam (enum TaskQParam param, long value)
{
	TQ_ASSERT ( (param > 0) && (param < TaskQNumParams));
	TQ_ASSERT (paramValues[TaskQNumParams] == (long) UNDEFINED_VALUE);
	paramValues[param] = value;
}

long taskQGetParam (enum TaskQParam param)
{
	TQ_ASSERT ( (param > 0) && (param < TaskQNumParams));
	TQ_ASSERT (paramValues[TaskQNumParams] == (long) UNDEFINED_VALUE);
	return paramValues[param];
}


#endif
