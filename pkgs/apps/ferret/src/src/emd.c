/* AUTORIGHTS
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/*
 *    emd.c
 *
 *    Last update: 3/14/98
 *
 *    An implementation of the Earth Movers Distance.
 *    Based of the solution for the Transportation problem as described in
 *    "Introduction to Mathematical Programming" by F. S. Hillier and 
 *    G. J. Lieberman, McGraw-Hill, 1990.
 *
 *    Copyright (C) 1998 Yossi Rubner
 *    Computer Science Department, Stanford University
 *    E-Mail: rubner@cs.stanford.edu   URL: http://vision.stanford.edu/~rubner
 *
 *    Bogus static data structures removed November 2005.
 */

/*    This file is modified by William K. Josephson and Wei Dong to by re-entrant
 *    and to work with the Ferret toolkit.  Under the permit of the original author,
 *    this file is redistributed under the GPL license */

#include <math.h>
#include "emd.h"

#define DEBUG_LEVEL 0
/*
 DEBUG_LEVEL:
   0 = NO MESSAGES
   1 = PRINT THE NUMBER OF ITERATIONS AND THE FINAL RESULT
   2 = PRINT THE RESULT AFTER EVERY ITERATION
   3 = PRINT ALSO THE FLOW AFTER EVERY ITERATION
   4 = PRINT A LOT OF INFORMATION (PROBABLY USEFUL ONLY FOR THE AUTHOR)
*/

/* NEW TYPES DEFINITION */
/* GLOBAL VARIABLE DECLARATION */

/* DECLARATION OF FUNCTIONS */
static float emdinit(emd_state_t*, signature_t *Signature1,
	signature_t *Signature2,
	float (*Dist)(cass_size_t, feature_t, feature_t, void *), cass_size_t, void *);
static void findBasicVariables(emd_state_t*, emd_node1_t *U, emd_node1_t *V);
static int isOptimal(emd_state_t*, emd_node1_t *U, emd_node1_t *V);
static int findLoop(emd_state_t*, emd_node2_t **Loop);
static void newSol(emd_state_t*);
static void russel(emd_state_t*, double *S, double *D);
static void addBasicVariable(emd_state_t*, int minI, int minJ, double *S,
				double *D, emd_node1_t *PrevUMinI,
				emd_node1_t *PrevVMinJ, emd_node1_t *UHead);
#if DEBUG_LEVEL > 0
static void printSolution(emd_state_t*);
#endif

emd_state_t*
mkemdstate(void)
{
	emd_state_t *state;

	state = malloc(sizeof *state);
	bzero(state, sizeof *state);
	return state;
}

void
freeemdstate(emd_state_t *state)
{
	free(state);
	return;
}

/******************************************************************************
float emd(signature_t *Signature1, signature_t *Signature2,
	  float (*Dist)(feature_t *, feature_t *), flow_t *Flow, int *FlowSize)
  
where

   Signature1, Signature2  Pointers to signatures that their distance we want
              to compute.
   Dist       Pointer to the ground distance. i.e. the function that computes
              the distance between two features.
   Flow       (Optional) Pointer to a vector of flow_t (defined in emd.h) 
              where the resulting flow will be stored. Flow must have n1+n2-1
              elements, where n1 and n2 are the sizes of the two signatures
              respectively.
              If NULL, the flow is not returned.
   FlowSize   (Optional) Pointer to an integer where the number of elements in
              Flow will be stored
              
******************************************************************************/
static __thread double **C = NULL;

