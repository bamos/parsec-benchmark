#ifndef _CASS_H_
#define _CASS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <values.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif


extern void __debug (const char *, int, const char *, const char *, ...);

#ifndef __func__
#define __func__ "__func__"
#endif

#define info(...) do { printf(__VA_ARGS__); fflush(stdout); } while (0)
#define warn(...) do { fprintf(stderr, __VA_ARGS__); } while (0)
#define fatal(...) do { fprintf(stderr, __VA_ARGS__); exit(-1); } while (0)
#define debug(...) __debug(__FILE__, __LINE__, __func__, __VA_ARGS__)

#include <cass_type.h>

enum {
    CASS_ERR_UNKNOWN = -9999,
    CASS_ERR_OUTOFMEM,  // following enum will be also negative.
    CASS_ERR_MALFORMATVEC,  
    CASS_ERR_PARAMETER,
    CASS_ERR_IO,
    CASS_ERR_CORRUPTED
    // others.
}; 

const char *cass_strerror (int err);

int cass_init (void);
int cass_cleanup (void);

#include <cass_array.h>
#include <cass_topk.h>
#include <cass_file.h>
#include <cass_matrix.h>
#include <cass_reg.h>
#include <cass_bitmap.h>

enum {
//    CASS_DATAOBJ_IS_VECSETID = 1 << 3,
    CASS_MAPPING      = 1<<3,

    CASS_VECSET_ID_BITMAP = 1 << 4,
    CASS_VECSET_ID_ARRAY = 1 << 5,
};

enum {
    CASS_DATA 		= 0x1,
    CASS_VEC_INDEX	= 0x2,
    CASS_VECSET_INDEX	= 0x4,
    CASS_VEC_SKETCH	= 0x8,
    CASS_VECSET_SKETCH	= 0x10,
    CASS_OUTOFCORE 	= 0x100,
    CASS_SEQUENTIAL	= 0x200
};

enum {
    CASS_READONLY = 1 << 1,
    CASS_EXCL = 1 << 2
};

// hide datalog
/*
typedef struct _cass_datum_t {
    uchar   *data;
    uint32_t size;
    uint32_t ulen;
    uint32_t flags;
} cass_datum_t;
*/

typedef struct _cass_vec_t {
    float weight;
    cass_vecset_id_t parent;
    union {	/* place holder */
	    uchar	data[1];
	    int32_t	int_data[1];
	    float	float_data[1];
//	    double	double_data[0]; -- not supposed to work with 64-bit data
	    chunk_t	bit_data[1];
    } u;
} cass_vec_t;

#define CASS_VEC_HEAD_SIZE	((size_t)(((cass_vec_t *)0)->u.data))


typedef struct _cass_vecset_t {
    uint32_t num_regions;
    cass_vec_id_t start_vecid;
              // Since cass_vec_t is continuously allocated,
              // start_id => start_id + num_regions belongs to the same vecset
} cass_vecset_t;  // Continuously allocated indexed by cass_vecset_id.

enum {
    CASS_NONE = 0,
    CASS_ANY = 0xFFFFFFFFU,

/* cass_vecset_type_t */
    CASS_VECSET_NONE	= 1 << 0,	/* for sketches, which do not store the vecsets */
    CASS_VECSET_SINGLE	= 1 << 1,
    CASS_VECSET_SET	= 1 << 2,

/* cass_vec_type_t */
    CASS_VEC_INT	= 1 << 0,	/* 32bit integer */
    CASS_VEC_FLOAT	= 1 << 1,	/* 32bit float */
    CASS_VEC_BIT	= 1 << 2,	/* bit vector */
    CASS_VEC_QUANT	= 1 << 3,	/* what does QUANT mean? */

/* cass_vecset_dist_t */
    CASS_VECSET_DIST_TYPE_TRIVIAL	= 1 << 0,
    CASS_VECSET_DIST_TYPE_SINGLE	= 1 << 1,
    CASS_VECSET_DIST_TYPE_EMD	= 1 << 2,
 
/* cass_vec_dist_t */
    CASS_VEC_DIST_TYPE_TRIVIAL	= 1 << 1,	/* always return 0 */
    CASS_VEC_DIST_TYPE_ID	= 1 << 2,	/* 0 if exactly the same, 1 otherwise */
    CASS_VEC_DIST_TYPE_L1	= 1 << 3,
    CASS_VEC_DIST_TYPE_L2	= 1 << 4,
    CASS_VEC_DIST_TYPE_MAX	= 1 << 5,
    CASS_VEC_DIST_TYPE_HAMMING	= 1 << 6,
    CASS_VEC_DIST_TYPE_COS	= 1 << 7,
};

