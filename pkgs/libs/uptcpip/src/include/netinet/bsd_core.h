#ifndef _CORE_H
#define _CORE_H

extern int bsd_tick;			/* usec per tick (1000000 / hz) */
extern int bsd_hz;				/* system clock's frequency */
extern int psratio;			/* ratio: prof / stat */
extern int stathz;			/* statistics clock's frequency */
extern int profhz;			/* profiling clock's frequency */
extern int profprocs;			/* number of process's profiling */
extern int bsd_ticks;

#endif
