/*************************************************************************/
/*                                                                       */
/*  Copyright (c) 1994 Stanford University                               */
/*                                                                       */
/*  All rights reserved.                                                 */
/*                                                                       */
/*  Permission is given to use, copy, and modify this software for any   */
/*  non-commercial purpose as long as this copyright notice is not       */
/*  removed.  All other uses, including redistribution in whole or in    */
/*  part, are forbidden without prior written permission.                */
/*                                                                       */
/*  This software is provided with absolutely no warranty and no         */
/*  support.                                                             */
/*                                                                       */
/*************************************************************************/


#ifndef _TASK_H
#define _TASK_H


/************************************************************************
*
*     Constants
*
*************************************************************************/

#define PAGE_SIZE 4096   /* page size of system, used for padding to 
allow page placement of some logically 
per-process data structures */

/*** Task types ***/
#define TASK_MODELING      (1)
#define TASK_BSP           (2)
#define TASK_FF_REFINEMENT (4)
#define TASK_RAY           (8)
#define TASK_RAD_AVERAGE   (16)
#define TASK_VISIBILITY    (32)


/*** Controling parallelism ***/

#define MAX_TASKGET_RETRY (32)	    /* Max # of retry get_task() can make */
#define N_ALLOCATE_LOCAL_TASK (8)   /* get_task() and free_task() transfer
this # of task objects to/from the
global shared queue at a time */


/************************************************************************
*
*     Task Descriptors
*
*************************************************************************/

/* Decompose modeling object into patches (B-reps) */
typedef struct {
    int   type ;		     /* Object type */
    Model *model ;		     /* Object to be decomposed */
} Modeling_Task ;


/* Insert a new patch to the BSP tree */
typedef struct {
    Patch *patch ;                 /* Patch to be inserted */
    Patch *parent ;		     /* Parent node in the BSP tree */
} BSP_Task ;


/* Refine element interaction based on FF value or BF value */
typedef struct {
    Element *e1, *e2 ;	     /* Interacting elements */
    float   visibility ;           /* Visibility of parent */
    int level ;		     /* Path length from the root element */
} Refinement_Task ;


typedef struct {
    int  ray_type ;
    Element *e ;		     /* The element we are interested in */
} Ray_Task ;


typedef struct {
    Element *e ;		     /* The element we are interested in */
    Interaction *inter ;	     /* Top of interactions */
    int   n_inter ;		     /* Number of interactions */
    void  (*k)() ;		     /* Continuation */
} Visibility_Task ;

/* Radiosity averaging task */

#define RAD_AVERAGING_MODE (0)
#define RAD_NORMALIZING_MODE (1)

typedef struct {
    Element *e ;
    int level ;
    int mode ;
} RadAvg_Task ;



/************************************************************************
*
*     Task Definition
*
*************************************************************************/


typedef struct _task {
    int task_type ;
    struct _task *next ;
    union {
        Modeling_Task   model ;
        BSP_Task        bsp ;
        Refinement_Task ref ;
        Ray_Task        ray ;
        Visibility_Task vis ;
        RadAvg_Task     rad ;
    } task ;
} Task ;


typedef struct {
    char pad1[PAGE_SIZE];	 	/* padding to avoid false-sharing 
    and allow page-placement */	
    LOCKDEC(q_lock)
    Task  *top, *tail ;
    int   n_tasks ;
    LOCKDEC(f_lock)
    int   n_free ;
    Task  *free ;
    char pad2[PAGE_SIZE];	 	/* padding to avoid false-sharing 
    and allow page-placement */	
} Task_Queue ;


#define TASK_APPEND (0)
#define TASK_INSERT (1)


extern Task *get_task() ;
extern void free_task() ;
extern void enqueue_task() ;
extern Task *dequeue_task(), *dequeue_neighbor_task() ;
#define taskq_length(q)   (q->n_tasks)
#define taskq_top(q)      (q->top)
extern void print_task(), print_taskq() ;
extern int assign_taskq() ;
extern void init_taskq() ;
#define taskq_too_long(q)  ((q)->n_tasks > n_tasks_per_queue)

extern void process_tasks() ;
extern void create_modeling_task(), create_bsp_task() ;
extern void create_ff_refine_task() ;
extern void create_ray_task(), create_radavg_task() ;
extern void create_visibility_tasks() ;
extern void enqueue_ray_task() ;
extern void enqueue_radavg_task() ;
extern int  check_task_counter() ;


#endif