typedef struct {
#define CASS_DATASET_VEC	(1 << 0)
#define CASS_DATASET_VECSET	(1 << 1)
#define CASS_DATASET_BOTH	(CASS_DATASET_VEC | CASS_DATASET_VECSET)

/* if CASS_DATASET_VEC only, then the parent id's are not converted when merged*/
    	uint32_t 		flags;
	uint32_t		loaded;
	cass_size_t		vec_size;
	cass_size_t		vec_dim;
	cass_vec_id_t		max_vec;
	cass_vec_id_t		num_vec;
	void			*vec;
	cass_vecset_id_t	max_vecset;
	cass_vecset_id_t	num_vecset;
	cass_vecset_t		*vecset;
} cass_dataset_t;

#define DATASET_VEC(ds, vec_id)	((cass_vec_t *)((char *)(ds)->vec + (vec_id) * (ds)->vec_size))

typedef int (*cass_dataset_map_t) (void *from, void *to, void *param);

int cass_dataset_init (cass_dataset_t *ds, cass_size_t vec_size, cass_size_t vec_dim, uint32_t flags);
/* this function only allocate the space so that ds->max_vec(set) > num_vec(set)
   do not actually change ds->num_vec(set) */
int cass_dataset_grow (cass_dataset_t *ds, cass_size_t num_vecset, cass_size_t num_vec);
int cass_dataset_release (cass_dataset_t *ds);
int cass_dataset_merge (cass_dataset_t *ds, const cass_dataset_t *src, cass_vecset_id_t start, cass_vecset_id_t num, cass_dataset_map_t map, void *map_param);
int cass_dataset_checkpoint (cass_dataset_t *ds, CASS_FILE *);
int cass_dataset_restore (cass_dataset_t *ds, CASS_FILE *);
cass_vecset_id_t cass_dataset_vec2vecset (cass_dataset_t *ds, cass_vec_id_t id);


int cass_dataset_load (cass_dataset_t *ds, CASS_FILE *in, cass_vec_type_t vec_type);
int cass_dataset_dump (cass_dataset_t *ds, CASS_FILE *out);

/* ================ ENVIRONMENT ==================== */

typedef struct _cass_env_t {
    // Internal management data for controlling the whole system 
    // similar to DB_ENV in BDB.
    // Need to hold all the meta data about tables, maps, indexes as well
    // as their checkpoint informations.
    // All the associations.
    uint32_t mode;
    char *base_dir;
    cass_reg_t table;
    cass_reg_t cfg;
    cass_reg_t map;
    cass_reg_t vecset_dist;
    cass_reg_t vec_dist;
} cass_env_t; 

// cass_env functions
// int cass_env_create(cass_env_t **env, uint32_t flags);
int cass_env_open(cass_env_t **env, char *db_home, uint32_t flags); 
      // Automatically recover to a consistent stage in case of crash.
int cass_env_close(cass_env_t *env, uint32_t flags);

int cass_env_errmsg(cass_env_t *env, int error, const char *fmt, ...);

int cass_env_panic(cass_env_t *env, const char *msg); // Can not proceed, fail the system.

int cass_env_checkpoint(cass_env_t *env);

int cass_env_describe (cass_env_t *env, CASS_FILE *);

