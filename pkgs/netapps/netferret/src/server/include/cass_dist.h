#ifndef __WDONG_DIST__
#define __WDONG_DIST__

/* type = int | float */
extern int32_t chunk_cnt [];

/*
#if sizeof(chunk_t) != 1
#error If you changed chunk_t to other types, update dist_asym_* as well.
#endif
*/

static inline int32_t dist_hamming (cass_size_t n, const chunk_t *c1, const chunk_t *c2)
/* n = M / CHUNK_BIT */
{
	cass_size_t dist = 0, i;
	for (i = 0; i < n; i++)
	{
		dist += chunk_cnt[c1[i] ^ c2[i]];
	}
	return dist;
}

/* Generate distance functions with/without weight/threshold */

#define GEN_DIST(type)\
\
static inline type dist_L2_##type (cass_size_t D, const type *P1, const type *P2)\
{\
	type result;\
	type tmp;\
	cass_size_t i;\
	result = 0;\
	for (i = 0; i < D; i++)\
	{\
		tmp = P1[i] - P2[i];\
		tmp *= tmp;\
		result += tmp;\
	}\
	return sqrt(result);\
}\
\
static inline type dist_L2_##type##_W (cass_size_t D, const type *P1, const type *P2, const type *weight)\
{\
	type result;\
	type tmp;\
	cass_size_t i;\
	result = 0;\
	for (i = 0; i < D; i++)\
	{\
		tmp = P1[i] - P2[i];\
		tmp *= tmp;\
		tmp *= weight[i];\
		result += tmp;\
	}\
	return sqrt(result);\
}\
\
static inline type dist_L2_##type##_T (cass_size_t D, const type *P1, const type *P2, type T)\
{\
	type result;\
	type tmp;\
	cass_size_t i;\
	result = 0;\
	for (i = 0; i < D; i++)\
	{\
		tmp = P1[i] - P2[i];\
		tmp *= tmp;\
		result += tmp;\
		if (result > T * T) break;\
	}\
	return sqrt(result);\
}\
\
static inline type dist_L1_##type (cass_size_t D, const type *P1, const type *P2)\
{\
	type result;\
	type tmp;\
	cass_size_t i;\
	result = 0;\
	for (i = 0; i < D; i++)\
	{\
		tmp = P1[i] - P2[i];\
		result += tmp >= 0 ? tmp : -tmp;\
	}\
	return result;\
}\
\
static inline type dist_L1_##type##_W (cass_size_t D, const type *P1, const type *P2, const type *weight)\
{\
	type result;\
	type tmp;\
	cass_size_t i;\
	result = 0;\
	for (i = 0; i < D; i++)\
	{\
		tmp = P1[i] - P2[i];\
		result += weight[i] * (tmp >= 0 ? tmp : -tmp);\
	}\
	return result;\
}\
static inline type dist_cos_##type (cass_size_t D, const type *P1, const type *P2)\
{\
	type result;\
	cass_size_t i;\
	result = 0;\
	for (i = 0; i < D; i++)\
	{\
		result += P1[i]*P2[i];\
	}\
	return result;\
}\


GEN_DIST(int32_t);
GEN_DIST(float);

#endif

