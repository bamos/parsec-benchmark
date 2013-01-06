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

#ifndef _Box_H
#define _Box_H 1

#include "defs.h"
#include "particle.h"

/* This definition sets the maximum number of particles allowed per box. */
#define MAX_PARTICLES_PER_BOX 40

/* This definition sets the number of subdivisions (offspring) of a box. */
#define NUM_OFFSPRING 4
#define MAX_SIBLINGS (NUM_OFFSPRING - 1)
#define MAX_COLLEAGUES 8
#define MAX_U_LIST 20
#define MAX_V_LIST 27
#define MAX_W_LIST 30
#define MAX_EXPANSION_TERMS 40

typedef struct _Box box;
typedef struct _Box_Node box_node;

typedef void (*list_function)(int my_id, box *list_box, box *b);

typedef enum { CHILDLESS, PARENT } box_type;

#define ID_LIMIT 1000000

/* Every box has :
 *    1. A unique ID number (made up of a unique ID number per processor plus
 *       the ID of the processor that created the box)
 *    2.- 3. An x and y position for its center
 *    4. The length of the box (measured as the length of one of its sides)
 *    5. The level of ancestry of the box (how many parents do you have to 
 *       visit before the first box is found?)
 *    6. The number of particles in the box
 *    7. A list of those particles
 *    8. A pointer to its parent
 *    9. The number of children
 *   10. A list of its children
 *   11. The number of siblings
 *   12. A list of its siblings
 *   13. A linked list of its colleagues
 *   14. A linked list representing list 1 in RR #496
 *   15. A linked list representing list 2 in RR #496
 *   16. A linked list representing list 3 in RR #496
 *   17. An array of its multipole expansion terms.
 *   18. An array of its local expansion terms.
 *   19. The id of the processor that is working on the box.
 *   20. The amount of computational work associated with the box.
 */

struct _Box
{
  double id;
  real x_center;
  real y_center;
  real length;
  int level;
  box_type type;
  particle *particles[MAX_PARTICLES_PER_BOX + 1];
  int num_particles;
  box *parent;
  int child_num;
  box *shadow[NUM_OFFSPRING];
  box *children[NUM_OFFSPRING];
  int num_children;
  box *siblings[MAX_SIBLINGS];
  int num_siblings;
  box *colleagues[MAX_COLLEAGUES];
  int num_colleagues;
  box *u_list[MAX_U_LIST];
  int num_u_list;
  box *v_list[MAX_V_LIST];
  int num_v_list;
  box *w_list[MAX_W_LIST];
  int num_w_list;
  complex mp_expansion[MAX_EXPANSION_TERMS];
  complex local_expansion[MAX_EXPANSION_TERMS];
  complex x_expansion[MAX_EXPANSION_TERMS];
  int exp_lock_index;
  int particle_lock_index;
  volatile int construct_synch;
  volatile int interaction_synch;
  int proc;
  int cost;
  int u_cost;
  int v_cost;
  int w_cost;
  int p_cost;
  int subtree_cost;
  box *next;
  box *prev;
  box *link1;
  box *link2;
};


/* This structure is used for a linked list of boxes */
struct _Box_Node
{
  box *data;
  struct _Box_Node *next;
};

extern box *Grid;

extern void CreateBoxes(int my_id, int num_boxes);
extern void FreeBoxes(int my_id);
extern box *InitBox(int my_id, real x_center, real y_center, real length,
		    box *parent);
extern void PrintBox(box *b);
extern void PrintBoxArrayIds(box *b_array[], int array_length);
extern void PrintExpansionTerms(complex expansion[]);

extern void ListIterate(int my_id, box *b, box **list, int length,
			list_function function);
extern int AdjacentBoxes(int my_id, box *b1, box *b2);
extern int WellSeparatedBoxes(int my_id, box *b1, box *b2);

#endif /* _Box_H */
