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

#ifndef _Construct_Grid_H
#define _Construct_Grid_H 1

extern void ConstructGrid(int my_id, time_info *local_time, int time_all);
extern void ConstructLists(int my_id, time_info *local_time, int time_all);
extern void DestroyGrid(int my_id, time_info *local_time, int time_all);
extern void PrintGrid(int my_id);

#endif /* _Construct_Grid_H */