float
emd(signature_t *Signature1, signature_t *Signature2, float (*Dist)(cass_size_t, feature_t, feature_t, void *), cass_size_t dim, void *param, flow_t *Flow, int*FlowSize)
{
  struct emd_state_t emd_state, *state = &emd_state;
  int itr;
  double totalCost;
  float w;
  emd_node2_t *XP;
  flow_t *FlowP;
  emd_node1_t U[MAX_SIG_SIZE1], V[MAX_SIG_SIZE1];

  if (C == NULL) C = type_matrix_alloc(double, MAX_SIG_SIZE1, MAX_SIG_SIZE1);

  bzero(state, sizeof *state);

  state->C = C;

  w = emdinit(state, Signature1, Signature2, Dist, dim, param);

  if (w == EMD_INFINITY) {
      // init failed, due to nr_reg too high, ignore this seg.
      return EMD_INFINITY;
  }

#if DEBUG_LEVEL > 1
  print("\nINITIAL SOLUTION:\n");
  printSolution(state);
#endif
 
  if (state->n1 > 1 && state->n2 > 1)  /* IF _n1 = 1 OR _n2 = 1 THEN WE ARE DONE */
    {
      for (itr = 1; itr < MAX_ITERATIONS; itr++)
	{
	  /* FIND BASIC VARIABLES */
	  findBasicVariables(state, U, V);
	  
	  /* CHECK FOR OPTIMALITY */
	  if (isOptimal(state, U, V))
	    break;
	  
	  /* IMPROVE SOLUTION */
	  newSol(state);
	  
#if DEBUG_LEVEL > 1
	  print("\nITERATION # %d \n", itr);
	  printSolution(state);
#endif
	}

      if (itr == MAX_ITERATIONS)
	warn("emd: Maximum number of iterations has been reached (%d)\n",
		MAX_ITERATIONS);
    }

  /* COMPUTE THE TOTAL FLOW */
  totalCost = 0;
  FlowP = Flow;
  for(XP=state->X; XP < state->EndX; XP++)
    {
      if (XP == state->EnterX)  /* _EnterX IS THE EMPTY SLOT */
	continue;
      if (XP->i == Signature1->n || XP->j == Signature2->n)  /* DUMMY FEATURE */
	continue;
      
      if (XP->val == 0)  /* ZERO FLOW */
	continue;

      totalCost += (double)XP->val * state->C[XP->i][XP->j];
      if (Flow != NULL)
	{
	  FlowP->from = XP->i;
	  FlowP->to = XP->j;
	  FlowP->amount = XP->val;
	  FlowP->cost = state->C[XP->i][XP->j];
	  state->tot_flow_costs += FlowP->cost;
	  state->tot_flow ++;
	  FlowP++;
	}
    }
  if (Flow != NULL)
    *FlowSize = FlowP-Flow;

#if DEBUG_LEVEL > 0
  print("\n*** OPTIMAL SOLUTION (%d ITERATIONS): %f ***\n", itr, totalCost);
#endif

  /* RETURN THE NORMALIZED COST == EMD */
//  print("avg_flow_costs: %g/%d = %g\n", state->tot_flow_costs,
//	state->tot_flow, state->tot_flow_costs / state->tot_flow);
  return (float)(totalCost / w);
}

static float
emdinit(emd_state_t *state, signature_t *Signature1, signature_t *Signature2, float (*Dist)(cass_size_t, feature_t, feature_t, void *), cass_size_t dim, void *param)
{
  int i, j;
  double sSum, dSum, diff;
  double S[MAX_SIG_SIZE1], D[MAX_SIG_SIZE1];
 
  state->n1 = Signature1->n;
  state->n2 = Signature2->n;

  if (state->n1 > MAX_SIG_SIZE || state->n2 > MAX_SIG_SIZE)
    {
      warn("emd: Signature size is limited to %d, n1: %d, n2: %d\n", MAX_SIG_SIZE, state->n1, state->n2);
      return EMD_INFINITY;
    }
  
  /* COMPUTE THE DISTANCE MATRIX */
  state->maxC = 0;
  for(i=0; i < state->n1; i++)
    for(j=0; j < state->n2; j++) 
      {
	state->C[i][j] = Dist(dim, Signature1->Features[i], Signature2->Features[j], param);
	if (state->C[i][j] > state->maxC)
	  state->maxC = state->C[i][j];
      }
	
  /* SUM UP THE SUPPLY AND DEMAND */
  sSum = 0.0;
  for(i=0; i < state->n1; i++)
    {
      S[i] = Signature1->Weights[i];
      sSum += Signature1->Weights[i];
      state->RowsX[i] = NULL;
    }
  dSum = 0.0;
  for(j=0; j < state->n2; j++)
    {
      D[j] = Signature2->Weights[j];
      dSum += Signature2->Weights[j];
      state->ColsX[j] = NULL;
    }

  /* IF SUPPLY DIFFERENT THAN THE DEMAND, ADD A ZERO-COST DUMMY CLUSTER */
  diff = sSum - dSum;
  if (fabs(diff) >= EPSILON * sSum)
    {
      if (diff < 0.0)
	{
	  for (j=0; j < state->n2; j++)
	    state->C[state->n1][j] = 0;
	  S[state->n1] = -diff;
	  state->RowsX[state->n1] = NULL;
	  state->n1++;
	}
      else
	{
	  for (i=0; i < state->n1; i++)
	    state->C[i][state->n2] = 0;
	  D[state->n2] = diff;
	  state->ColsX[state->n2] = NULL;
	  state->n2++;
	}
    }

  /* INITIALIZE THE BASIC VARIABLE STRUCTURES */
  for (i=0; i < state->n1; i++)
    for (j=0; j < state->n2; j++)
	state->IsX[i][j] = 0;
  state->EndX = state->X;
   
  state->maxW = sSum > dSum ? sSum : dSum;

  /* FIND INITIAL SOLUTION */
  russel(state, S, D);

  state->EnterX = state->EndX++;  /* AN EMPTY SLOT (ONLY _n1+_n2-1 BASIC VARIABLES) */

  return sSum > dSum ? dSum : sSum;
}

