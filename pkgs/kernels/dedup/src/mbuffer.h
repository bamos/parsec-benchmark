//Copyright 2009 Princeton University
//Written by Christian Bienia

/* This file contains methods and data structures to:
 *  - Allocate and manage memory buffers
 *  - Keep track of the usage of memory buffers and free them if necessary
 *  - Methods to break memory buffers into smaller buffers
 * The purpose of all this is to eliminate memory copies by reusing allocated memory as much as possible
 *
 * Note on use in multithreaded programs:
 * The routines expect that the caller takes care of guaranteeing exclusive access to the arguments
 * passed to the routines. Access to meta data that might be shared between multiple arguments (and
 * hence threads) will be synchronized. Obviously the user is responsible for synchronizing access
 * to the contents of the buffers.
 *
 * Note on allocating and freeing:
 * Two memory areas need to be distinguished: The mbuffer_t structure and the buffer that is encapsulated.
 * The subsystem can be used with both statically and dynamically allocated mbuffer_t structures. It will
 * always automatically free mbuffer_t structures it has allocated itself. Manually allocated mbuffer_t
 * structuers also need to be freed manually. The memory for the encapsulated buffer is always freed
 * automatically and needs to be dynamically allocated if manual allocation is used.
 */

#ifndef _MBUFFER_H_
#define _MBUFFER_H_

#include <stdlib.h>

//Add additional code to catch unallocated mbuffers and multiple frees
//#define ENABLE_MBUFFER_CHECK

#ifdef ENABLE_MBUFFER_CHECK
//random number to detect properly allocated & initialized mbuffers
#define MBUFFER_CHECK_MAGIC 4363097
#endif

//Definition of a memory control block (MCB) which tracks everything relevant for the correct use of malloc/free
//Dedup breaks memory buffers into smaller memory buffers during its operation, which means that free() cannot
//be called until all resulting buffers are no longer used. Furthermore we need to keep track of the original
//pointer returned by malloc & co so we know which one to pass to free().
typedef struct {
  unsigned int i; //reference counter
  void *ptr; //original pointer returned by malloc that needs to be passed to free()
} mcb_t;

//Definition of a memory buffer
typedef struct {
  void *ptr; //pointer to the buffer
  size_t n; //size of the buffer in bytes
  mcb_t *mcb; //meta information needed for malloc/free operations
#ifdef ENABLE_MBUFFER_CHECK
  int check_flag;
#endif
} mbuffer_t;



//Initialize memory buffer subsystem
int mbuffer_system_init();

//Shutdown memory buffer subsystem
int mbuffer_system_destroy();

//Initialize a memory buffer that has been manually or statically allocated
//The mbuffer system will not attempt to free argument *m
int mbuffer_create(mbuffer_t *m, size_t size);

//Make a shallow copy of a memory buffer
mbuffer_t *mbuffer_clone(mbuffer_t *m);

//Make a deep copy of a memory buffer
mbuffer_t *mbuffer_copy(mbuffer_t *m);

//Free a memory buffer
void mbuffer_free(mbuffer_t *m);

//Resize a memory buffer
//Returns 0 if the operation was successful
int mbuffer_realloc(mbuffer_t *m, size_t size);

//Split a memory buffer m1 into two buffers m1 and m2 at the designated location
//Returns 0 if the operation was successful
int mbuffer_split(mbuffer_t *m1, mbuffer_t *m2, size_t split);

#endif //_MBUFFER_H_