//int cass_env_restore(cass_env_t *env); 


/* ================ VEC_DIST_CLASS / VECSET_DIST_CLASS ==================== */

typedef cass_dist_t (*cass_vec_dist_func_t) (cass_size_t n, void *, void *, void *);

typedef struct _cass_vec_dist_class {
	char *name;
	cass_vec_type_t vec_type;
	cass_vec_dist_type_t type;
	cass_vec_dist_func_t dist;
	/* void ** is actually cass_vec_dist_t ** */
	int (*describe) (void *, CASS_FILE *);
	int (*construct) (void **, const char *);
	int (*checkpoint) (void *, CASS_FILE *);
	int (*restore) (void **, CASS_FILE *);
	void (*free) (void *);
} cass_vec_dist_class_t;

typedef struct {
	uint32_t refcnt;
	char *name;
	cass_vec_dist_class_t *__class;
	/* private data... */
} cass_vec_dist_t;

#include <cass_dist.h>

extern cass_reg_t cass_vec_dist_class_reg;

static inline int cass_vec_dist_class_init (void)
{
	return cass_reg_init(&cass_vec_dist_class_reg);
}

static inline int cass_vec_dist_class_cleanup (void)
{
	return cass_reg_cleanup(&cass_vec_dist_class_reg);
}

static inline int32_t cass_vec_dist_class_lookup (const char *n)
{
	return cass_reg_lookup(&cass_vec_dist_class_reg, n);
}

static inline int32_t cass_vec_dist_class_find (const cass_vec_dist_class_t *p)
{
	return cass_reg_find(&cass_vec_dist_class_reg, p);
}

static inline cass_vec_dist_class_t *cass_vec_dist_class_get (uint32_t i)
{
	return (cass_vec_dist_class_t *)cass_reg_get(&cass_vec_dist_class_reg, i);
}

static inline int cass_vec_dist_class_add (cass_vec_dist_class_t *p)
{
	return cass_reg_add(&cass_vec_dist_class_reg, p->name, p);
}

typedef cass_dist_t (*cass_vecset_dist_func_t) (cass_dataset_t *, cass_vecset_id_t,
		cass_dataset_t *, cass_vecset_id_t , cass_vec_dist_t *vec_dist, void *);

typedef struct _cass_vecset_dist_class{
	char *name;
	cass_vecset_type_t vecset_type;
	cass_vecset_dist_type_t type;
	cass_vecset_dist_func_t dist;
	/* void ** is actually cass_vecset_dist_t ** */
	int (*describe) (void *, CASS_FILE *);
	int (*construct) (void **, const char *);
	int (*checkpoint) (void *, CASS_FILE *);
	int (*restore) (void **, CASS_FILE *);
	void (*free) (void *);
	/* private data... */
} cass_vecset_dist_class_t;

typedef struct {
	uint32_t refcnt;
	char *name;
	int32_t vec_dist;
	cass_vecset_dist_class_t *__class;
} cass_vecset_dist_t;

extern cass_reg_t cass_vecset_dist_class_reg;

static inline int cass_vecset_dist_class_init (void)
{
	return cass_reg_init(&cass_vecset_dist_class_reg);
}

static inline int cass_vecset_dist_class_cleanup (void)
{
	return cass_reg_cleanup(&cass_vecset_dist_class_reg);
}

static inline int32_t cass_vecset_dist_class_lookup (const char *n)
{
	return cass_reg_lookup(&cass_vecset_dist_class_reg, n);
}

/* find id by pointer */
static inline int32_t cass_vecset_dist_class_find (const cass_vecset_dist_class_t *p)
{
	return cass_reg_find(&cass_vecset_dist_class_reg, p);
}

static inline cass_vecset_dist_class_t *cass_vecset_dist_class_get (uint32_t i)
{
	return (cass_vecset_dist_class_t *)cass_reg_get(&cass_vecset_dist_class_reg, i);
}