static void
findBasicVariables(emd_state_t *state, emd_node1_t *U, emd_node1_t *V)
{
  int i, j, found;
  int UfoundNum, VfoundNum;
  emd_node1_t u0Head, u1Head, *CurU, *PrevU;
  emd_node1_t v0Head, v1Head, *CurV, *PrevV;

  /* INITIALIZE THE ROWS LIST (U) AND THE COLUMNS LIST (V) */
  u0Head.Next = CurU = U;
  for (i=0; i < state->n1; i++)
    {
      CurU->i = i;
      CurU->Next = CurU+1;
      CurU++;
    }
  (--CurU)->Next = NULL;
  u1Head.Next = NULL;

  CurV = V+1;
  v0Head.Next = state->n2 > 1 ? V+1 : NULL;
  for (j=1; j < state->n2; j++)
    {
      CurV->i = j;
      CurV->Next = CurV+1;
      CurV++;
    }
  (--CurV)->Next = NULL;
  v1Head.Next = NULL;

  /* THERE ARE _n1+_n2 VARIABLES BUT ONLY _n1+_n2-1 INDEPENDENT EQUATIONS,
     SO SET V[0]=0 */
  V[0].i = 0;
  V[0].val = 0;
  v1Head.Next = V;
  v1Head.Next->Next = NULL;

  /* LOOP UNTIL ALL VARIABLES ARE FOUND */
  UfoundNum=VfoundNum=0;
  while (UfoundNum < state->n1 || VfoundNum < state->n2)
    {

#if DEBUG_LEVEL > 3
      print("UfoundNum=%d/%d,VfoundNum=%d/%d\n",UfoundNum,state->n1,VfoundNum,state->n2);
      print("U0=");
      for(CurU = u0Head.Next; CurU != NULL; CurU = CurU->Next)
	print("[%d]",CurU-U);
      print("\n");
      print("U1=");
      for(CurU = u1Head.Next; CurU != NULL; CurU = CurU->Next)
	print("[%d]",CurU-U);
      print("\n");
      print("V0=");
      for(CurV = v0Head.Next; CurV != NULL; CurV = CurV->Next)
	print("[%d]",CurV-V);
      print("\n");
      print("V1=");
      for(CurV = v1Head.Next; CurV != NULL; CurV = CurV->Next)
	print("[%d]",CurV-V);
      print("\n\n");
#endif
      
      found = 0;
      if (VfoundNum < state->n2)
	{
	  /* LOOP OVER ALL MARKED COLUMNS */
	  PrevV = &v1Head;
	  for (CurV=v1Head.Next; CurV != NULL; CurV=CurV->Next)
	    {
	      j = CurV->i;
	      /* FIND THE VARIABLES IN COLUMN j */
	      PrevU = &u0Head;
	      for (CurU=u0Head.Next; CurU != NULL; CurU=CurU->Next)
		{
		  i = CurU->i;
		  if (state->IsX[i][j])
		    {
		      /* COMPUTE U[i] */
		      CurU->val = state->C[i][j] - CurV->val;
		      /* ...AND ADD IT TO THE MARKED LIST */
		      PrevU->Next = CurU->Next;
		      CurU->Next = u1Head.Next != NULL ? u1Head.Next : NULL;
		      u1Head.Next = CurU;
		      CurU = PrevU;
		    }
		  else
		    PrevU = CurU;
		}
	      PrevV->Next = CurV->Next;
	      VfoundNum++;
	      found = 1;
	    }
	}
     if (UfoundNum < state->n1)
	{
	  /* LOOP OVER ALL MARKED ROWS */
	  PrevU = &u1Head;
	  for (CurU=u1Head.Next; CurU != NULL; CurU=CurU->Next)
	    {
	      i = CurU->i;
	      /* FIND THE VARIABLES IN ROWS i */
	      PrevV = &v0Head;
	      for (CurV=v0Head.Next; CurV != NULL; CurV=CurV->Next)
		{
		  j = CurV->i;
		  if (state->IsX[i][j])
		    {
		      /* COMPUTE V[j] */
		      CurV->val = state->C[i][j] - CurU->val;
		      /* ...AND ADD IT TO THE MARKED LIST */
		      PrevV->Next = CurV->Next;
		      CurV->Next = v1Head.Next != NULL ? v1Head.Next: NULL;
		      v1Head.Next = CurV;
		      CurV = PrevV;
		    }
		  else
		    PrevV = CurV;
		}
	      PrevU->Next = CurU->Next;
	      UfoundNum++;
	      found = 1;
	    }
	}
     if (! found)
       {
	 warn("emd: Unexpected error in findBasicVariables!\n");
	 warn("This typically happens when the EPSILON defined in\n");
	 warn("emd.h is not right for the scale of the problem.\n");
	 fatal("emd: errr in findBasicVariables");
       }
    }
}

