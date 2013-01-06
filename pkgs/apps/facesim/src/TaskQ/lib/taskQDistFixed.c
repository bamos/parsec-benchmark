static void initTaskQDetails( TaskQ *t) {
    TaskQList *shared =  ( TaskQList *)&t->q.shared;
    initTaskQList( shared, maxTasks);
}

static inline int getATaskFromHead( TaskQ *t, void *task[NUM_FIELDS]) {
    TaskQList *shared =  ( TaskQList *)&t->q.shared;
    int found;

#ifdef ENABLE_PTHREADS
    pthread_mutex_lock(&(t->lock));;
#endif //ENABLE_PTHREADS
    found = shared->count;
    if ( found) {
        IF_STATS( t->statLocal++);
        getEntryHead( shared, task);
    }
#ifdef ENABLE_PTHREADS
    pthread_mutex_unlock(&(t->lock));;
#endif //ENABLE_PTHREADS
    return found;
}

static inline int stealTasksSpecialized( TaskQ *myT, TaskQ *srcT) {
    TaskQList *srcShared =  ( TaskQList *)&srcT->q.shared;
    volatile int *srcCount = &srcShared->count;
    if ( *srcCount == 0)    return 0; // Quick Check. Unprotected.

    TaskQList *myShared =  ( TaskQList *)&myT->q.shared;

#ifdef ENABLE_PTHREADS
    // Grab to locks in a total order to avoid deadlock
    if ( ( long)myT < ( long)srcT)   { pthread_mutex_lock(&(myT->lock));;  pthread_mutex_lock(&(srcT->lock));; } 
    else                             { pthread_mutex_lock(&(srcT->lock));; pthread_mutex_lock(&(myT->lock));;  }
#endif //ENABLE_PTHREADS

    int toSteal = 0;
    int found = ( int)*srcCount;
    if ( found) {
        toSteal = calculateNumSteal( found);
        moveEntriesFromTailToHead( srcShared, myShared, toSteal);
    }
#ifdef ENABLE_PTHREADS
    pthread_mutex_unlock(&(myT->lock));;
    pthread_mutex_unlock(&(srcT->lock));;
#endif //ENABLE_PTHREADS

    return toSteal;
}

#if 0
static inline int getTasksFromTail( TaskQ *t, void *tasks[MAX_TASKS_TO_STEAL][NUM_FIELDS]){
    TaskQList *shared =  ( TaskQList *)&t->q.shared;
    volatile int *count = &shared->count;
    if ( *count == 0)    return 0; // Quick Check. Unprotected.
    int found,num_tasks_stolen=0;

#ifdef ENABLE_PTHREADS
    pthread_mutex_lock(&(t->lock));;
#endif //ENABLE_PTHREADS
    found = ( int)*count;
    if (found)
        num_tasks_stolen=getEntriesTail(shared,tasks);
#ifdef ENABLE_PTHREADS
    pthread_mutex_unlock(&(t->lock));;
#endif //ENABLE_PTHREADS

    return num_tasks_stolen;
}
#endif

static inline void taskQEnqueueTaskSpecialized( TaskQ *t, void *task[NUM_FIELDS]) {
    TaskQList *shared =  ( TaskQList *)&t->q.shared;
#ifdef ENABLE_PTHREADS
    pthread_mutex_lock(&(t->lock));;
#endif //ENABLE_PTHREADS
    putEntryHead( shared, task);
    IF_STATS( t->statEnqueued++);
#ifdef ENABLE_PTHREADS
    pthread_mutex_unlock(&(t->lock));;
#endif //ENABLE_PTHREADS
}

static inline void assignTasks( TaskQTask3 taskFn, int numDimensions, int queueNo, 
                                long min[MAX_DIMENSION], long max[MAX_DIMENSION], long step[MAX_DIMENSION]) {
    TaskQ *t = &taskQs[queueNo];
    TaskQList *shared =  ( TaskQList *)&t->q.shared;
    long i, j, k;

    // printf( "-----------   %4d %4d      ------------------------------\n", numDimensions, queueNo);
    // for ( j = 0; j < numDimensions; j++) printf( "       %ld :: %4ld %4ld %4ld\n", j, min[j], max[j], step[j]);

#ifdef ENABLE_PTHREADS
    pthread_mutex_lock(&(t->lock));;
#endif //ENABLE_PTHREADS
    for ( i = min[0]; i < max[0]; i++)
        for ( j = min[1]; j < max[1]; j++)
            for ( k = min[2]; k < max[2]; k++) {
                IF_STATS( t->statEnqueued++);
                void *task[NUM_FIELDS];
                task[0] = ( void *)taskFn;
                task[1] = ( void *)( i * step[0]);
                task[2] = ( void *)( j * step[1]);
                task[3] = ( void *)( k * step[2]);
                // long m; for ( m = 0; m < NUM_FIELDS; m++) { printf( "%10ld", ( long)task[m]); } printf( "\n");
                putEntryHead( shared, task);
                
            }
#ifdef ENABLE_PTHREADS
    pthread_mutex_unlock(&(t->lock));;
#endif //ENABLE_PTHREADS
}

static inline void taskQEnqueueGridSpecialized( TaskQTask3 taskFunction, TaskQThreadId threadId, 
                                                int numOfDimensions, long tileSize[MAX_DIMENSION]) {
    TQ_ASSERT( MAX_DIMENSION == 3); // assignTasks assumes this
    TQ_ASSERT( ( threadId == 0) && ( parallelRegion == 0)); // Since we are enqueuing tasks in other threads
}