static inline int cass_vecset_dist_class_add (cass_vecset_dist_class_t *p)
{
	return cass_reg_add(&cass_vecset_dist_class_reg, p->name, p);
}

/* ================ CONFIG ==================== */

typedef struct _cass_vecset_cfg_t {
    uint32_t refcnt;
    char *name;  // vecset_config can be shared between
                        // different datasets with same feature(eg: color hist)
    cass_vecset_type_t	vecset_type; // TBD
    cass_vec_type_t	vec_type; // float_array, bitvec, vec_quant, etc.
    cass_size_t		vec_dim;
    cass_size_t		vec_size;   // number of bytes
    uint32_t 		flag;  // one flag: data_obj_name can be used as vecset_id.

  //  int32_t vecset_dist_id;
  //  int32_t vec_dist_id;
  //  int data_is_dirty;
  //  int private_data_size;
  //  void *private_data;  // Eg: May include sketch upper/lower bound stuff?
} cass_vecset_cfg_t; // one per vecset type

cass_size_t cass_vec_dim2size (cass_vec_type_t, cass_size_t dim);

int cass_vecset_cfg_describe (cass_vecset_cfg_t *, cass_env_t *, CASS_FILE *);

int cass_vecset_cfg_checkpoint (cass_vecset_cfg_t *vecset_cfg, CASS_FILE *out);

int cass_vecset_cfg_restore (cass_vecset_cfg_t **, CASS_FILE *in);

int cass_vecset_cfg_free (cass_vecset_cfg_t *cfg);

/* ================ MAP ==================== */
struct CKHash_Table_;
typedef struct cass_map_t {
    int magic;
    uint32_t refcnt;
    char *name;
    //    char *dataset_name;   // same dataset can have multiple features.

    struct _cass_env_t *env;    // The containing cass_env.
    uint32_t flag; // id is obj_name.
    ARRAY_TYPE(int32_t) table_ids; //tables that uses the map.

    int loaded;  // Whether mapping is in memory.
    int dirty;

    // Real data, to map vecset_id -> dataobj_name
    ARRAY_TYPE(char *) vtable;
    // Reverse mapping, dataobj_name ->vecset_id
    struct CKHash_Table_ *htable;
} cass_map_t;

// Deal with cass_map_t.
int cass_map_create(cass_map_t **map, cass_env_t *env, char *map_name, uint32_t flag);
     // flag says whether the dataobj name can be treated as id,
     // then translation need no memory overhead.
int cass_map_associate_table(cass_map_t *map, int32_t table_id);
int cass_map_disassociate_table(cass_map_t *map, int32_t table_id);

int cass_map_insert(cass_map_t *map, cass_vecset_id_t *id,
                    char *dataobj_name); // Will return the id assigned.
int cass_map_load(cass_map_t *map);  // bring mapping in-mem.
int cass_map_release(cass_map_t *map);  // release in-mem mapping.

int cass_map_dataobj_to_id(cass_map_t *map, char *dataobj_name, cass_vecset_id_t *id);
int cass_map_id_to_dataobj(cass_map_t *map, cass_vecset_id_t id, char **dataobj_name);

int cass_map_describe(cass_map_t *map, CASS_FILE *);
int cass_map_checkpoint(cass_map_t *map, CASS_FILE *out);
int cass_map_restore(cass_map_t **, cass_env_t *env, CASS_FILE *in);
int cass_map_drop(cass_map_t *map); // destroy on-disk version.
int cass_map_free(cass_map_t *map); // release map struct.

/* ================ QUERY RELATED ==================== */

typedef struct {
	cass_id_t id;
	cass_dist_t dist;
} cass_list_entry_t;

typedef ARRAY_TYPE(cass_list_entry_t) cass_list_t;

QUICKSORT_PROTOTYPE(__cass_list_entry, cass_list_entry_t);

/* see cass_bitmap.h
typedef struct {
	// only work for cass_id_t;
	uint32_t	size;	// size in bits
	uint32_t	count;	// number of non-zero entries 
	uchar		*vec; 	
} cass_bitmap_t;
*/