static int
isOptimal(emd_state_t *state, emd_node1_t *U, emd_node1_t *V)
{    
  double delta, deltaMin;
  int i, j, minI, minJ;

  minI = minJ = 0;	/* XXX: used but not set */ 

  /* FIND THE MINIMAL Cij-Ui-Vj OVER ALL i,j */
  deltaMin = EMD_INFINITY;
  for(i=0; i < state->n1; i++)
    for(j=0; j < state->n2; j++)
      if (! state->IsX[i][j])
	{
	  delta = state->C[i][j] - U[i].val - V[j].val;
	  if (deltaMin > delta)
	    {
              deltaMin = delta;
	      minI = i;
	      minJ = j;
	    }
	}

#if DEBUG_LEVEL > 3
  print("deltaMin=%f\n", deltaMin);
#endif

   if (deltaMin == EMD_INFINITY)
     {
       warn("emd: Unexpected error in isOptimal.\n");
       fatal("emd: error in isOptimal");
     }
   
   state->EnterX->i = minI;
   state->EnterX->j = minJ;
   
   /* IF NO NEGATIVE deltaMin, WE FOUND THE OPTIMAL SOLUTION */
   return deltaMin >= -EPSILON * state->maxC;

/*
   return deltaMin >= -EPSILON;
 */
}

