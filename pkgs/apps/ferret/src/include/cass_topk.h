/* HEAP BASED TOP-K */
#ifndef __WDONG_TOPK__
#define __WDONG_TOPK__

#include <stdlib.h>

typedef struct {
	uint32_t key;
	uint32_t index;
} itopk_t;

typedef struct {
	float key;
	uint32_t index;
} ftopk_t;

/* prefix_ge (type a, type b) is needed */

#define QUICKSORT_PROTOTYPE(prefix, type)					\
void prefix##_qsort(type *, cass_size_t)

#define QUICKSORT_GENERATE(prefix, type)					\
static void prefix##_qsort_help(type *numbers, cass_size_t left, cass_size_t right)	\
{										\
  cass_size_t pivot, l_hold, r_hold;							\
  type pivot_v;									\
  l_hold = left;								\
  r_hold = right;								\
  pivot_v = numbers[left];							\
  while (left < right) {							\
    while (prefix##_ge(numbers[right], pivot_v) && (left < right)) right--;	\
    if (left != right) {							\
      numbers[left] = numbers[right];						\
      left++;									\
    }										\
    while (prefix##_ge(pivot_v, numbers[left]) && (left < right)) left++;	\
    if (left != right) {							\
      numbers[right] = numbers[left];						\
      right--;									\
    }										\
  }										\
  numbers[left] = pivot_v;							\
  pivot = left;									\
  if (l_hold < pivot) prefix##_qsort_help(numbers, l_hold, pivot-1);		\
  if (r_hold > pivot) prefix##_qsort_help(numbers, pivot+1, r_hold);		\
}										\
										\
void prefix##_qsort(type* numbers, cass_size_t array_size) {				\
  prefix##_qsort_help(numbers, 0, array_size - 1);				\
}

/**
  The top-k package works on an array of structures.  The structure has an field 'key',
  which can be compared by ">".

  The struct need not necessarily be itopk_t or ftopk_t.
  */


/* Initialize a top-k array.  The array should have k elements.  The value "minimal"
   is considered -inf, and should not be used by a real array element. */
#define TOPK_INIT(array, key, k, minimal)		\
	do {						\
		int iiii;				\
		for (iiii = 0; iiii < (k); iiii++) {	\
			(array)[iiii].key = (minimal);	\
		}					\
	} while (0)					\

#define TOPK_INSERT_MAX(array, key, k, el)		\
	({						\
	int inserted = 0;				\
	do {						\
		int iiii, llll, rrrr;			\
		if ((el).key < (array)[0].key) break;	\
		iiii = 0;				\
		for (;;) {				\
			llll = (iiii << 1) + 1;		\
			if (llll >= (k)) break;		\
			rrrr = llll + 1;		\
			if ((rrrr < (k)) && ((array)[rrrr].key < (array)[llll].key)) { \
				llll = rrrr;		\
			}				\
			if ((el).key < (array)[llll].key) break;	\
			(array)[iiii] = (array)[llll];	\
			iiii = llll;			\
		}					\
		(array)[iiii] = (el);			\
	 	inserted = 1;				\
	} while (0);					\
	inserted;					\
	})

#define TOPK_INSERT_MIN(array, key, k, el)		\
	do {						\
		int iiii, llll, rrrr;			\
		if ((el).key > (array)[0].key) break;	\
		iiii = 0;				\
		for (;;) {				\
			llll = (iiii << 1) + 1;		\
			if (llll >= (k)) break;		\
			rrrr = llll + 1;		\
			if ((rrrr < (k)) && ((array)[rrrr].key > (array)[llll].key)) { \
				llll = rrrr;		\
			}				\
			if ((el).key > (array)[llll].key) break;	\
			(array)[iiii] = (array)[llll];	\
			iiii = llll;			\
		}					\
		(array)[iiii] = (el);			\
	} while (0)

#define TOPK_INSERT_MIN_UNIQ(array, key, index, k, el)	\
	 do {						\
	 	int iiii, jjjj;				\
	 	iiii = jjjj = 0;			\
	 	while (iiii < k) {			\
	 		if ((el).key > (array)[iiii].key) break; \
	 		if ((el).index == (array)[iiii].index) { jjjj = 1; break; } \
	 		iiii++;				\
	 	}					\
	 	if (jjjj) break;			\
	 	if (iiii == 0) break;			\
		for (jjjj = 0; jjjj < iiii-1; jjjj++)	\
			(array)[jjjj] = (array)[jjjj+1];\
		(array)[jjjj] = el;			\
	 } while(0)

#define TOPK_INSERT_MIN_UNIQ_DO(array, key, index, k, el, stmt) \
	 do {						\
	 	int iiii, jjjj;				\
	 	iiii = jjjj = 0;			\
	 	while (iiii < k) {			\
	 		if ((el).key > (array)[iiii].key) break; \
	 		if ((el).index == (array)[iiii].index) { jjjj = 1; break; } \
	 		iiii++;				\
	 	}					\
	 	if (jjjj) break;			\
	 	if (iiii == 0) break;			\
		for (jjjj = 0; jjjj < iiii-1; jjjj++)	\
			(array)[jjjj] = (array)[jjjj+1];\
		(array)[jjjj] = el;			\
		 { stmt; }				\
	 } while(0)

#define TOPK_SORT_MIN(array, type, key, k)		\
	do {						\
 		int __last = k;				\
	 	type __el;				\
	 	int iiii, llll, rrrr;			\
	 	while (__last > 0) {			\
	 		__last--;			\
			__el = (array)[__last];		\
			(array)[__last] = (array)[0];	\
			iiii = 0;			\
			for (;;) {			\
				llll = (iiii << 1) + 1;	\
				if (llll >= __last) break;	\
				rrrr = llll + 1;	\
				if ((rrrr < __last) && ((array)[rrrr].key > (array)[llll].key)) { \
					llll = rrrr;	\
				}			\
				if (__el.key > (array)[llll].key) break;\
				(array)[iiii] = (array)[llll];	\
				iiii = llll;		\
			}				\
			(array)[iiii] = __el;		\
	 	}					\
	 } while (0)


QUICKSORT_PROTOTYPE(ftopk, ftopk_t);

QUICKSORT_PROTOTYPE(ftopk_rev, ftopk_t);

QUICKSORT_PROTOTYPE(itopk, itopk_t);

QUICKSORT_PROTOTYPE(itopk_rev, itopk_t);

#endif

