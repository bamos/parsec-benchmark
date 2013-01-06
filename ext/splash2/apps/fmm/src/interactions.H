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

#ifndef _Expansions_H
#define _Expansions_H 1

#include "box.h"

extern void InitExpTables();
extern void PrintExpTables();
extern void UpwardPass(int my_id, box *b);
extern void ComputeInteractions(int my_id, box *b);
extern void DownwardPass(int my_id, box *b);
extern void ComputeParticlePositions(int my_id, box *b);

#endif /* _Interactions_H */

