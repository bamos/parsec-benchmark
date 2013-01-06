#ifndef _CONFIG_H_
#define _CONFIG_H_

//Set to 1 to statically enable parallelization with pthreads
//#define ENABLE_PTHREADS 1

//Set to desired number of threads per queues
//The total number of queues between two pipeline stages will be
//greater or equal to #threads/MAX_THREADS_PER_QUEUE
#define MAX_THREADS_PER_QUEUE 4

//Set to 1 to add support with statistics collection
//Use argument `-v' to display statistics at end of runtime
#define ENABLE_STATISTICS 1

#endif //_CONFIG_H_

