#ifndef __TASKQ_TASKQ_LIST_H__
#define __TASKQ_TASKQ_LIST_H__

typedef struct
{
	void                   *args[NUM_FIELDS];
} Entry;

typedef struct
{
	int           head, tail, size, count;
	Entry         *array;
} TaskQList;

#define INDEX_INC(i,max) { if ( ++(i) == max)    (i) = 0; }
#define INDEX_DEC(i,max) { if ( --(i) < 0)       (i) = max-1; }

static inline void copyArgs (void *dest[NUM_FIELDS], void *src[NUM_FIELDS])
{
	TQ_ASSERT (NUM_FIELDS == 4);
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
	dest[3] = src[3];
}

static void initTaskQList (TaskQList *t, long maxNumOfTasks)
{
	t->head = t->tail = t->count = 0;
	t->size = maxNumOfTasks;
	t->array = (Entry *) malloc (t->size * sizeof (Entry));
}

static inline void getEntryHead (TaskQList *t, void *task[NUM_FIELDS])
{
	t->count--;
	INDEX_DEC (t->head, t->size);
	void **args = t->array[t->head].args;
	copyArgs (task, args);
}

static inline void getEntryTail (TaskQList *t, void *task[NUM_FIELDS])
{
	t->count--;
	void **args = t->array[t->tail].args;
	copyArgs (task, args);
	INDEX_INC (t->tail, t->size);
}

static inline void putEntryHead (TaskQList *t, void *task[NUM_FIELDS])
{
	t->count++;
	DEBUG_ASSERT (t->count <= t->size);
	void **args = t->array[t->head].args;
	copyArgs (args, task);
	INDEX_INC (t->head, t->size);
}

static inline void putEntryTail (TaskQList *t, void *task[NUM_FIELDS])
{
	t->count++;
	DEBUG_ASSERT (t->count <= t->size);
	INDEX_DEC (t->tail, t->size);
	void **args = t->array[t->tail].args;
	copyArgs (args, task);
}

static inline int moveEntriesFromHeadToTail (TaskQList *src, TaskQList *dest, long maxTasks)
{
	int toMove = (src->count < maxTasks) ? src->count : maxTasks;
	int srcHead = src->head;
	int destTail = dest->tail;
	int srcSize = src->size;
	int destSize = dest->size;
	int moved = toMove;

	while (toMove-- != 0)
	{
		INDEX_DEC (destTail, destSize);
		INDEX_DEC (srcHead, srcSize);
		void **destArgs = dest->array[destTail].args;
		void **srcArgs = src->array[srcHead].args;
		copyArgs (destArgs, srcArgs);
	}

	dest->tail = destTail;
	dest->count += moved;
	src->head = srcHead;
	src->count -= moved;
	DEBUG_ASSERT (dest->count <= dest->size);
	DEBUG_ASSERT (src->count >= 0);
	return moved;
}

static inline int moveEntriesFromTailToTail (TaskQList *src, TaskQList *dest, long maxTasks)
{
	int toMove = (src->count < maxTasks) ? src->count : maxTasks;
	int srcTail = src->tail;
	int destTail = dest->tail;
	int srcSize = src->size;
	int destSize = dest->size;
	int moved = toMove;

	while (toMove-- != 0)
	{
		INDEX_DEC (destTail, destSize);
		void **destArgs = dest->array[destTail].args;
		void **srcArgs = src->array[srcTail].args;
		copyArgs (destArgs, srcArgs);
		INDEX_INC (srcTail, srcSize);
	}

	dest->tail = destTail;
	dest->count += moved;
	src->tail = srcTail;
	src->count -= moved;
	DEBUG_ASSERT (dest->count <= dest->size);
	DEBUG_ASSERT (src->count >= 0);
	return moved;
}

static inline int moveEntriesFromTailToHead (TaskQList *src, TaskQList *dest, long maxTasks)
{
	int toMove = (src->count < maxTasks) ? src->count : maxTasks;
	int srcTail = src->tail;
	int destHead = dest->head;
	int srcSize = src->size;
	int destSize = dest->size;
	int moved = toMove;

	while (toMove-- != 0)
	{
		void **destArgs = dest->array[destHead].args;
		void **srcArgs = src->array[srcTail].args;
		copyArgs (destArgs, srcArgs);
		INDEX_INC (destHead, destSize);
		INDEX_INC (srcTail, srcSize);
	}

	dest->head = destHead;
	dest->count += moved;
	src->tail = srcTail;
	src->count -= moved;
	DEBUG_ASSERT (dest->count <= dest->size);
	DEBUG_ASSERT (src->count >= 0);
	return moved;
}

//------------------------------------------------------  CLEAN THIS UP. MAX_TASKS_TO_STEAL => MAX_MOVE_TASKS

#define MAX_TASKS_TO_STEAL 8

#ifdef USE_INFORMATION
static __inline void INFORMATION (int arg1, int arg2, int arg3)
{
	__asm__ volatile ("information %[r1], %[r2], %[r3]"
			  :
			  : [r1] "r" (arg1), [r2] "r" (arg2), [r3] "r" (arg3));
}
#endif

static inline int getEntriesTail (TaskQList *t, void *tasks[MAX_TASKS_TO_STEAL][NUM_FIELDS])
{
	int num_to_get = t->count / 2 <= MAX_TASKS_TO_STEAL ? t->count / 2 : MAX_TASKS_TO_STEAL;
	int i;
	void **args;
	t->count -= num_to_get;

	for (i = 0; i < num_to_get; i++)
	{
		args = t->array[t->tail].args;
		copyArgs (tasks[i], args);
		INDEX_INC (t->tail, t->size);
	}

	return num_to_get;
}

static inline void putEntriesHead (TaskQList *t, void *tasks[MAX_TASKS_TO_STEAL][NUM_FIELDS], int num_tasks)
{
	int i;
	void **args;
	t->count += num_tasks;
	DEBUG_ASSERT (t->count <= t->size);

	for (i = 0; i < num_tasks; i++)
	{
		args = t->array[t->head].args;
		copyArgs (args, tasks[i]);
		INDEX_INC (t->head, t->size);
	}
}


#endif