static void
newSol(emd_state_t *state)
{
    int i, j, k;
    double xMin;
    int steps;
    emd_node2_t *Loop[2*MAX_SIG_SIZE1], *CurX, *LeaveX;

    LeaveX = NULL;	/* XXX: used but not set */
 
#if DEBUG_LEVEL > 3
    print("EnterX = (%d,%d)\n", state->EnterX->i, state->EnterX->j);
#endif

    /* ENTER THE NEW BASIC VARIABLE */
    i = state->EnterX->i;
    j = state->EnterX->j;
    state->IsX[i][j] = 1;
    state->EnterX->NextC = state->RowsX[i];
    state->EnterX->NextR = state->ColsX[j];
    state->EnterX->val = 0;
    state->RowsX[i] = state->EnterX;
    state->ColsX[j] = state->EnterX;

    /* FIND A CHAIN REACTION */
    steps = findLoop(state, Loop);

    /* FIND THE LARGEST VALUE IN THE LOOP */
    xMin = EMD_INFINITY;
    for (k=1; k < steps; k+=2)
      {
	if (Loop[k]->val < xMin)
	  {
	    LeaveX = Loop[k];
	    xMin = Loop[k]->val;
	  }
      }

    /* UPDATE THE LOOP */
    for (k=0; k < steps; k+=2)
      {
	Loop[k]->val += xMin;
	Loop[k+1]->val -= xMin;
      }

#if DEBUG_LEVEL > 3
    print("LeaveX = (%d,%d)\n", LeaveX->i, LeaveX->j);
#endif

    /* REMOVE THE LEAVING BASIC VARIABLE */
    i = LeaveX->i;
    j = LeaveX->j;
    state->IsX[i][j] = 0;
    if (state->RowsX[i] == LeaveX)
      state->RowsX[i] = LeaveX->NextC;
    else
      for (CurX=state->RowsX[i]; CurX != NULL; CurX = CurX->NextC)
	if (CurX->NextC == LeaveX)
	  {
	    CurX->NextC = CurX->NextC->NextC;
	    break;
	  }
    if (state->ColsX[j] == LeaveX)
      state->ColsX[j] = LeaveX->NextR;
    else
      for (CurX=state->ColsX[j]; CurX != NULL; CurX = CurX->NextR)
	if (CurX->NextR == LeaveX)
	  {
	    CurX->NextR = CurX->NextR->NextR;
	    break;
	  }

    /* SET _EnterX TO BE THE NEW EMPTY SLOT */
    state->EnterX = LeaveX;
}

static int
findLoop(emd_state_t *state, emd_node2_t **Loop)
{
  int i, steps;
  emd_node2_t **CurX, *NewX;
  char IsUsed[2*MAX_SIG_SIZE1]; 
 
  for (i=0; i < state->n1+state->n2; i++)
    IsUsed[i] = 0;

  CurX = Loop;
  NewX = *CurX = state->EnterX;
  IsUsed[state->EnterX-state->X] = 1;
  steps = 1;

  do
    {
      if (steps%2 == 1)
	{
	  /* FIND AN UNUSED X IN THE ROW */
	  NewX = state->RowsX[NewX->i];
	  while (NewX != NULL && IsUsed[NewX-state->X])
	    NewX = NewX->NextC;
	}
      else
	{
	  /* FIND AN UNUSED X IN THE COLUMN, OR THE ENTERING X */
	  NewX = state->ColsX[NewX->j];
	  while (NewX != NULL && IsUsed[NewX-state->X] && NewX != state->EnterX)
	    NewX = NewX->NextR;
	  if (NewX == state->EnterX)
	    break;
 	}

     if (NewX != NULL)  /* FOUND THE NEXT X */
       {
	 /* ADD X TO THE LOOP */
	 *++CurX = NewX;
	 IsUsed[NewX-state->X] = 1;
	 steps++;
#if DEBUG_LEVEL > 3
	 print("steps=%d, NewX=(%d,%d)\n", steps, NewX->i, NewX->j);    
#endif
       }
     else  /* DIDN'T FIND THE NEXT X */
       {
	 /* BACKTRACK */
	 do
	   {
	     NewX = *CurX;
	     do 
	       {
		 if (steps%2 == 1)
		   NewX = NewX->NextR;
		 else
		   NewX = NewX->NextC;
	       } while (NewX != NULL && IsUsed[NewX-state->X]);
	     
	     if (NewX == NULL)
	       {
		 IsUsed[*CurX-state->X] = 0;
		 CurX--;
		 steps--;
	       }
	 } while (NewX == NULL && CurX >= Loop);
	 
#if DEBUG_LEVEL > 3
	 print("BACKTRACKING TO: steps=%d, NewX=(%d,%d)\n",
		steps, NewX->i, NewX->j);    
#endif
           IsUsed[*CurX-state->X] = 0;
	   *CurX = NewX;
	   IsUsed[NewX-state->X] = 1;
       }     
    } while(CurX >= Loop);
  
  if (CurX == Loop)
    {
      warn("emd: Unexpected error in findLoop!\n");
      fatal("emd: findLoop");
    }
#if DEBUG_LEVEL > 3
  print("FOUND LOOP:\n");
  for (i=0; i < steps; i++)
    print("%d: (%d,%d)\n", i, Loop[i]->i, Loop[i]->j);
#endif

  return steps;
}

