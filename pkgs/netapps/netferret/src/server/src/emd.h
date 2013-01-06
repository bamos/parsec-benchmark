#ifndef _EMD_H
#define _EMD_H

#include <cass.h>

/* DEFINITIONS */
#define MAX_SIG_SIZE   256
#define MAX_SIG_SIZE1  (MAX_SIG_SIZE+1)  /* FOR THE POSIBLE DUMMY FEATURE */

#define MAX_ITERATIONS 500
#define EMD_INFINITY   1e20
#define EPSILON        1e-6

/* feature_t SHOULD BE MODIFIED BY THE USER TO REFLECT THE FEATURE TYPE      */
typedef void *feature_t;
typedef struct flow_t flow_t;
typedef struct signature_t signature_t;
typedef struct emd_node1_t emd_node1_t;
typedef struct emd_node2_t emd_node2_t;
typedef struct emd_state_t emd_state_t;

struct signature_t
{
	int n;                /* Number of features in the signature */
	feature_t *Features;  /* Pointer to the feature vector */
	float *Weights;       /* Pointer to the weights of the features */
};

struct flow_t
{
	int from;             /* Feature number in signature 1 */
	int to;               /* Feature number in signature 2 */
	float amount;         /* Amount of flow from "from" to "to" */
	float cost;           /* cost of the flow associated with the move. */
};

struct emd_node1_t {
	int i;
	double val;
	struct emd_node1_t *Next;
};

struct emd_node2_t {
	int i, j;
	double val;
	struct emd_node2_t *NextC;               /* NEXT COLUMN */
	struct emd_node2_t *NextR;               /* NEXT ROW */
};

struct emd_state_t {
	int		n1;			/* SIGNATURES SIZES */
	int		n2;
	float		**C;//[MAX_SIG_SIZE1][MAX_SIG_SIZE1]; /* THE COST MATRIX */
	emd_node2_t	X[MAX_SIG_SIZE1*2];	/* THE BASIC VARIABLES VECTOR */

	/* VARIABLES TO HANDLE _X EFFICIENTLY */
	emd_node2_t	*EndX;
	emd_node2_t	*EnterX;
	char		IsX[MAX_SIG_SIZE1][MAX_SIG_SIZE1];
	emd_node2_t	*RowsX[MAX_SIG_SIZE1];
	emd_node2_t	*ColsX[MAX_SIG_SIZE1];
	double		maxW;
	float		maxC;
	double		tot_flow_costs;
	int		tot_flow;
};

/*
 * This implementation is taken from Rubner's with modifications
 * also taken from the original Ferret Toolkit.  Those latter
 * modifications force a distance matrix to be allocated twice
 * and copied inefficiently into the EMD module, but that's the
 * interface the rest of the toolkit expects at this time.
 */

emd_state_t	*mkemdstate(void);
void		freeemdstate(emd_state_t*);
float		emd(signature_t*, signature_t*, float (*)(cass_size_t, feature_t, feature_t, void *), cass_size_t dim, void *param, flow_t*, int*);
#endif
