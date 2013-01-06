#ifndef _LSH_PICKPERT_H_
#define _LSH_PICKPERT_H_

#include "heap.h"

#define MAX_PERT_LOC 5 // perterb up to 5 locations.
#define PERT_POINTS_FACTOR 20  // Increase this value if you can not get enough perterbation points.

typedef struct _pert_points {
    int table_id;
    int func_id;
    int left_right;  // 0 is left, 1 is right.
    float value; // value of x_i
    float prob_val; // value of (x_i)^2 for now, but could be modified in the future.
} pert_points_t;

typedef struct _pickpert_t {
    int n_tables;
    int n_entries;  // n_entries = n_funcs * 2
    pert_points_t **pnts; // reference to the pnts passed within _init()
    Heap *heap;
} pickpert_t;

extern pickpert_t *pickpert_init (int n_tables, int n_funcs, int n_max_perts, pert_points_t **pnts);
extern int pickpert_get (pickpert_t *pert, int *pert_point_index);
extern void pickpert_free (pickpert_t *pert);

extern pert_points_t * pert_points_alloc ();
extern void pert_points_free (pert_points_t *p);
extern void pert_points_sort (pert_points_t **list, int start, int end);

struct saved_index {
    int cnt;
    int pert_points_index[MAX_PERT_LOC];
};

typedef struct _pickpert2_t {
    int n_max_perts;
    struct saved_index *index_table;
} pickpert2_t;

extern pickpert2_t * pickpert2_init (int n_funcs, int n_max_perts);
extern struct saved_index *pickpert2_get (pickpert2_t *pert2, int index);
extern void pickpert2_free (pickpert2_t *pert2);

#endif

