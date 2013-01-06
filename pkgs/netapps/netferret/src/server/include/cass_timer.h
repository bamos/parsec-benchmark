#ifndef _TIMER_H_
#define _TIMER_H_

#include <sys/time.h> 

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	struct	timeval	start, end; 
	float	diff; 
} stimer_t; 

void stimer_tick(stimer_t *timer); 

float stimer_tuck(stimer_t *timer, const char *msg); 

#ifdef __cplusplus
};
#endif

#endif /* _TIMER_H_ */