typedef struct {
	uint32_t flags;
/* flags = combination of the following values */
#define CASS_RESULT_BITMAP	0x1
#define CASS_RESULT_LIST	0x2
#define CASS_RESULT_BITMAPS	0x4
#define CASS_RESULT_LISTS	0x8

#define CASS_RESULT_SORT	0x10
#define CASS_RESULT_DIST	0x20	/* the dist field in list entries are
valid */
#define CASS_RESULT_MALLOC	0x100
#define CASS_RESULT_REALLOC	0x200	/* usermem is not sufficient, memory reallocated */
#define CASS_RESULT_USERMEM	0x400
	union {
	bitmap_t bitmap;
	cass_list_t list;
	ARRAY_TYPE(bitmap_t) bitmaps;
	ARRAY_TYPE(cass_list_t) lists;
	} u;
} cass_result_t;

typedef struct {
	uint32_t flags;	/* the same as the value of cass_result_t.flags,
			soft requirement, hoping the query algorithm will
			return the required format, but not guaranteed. */
	cass_dataset_t	*dataset;	/* the query dataset */
	cass_id_t	vecset_id;	/* and the query vecset_id */

	cass_size_t topk;
	cass_dist_t range;

	const char *extra_params;  // note: we can use more efficient repre if needed.
	cass_result_t *candidate; // NULL means no filter.

	int32_t vec_dist_id;
	int32_t vecset_dist_id;
} cass_query_t;

/* ================ TABLE ==================== */

typedef struct _cass_table {
    uint32_t refcnt;
    char *name;   // Name of the table, (when create/drop, use name)
    char *filename;	// name of the backing storage file

    struct _cass_env_t *env;    // The containing cass_env.
    int32_t parent_id; // table associated with this index.
    int32_t cfg_id;
    int32_t parent_cfg_id;
    int32_t map_id;

    //int32_t default_vec_dist_id;
    //int32_t default_vecset_dist_id;

    ARRAY_TYPE(int32_t) child_ids; // including sketch & index

    // Convention: all cass_table's filename start with "casstb_" + table_name.

    struct _cass_table_opr *opr;  // restored at restore_checkpoint()

/* !!!! loaded & dirty are managed by the cass_table_*** functions, the table
 * opr implementations could use these flags, but should not change them. */

    int loaded;	// as original vecsets_in_memory -- the data are in memoroy 
    int dirty;	// data is dirty
    
    //char *parameters;  // a copy of input paramters, (will be saved in private_data)

    void *__private;  // private data to store index-specific info,

/* for tables keep the following as private data
    cass_vecset_id_t num_vecsets;
    cass_vecset_t *vecsets; // If in memory: array of vecset indexed by
    */
    cass_vecset_cfg_t *cfg;
    cass_vecset_cfg_t *parent_cfg;
    cass_map_t *map;

    ARRAY_TYPE(struct _cass_table *) children; // including sketch & index

} cass_table_t;

typedef struct _cass_table_opr cass_table_opr_t;
/*    table operations */

typedef cass_vecset_cfg_t *cass_table_cfg_t (const char *param);

typedef int cass_table_init_private_t (cass_table_t *table,  // See management section
				const char *param);

typedef int cass_table_opr_describe_t (cass_table_opr_t *table, CASS_FILE *out);

typedef int cass_table_checkpoint_private_t (cass_table_t *table, CASS_FILE *out);
                       // Initiate the checkpoint, upon success,
                       // return the chkpnt_data to store in the
                       // cass_env for checkpointing and future recovery.
                       // Note: this will also checkpoint the meta data for table.
typedef int cass_table_restore_private_t (cass_table_t *table, CASS_FILE *in);                    // This shall also restore the meta data for table.

typedef int cass_table_load_t (cass_table_t *table);  // bring data in mem

typedef int cass_table_release_t (cass_table_t *table);  // release in-mem vecsets.