static void
russel(emd_state_t *state, double *S, double *D)
{
  int i, j, found, minI, minJ;
  double deltaMin, oldVal, diff;
  double Delta[MAX_SIG_SIZE1][MAX_SIG_SIZE1];
  emd_node1_t Ur[MAX_SIG_SIZE1], Vr[MAX_SIG_SIZE1];
  emd_node1_t uHead, *CurU, *PrevU;
  emd_node1_t vHead, *CurV, *PrevV;
  emd_node1_t *PrevUMinI, *PrevVMinJ, *Remember;

  PrevUMinI = PrevVMinJ = NULL;

  /* INITIALIZE THE ROWS LIST (Ur), AND THE COLUMNS LIST (Vr) */
  uHead.Next = CurU = Ur;
  for (i=0; i < state->n1; i++)
    {
      CurU->i = i;
      CurU->val = -EMD_INFINITY;
      CurU->Next = CurU+1;
      CurU++;
    }
  (--CurU)->Next = NULL;
  
  vHead.Next = CurV = Vr;
  for (j=0; j < state->n2; j++)
    {
      CurV->i = j;
      CurV->val = -EMD_INFINITY;
      CurV->Next = CurV+1;
      CurV++;
    }
  (--CurV)->Next = NULL;
  
  /* FIND THE MAXIMUM ROW AND COLUMN VALUES (Ur[i] AND Vr[j]) */
  for(i=0; i < state->n1 ; i++)
    for(j=0; j < state->n2 ; j++)
      {
	float v;
	v = state->C[i][j];
	if (Ur[i].val <= v)
	  Ur[i].val = v;
	if (Vr[j].val <= v)
	  Vr[j].val = v;
      }
  
  /* COMPUTE THE Delta MATRIX */
  for(i=0; i < state->n1 ; i++)
    for(j=0; j < state->n2 ; j++)
      Delta[i][j] = state->C[i][j] - Ur[i].val - Vr[j].val;

  /* FIND THE BASIC VARIABLES */
  do
    {
#if DEBUG_LEVEL > 3
      print("Ur=");
      for(CurU = uHead.Next; CurU != NULL; CurU = CurU->Next)
	print("[%d]",CurU-Ur);
      print("\n");
      print("Vr=");
      for(CurV = vHead.Next; CurV != NULL; CurV = CurV->Next)
	print("[%d]",CurV-Vr);
      print("\n");
      print("\n\n");
#endif
 
      /* FIND THE SMALLEST Delta[i][j] */
      found = 0; 
      minI = uHead.Next ? uHead.Next->i : 0;	/* XXX: used but not set */
      minJ = vHead.Next ? vHead.Next->i : 0;	/* XXX: used but not set */
      deltaMin = EMD_INFINITY;      
      PrevU = &uHead;
      for (CurU=uHead.Next; CurU != NULL; CurU=CurU->Next)
	{
	  int i;
	  i = CurU->i;
	  PrevV = &vHead;
	  for (CurV=vHead.Next; CurV != NULL; CurV=CurV->Next)
	    {
	      int j;
	      j = CurV->i;
	      if (deltaMin > Delta[i][j])
		{
		  deltaMin = Delta[i][j];
		  minI = i;
		  minJ = j;
		  PrevUMinI = PrevU;
		  PrevVMinJ = PrevV;
		  found = 1;
		}
	      PrevV = CurV;
	    }
	  PrevU = CurU;
	}
      
      if (! found)
	break;

      /* ADD X[minI][minJ] TO THE BASIS, AND ADJUST SUPPLIES AND COST */
      Remember = PrevUMinI->Next;
      addBasicVariable(state, minI, minJ, S, D, PrevUMinI, PrevVMinJ, &uHead);

      /* UPDATE THE NECESSARY Delta[][] */
      if (Remember == PrevUMinI->Next)  /* LINE minI WAS DELETED */
	{
	  for (CurV=vHead.Next; CurV != NULL; CurV=CurV->Next)
	    {
	      int j;
	      j = CurV->i;
	      if (CurV->val == state->C[minI][j])  /* COLUMN j NEEDS UPDATING */
		{
		  /* FIND THE NEW MAXIMUM VALUE IN THE COLUMN */
		  oldVal = CurV->val;
		  CurV->val = -EMD_INFINITY;
		  for (CurU=uHead.Next; CurU != NULL; CurU=CurU->Next)
		    {
		      int i;
		      i = CurU->i;
		      if (CurV->val <= state->C[i][j])
			CurV->val = state->C[i][j];
		    }
		  
		  /* IF NEEDED, ADJUST THE RELEVANT Delta[*][j] */
		  diff = oldVal - CurV->val;
		  if (fabs(diff) < EPSILON * state->maxC)
		    for (CurU=uHead.Next; CurU != NULL; CurU=CurU->Next)
		      Delta[CurU->i][j] += diff;
		}
	    }
	}
      else  /* COLUMN minJ WAS DELETED */
	{
	  for (CurU=uHead.Next; CurU != NULL; CurU=CurU->Next)
	    {
	      int i;
	      i = CurU->i;
	      if (CurU->val == state->C[i][minJ])  /* ROW i NEEDS UPDATING */
		{
		  /* FIND THE NEW MAXIMUM VALUE IN THE ROW */
		  oldVal = CurU->val;
		  CurU->val = -EMD_INFINITY;
		  for (CurV=vHead.Next; CurV != NULL; CurV=CurV->Next)
		    {
		      int j;
		      j = CurV->i;
		      if(CurU->val <= state->C[i][j])
			CurU->val = state->C[i][j];
		    }
		  
		  /* If NEEDED, ADJUST THE RELEVANT Delta[i][*] */
		  diff = oldVal - CurU->val;
		  if (fabs(diff) < EPSILON * state->maxC)
		    for (CurV=vHead.Next; CurV != NULL; CurV=CurV->Next)
		      Delta[i][CurV->i] += diff;
		}
	    }
	}
    } while (uHead.Next != NULL || vHead.Next != NULL);
}

