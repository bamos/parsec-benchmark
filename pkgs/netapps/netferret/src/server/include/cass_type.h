#ifndef CASS_TYPE
#define CASS_TYPE
#include <stdint.h>
#include <limits.h>
//typedef union {float f; int32_t i;} cass_featprec_t;
typedef float cass_dist_t;

#define CASS_DIST_MAX	MAXFLOAT

#ifndef uchar
typedef unsigned char uchar;
#endif

typedef unsigned char chunk_t;	/* for bit vector */
#define CHUNK_BIT	(sizeof(chunk_t) * 8)

#define CASS_ID_MAX UINT_MAX
typedef uint32_t cass_size_t;	/* replacement of size_t */


#define CASS_ID_INV UINT_MAX
typedef uint32_t cass_id_t;

typedef cass_id_t cass_vec_id_t;
typedef cass_id_t cass_vecset_id_t;

typedef uint32_t cass_vec_dist_id_t;
typedef uint32_t cass_vecset_dist_id_t;

typedef uint32_t cass_vecset_type_t;
typedef uint32_t cass_vec_type_t;
typedef uint32_t cass_vecset_dist_type_t;
typedef uint32_t cass_vec_dist_type_t;

#ifndef uint
typedef unsigned int uint;
#endif

#endif