//typedef int cass_table_insert_t (cass_table_t *table, cass_vecset_id_t id, cass_vecset_t *vecset);

typedef int cass_table_batch_insert_t (cass_table_t *table, cass_dataset_t *dataset, cass_vecset_id_t start, cass_vecset_id_t end);

typedef int cass_table_query_t(cass_table_t *table, 
	cass_query_t *query, cass_result_t *result);

typedef int cass_table_batch_query_t(cass_table_t *table, 
/* the dataset of all the queries should be the same */
	uint32_t count, cass_query_t **queries, cass_result_t **results);

typedef int cass_table_drop_t(cass_table_t *table);
                       // Destroy on-disk datalog.
typedef int cass_table_free_private_t(cass_table_t *table); // free the table struct.

// Note, if data is not in memory, this will trigger on-disk sequential scan.
// Need to add linear scan cursor interface.

typedef char *cass_table_tune_t(cass_table_t *parent, char *extra_input);

struct _cass_table_opr {
    char *name;
    int type;	// CASS_DATA or CASS_SKETCH or CASS_INDEX, CASS_OUTOFCORE, CASS_SEQUENTIAL
    cass_vecset_type_t		vecset_type;
    cass_vec_type_t		vec_type;
    cass_vecset_dist_type_t	dist_vecset;
    cass_vec_dist_type_t	dist_vec;

    cass_table_cfg_t		*cfg;
    cass_table_tune_t		*tune; // a set of function pointers.
    cass_table_init_private_t	*init_private;
//    cass_table_insert_t		*insert;
    cass_table_batch_insert_t	*batch_insert;
    cass_table_query_t		*query;
    cass_table_batch_query_t	*batch_query;
    cass_table_load_t		*load; // load data
    cass_table_release_t	*release; // release data
    cass_table_checkpoint_private_t	*checkpoint_private;
    cass_table_restore_private_t	*restore_private;
    //cass_table_drop_t		*drop;
    cass_table_free_private_t	*free_private;
    cass_table_opr_describe_t	*describe; // load data
};

extern cass_reg_t cass_table_opr_reg;

static inline int cass_table_opr_init (void)
{
	return cass_reg_init(&cass_table_opr_reg);
}

static inline int cass_table_opr_cleanup (void)
{
	return cass_reg_cleanup(&cass_table_opr_reg);
}

static inline int32_t cass_table_opr_lookup (const char *n)
{
	return cass_reg_lookup(&cass_table_opr_reg, n);
}

/* find id by pointer */
static inline int32_t cass_table_opr_find (const cass_table_opr_t *p)
{
	return cass_reg_find(&cass_table_opr_reg, p);
}

static inline cass_table_opr_t *cass_table_opr_get (uint32_t i)
{
	return (cass_table_opr_t *)cass_reg_get(&cass_table_opr_reg, i);
}

static inline int cass_table_opr_add (const char *n, cass_table_opr_t *p)
{
	return cass_reg_add(&cass_table_opr_reg, n, p);
}

int cass_table_create (cass_table_t **table,
				cass_env_t *env,  // See management section
                                char *table_name,
				uint32_t opr_idx,
                                int32_t cfg_id,
                                int32_t parent_id,
				int32_t parent_cfg_id,
                                int32_t map_id,
				char *param);

int cass_table_free(cass_table_t *table);

int cass_table_describe (cass_table_t *, CASS_FILE *);

int cass_table_restore (cass_table_t **, cass_env_t *env, CASS_FILE *in);

int cass_table_checkpoint (cass_table_t *, CASS_FILE *out);

/* This function is called after every table is restored, then all the
 * cfg/map/children ids are converted into pointers, and the corresponding
 * reference counts are incremented, and also, put in the self_id. */
int __cass_table_ref (cass_table_t *);
/* decrement the reference count. */
int __cass_table_unref (cass_table_t *);

int cass_table_import_data(cass_table_t *table, char *fname); 
int cass_table_export_data(cass_table_t *table, char *fname); 

