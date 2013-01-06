
#ifndef __TASKQ_DISTLIST_H__
#define __TASKQ_DISTLIST_H__

#define VERSION "Fixed Arrays"

typedef struct
{
	volatile TaskQList     shared;
	char                   padding[CACHE_LINE_SIZE];
} TaskQDetails;

#endif