static void
addBasicVariable(emd_state_t *state, int minI, int minJ, double *S, double *D, emd_node1_t *PrevUMinI, emd_node1_t *PrevVMinJ, emd_node1_t *UHead)
{
  double T;
  
  if (fabs(S[minI]-D[minJ]) <= EPSILON * state->maxW)  /* DEGENERATE CASE */
    {
      T = S[minI];
      S[minI] = 0;
      D[minJ] -= T; 
    }
  else if (S[minI] < D[minJ])  /* SUPPLY EXHAUSTED */
    {
      T = S[minI];
      S[minI] = 0;
      D[minJ] -= T; 
    }
  else  /* DEMAND EXHAUSTED */
    {
      T = D[minJ];
      D[minJ] = 0; 
      S[minI] -= T; 
    }

  /* X(minI,minJ) IS A BASIC VARIABLE */
  state->IsX[minI][minJ] = 1; 

  state->EndX->val = T;
  state->EndX->i = minI;
  state->EndX->j = minJ;
  state->EndX->NextC = state->RowsX[minI];
  state->EndX->NextR = state->ColsX[minJ];
  state->RowsX[minI] = state->EndX;
  state->ColsX[minJ] = state->EndX;
  state->EndX++;

  /* DELETE SUPPLY ROW ONLY IF THE EMPTY, AND IF NOT LAST ROW */
  if (S[minI] == 0 && UHead->Next->Next != NULL)
    PrevUMinI->Next = PrevUMinI->Next->Next;  /* REMOVE ROW FROM LIST */
  else
    PrevVMinJ->Next = PrevVMinJ->Next->Next;  /* REMOVE COLUMN FROM LIST */
}

#if DEBUG_LEVEL > 0
static void
printSolution(emd_state_t *state)
{
  emd_node2_t *P;
  double totalCost;

  totalCost = 0;

#if DEBUG_LEVEL > 2
  print("SIG1\tSIG2\tFLOW\tCOST\n");
#endif
  for(P=state->X; P < state->EndX; P++)
    if (P != state->EnterX && state->IsX[P->i][P->j])
      {
#if DEBUG_LEVEL > 2
	print("%d\t%d\t%f\t%f\n", P->i, P->j, P->val, state->C[P->i][P->j]);
#endif
	totalCost += (double)P->val * state->C[P->i][P->j];
      }

  print("COST = %f\n", totalCost);
}
#endif