int cass_table_associate (cass_table_t *table, int32_t tbl2_id);

int cass_table_disassociate (cass_table_t *table, int32_t tbl2_id);

cass_table_t *cass_table_parent (cass_table_t *table);

static inline int cass_table_load (cass_table_t *table)
{
	int ret = table->opr->load(table);
	if (ret == 0) table->loaded = 1;
	return ret;
}

static inline int cass_table_release (cass_table_t *table)
{
	int ret =  table->opr->release(table);
	if (ret == 0) table->loaded = 0;
	return ret;
}

//int cass_table_insert (cass_table_t *table, cass_vecset_id_t id, cass_vecset_t *vecset);

int cass_table_batch_insert (cass_table_t *table, cass_dataset_t *dataset, cass_vecset_id_t start, cass_vecset_id_t end);

int cass_table_query (cass_table_t *table,
                           cass_query_t *query, cass_result_t *result);

int cass_table_batch_query (cass_table_t *table, uint32_t count,
                           cass_query_t **queries, cass_result_t **results);

/* ================ UTIL FUNCTIONS ==================== */

int mkpath (char *buf, const char *base, const char *table, const char *ext);

/* non-0 if exist, 0 if not */
int dexist (const char *path); /* dir */
int fexist (const char *path); /* file */

struct raw_private
{
	cass_dataset_t dataset;
};

cass_size_t grow (cass_size_t from, cass_size_t to);

const char *cass_vec_type_name (uint32_t );
const char *cass_vecset_type_name (uint32_t );
const char *cass_vec_dist_type_name (uint32_t );
const char *cass_vecset_dist_type_name (uint32_t );

/* get value from parameter list */
/* Eg., to get from "-N 18 -L 32" the value 18, use param_get_int("-N 18 -L 32",
 * "-N");
 */
int32_t param_get_int (const char *,  const char *, int32_t def /* default */);
float param_get_float (const char *,  const char *, float def /* default */);
int param_get_float_array (const char *,  const char *, cass_size_t *n, float *f /* default */);

extern cass_table_opr_t opr_raw;
extern cass_table_opr_t opr_lsh; 
//extern cass_table_opr_t opr_tree; 

extern cass_vecset_dist_class_t vecset_dist_trivial;
extern cass_vecset_dist_class_t vecset_dist_single;
extern cass_vecset_dist_class_t vecset_dist_emd;
extern cass_vecset_dist_class_t vecset_dist_myemd;

extern cass_vec_dist_class_t vec_dist_trivial;
extern cass_vec_dist_class_t vec_dist_L1_int;
extern cass_vec_dist_class_t vec_dist_L2_int;
extern cass_vec_dist_class_t vec_dist_L1_float;
extern cass_vec_dist_class_t vec_dist_L2_float;
extern cass_vec_dist_class_t vec_dist_hamming;
extern cass_vec_dist_class_t vec_dist_cos_float;


/* the following functions are shared among the sketches */
typedef int (*sketch_func_t) (chunk_t *out, const void *in, void *sketch);
typedef int (*sketch_asym_func_t) (float *out, const void *in, void *sketch);

int __vec_sketch_query(cass_table_t *table, 
                     cass_query_t *query, cass_result_t *result, sketch_func_t sketch, void *param);

int __vec_sketch_query_asym(cass_table_t *table, 
                     cass_query_t *query, cass_result_t *result, sketch_asym_func_t sketch, void *param);

int cass_result_alloc_list (cass_result_t *result, cass_size_t num_regions, cass_size_t topk);

int cass_result_alloc_bitmap (cass_result_t *result, cass_size_t num_regions, cass_size_t size);

int cass_result_merge_bitmaps(cass_result_t *src, cass_result_t *dst); 

int cass_result_free (cass_result_t *);

cass_result_t *
cass_result_merge_lists (cass_result_t *src, cass_dataset_t *ds, int32_t topk);

#ifdef __cplusplus
};
#endif
#endif

