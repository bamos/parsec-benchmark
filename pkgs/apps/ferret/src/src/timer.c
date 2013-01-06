/* AUTORIGHTS
Copyright (C) 2007 Princeton University
      
This file is part of Ferret Toolkit.

Ferret Toolkit is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <stdio.h>
#include <stdlib.h>
#include <cass_timer.h>

void stimer_tick(stimer_t *timer)
{ 
	gettimeofday(&(timer->start), 0); 
} 

float stimer_tuck(stimer_t *timer, const char *msg)
{ 
	gettimeofday(&(timer->end), 0); 

	timer->diff = (timer->end.tv_sec - timer->start.tv_sec) 
	   			+ (timer->end.tv_usec - timer->start.tv_usec) * 0.000001; 

	if (msg) 
		printf("%s: %.3f seconds\n", msg, timer->diff); 

	return timer->diff; 
} 

