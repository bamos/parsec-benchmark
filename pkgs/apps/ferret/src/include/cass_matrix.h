#ifndef MATRIX
#define MATRIX

void __matrix_check (void **matrix);
cass_size_t __matrix_col (void **matrix, cass_size_t size);
cass_size_t __matrix_row (void **matrix);
void **__array2matrix (cass_size_t row, cass_size_t col, cass_size_t size, void *data);
void *__matrix2array (void **);	/* free the matrix index structure */
void **__matrix_alloc (cass_size_t row, cass_size_t col, cass_size_t size);
void ***__matrix3_alloc (cass_size_t num, cass_size_t row, cass_size_t col, cass_size_t size);
void **__matrix_dup (cass_size_t row, cass_size_t col, cass_size_t size, void **data);
void __matrix_free (void **data);
void __matrix3_free (void ***data);

int matrix_load_stream_compat (CASS_FILE *fin, cass_size_t size, cass_size_t *row, cass_size_t *col, void ***matrix);

int matrix_load_stream_x (CASS_FILE *fin, cass_size_t *size, cass_size_t *row, cass_size_t *col, void ***matrix, cass_size_t nrow);
int matrix_load_stream (CASS_FILE *fin, cass_size_t *size, cass_size_t *row, cass_size_t *col, void ***matrix);
int matrix_dump_stream (CASS_FILE *fout, cass_size_t size, cass_size_t row, cass_size_t col, void **matrix);

int matrix_load_file_head (const char *filename, cass_size_t *size, cass_size_t *row, cass_size_t *col);

int matrix_load_file (const char *filename, cass_size_t *size, cass_size_t *row, cass_size_t *col, void ***matrix);
int matrix_dump_file (const char *filename, cass_size_t size, cass_size_t row, cass_size_t col, void **matrix);

/* read only */
int matrix_map_file (const char *filename, cass_size_t *size, cass_size_t *row, cass_size_t *col, void ***matrix);

int matrix_unmap_file (void **matrix);

#define type_alloc(type)		((type*)malloc(sizeof(type)))
#define type_calloc(type,col)		((type*)calloc(sizeof(type), col))
#define type_realloc(type,ptr, col)	((type*)realloc(ptr, sizeof(type)*col))

#define matrix_check(matrix)		__matrix_check((void **)matrix)
#define type_matrix_row(matrix,type)	__matrix_row((void **)matrix)
#define type_matrix_col(matrix,type)	__matrix_col((void **)matrix, sizeof(type))
#define type_array2matrix(type,row,col,data)	(type**)__array2matrix(row,col,sizeof (type), (void*)data)
#define type_matrix2array(type,data)	(type *)__matrix2array((void **)data)
#define type_matrix_data(type,data)	(((type **)data)[0])
#define type_matrix_alloc(type,row,col)	(type **)__matrix_alloc(row, col, sizeof (type))
#define type_matrix3_alloc(type,num,row,col)	(type ***)__matrix3_alloc(num,row,col, sizeof (type))
#define type_matrix_dup(type,row,col,data)	(type**)__matrix_dup(row, col, sizeof (type), (void *)data)
#define matrix_free(data)		__matrix_free((void **)data)
#define matrix3_free(data)		__matrix3_free((void ***)data)
#define matrix_free_index(data)		free((void *)data)

#define type_matrix_load_stream(type,fin,row,col,matrix) \
	do { \
		cass_size_t ___size; \
		void **___p;	\
		matrix_load_stream(fin,&___size,row,col, &___p); \
		assert(___size == sizeof(type)); \
		*matrix = ___p;	\
	} while (0)

#define type_matrix_load_stream_x(type,fin,row,col,matrix,cnt) \
	do { \
		cass_size_t ___size; \
		void **___p;	\
		matrix_load_stream_x(fin,&___size,row,col,&___P,cnt); \
		assert(___size == sizeof(type)); \
		*matrix = ___p;	\
	} while (0)

#define type_matrix_dump_stream(type,fout,row,col,matrix) \
	matrix_dump_stream(fout,sizeof(type),row,col,(void **)matrix)

#define type_matrix_load_file(type,fin,row,col,matrix) \
	do { \
		cass_size_t ___size; \
		void **___p;	\
		matrix_load_file(fin,&___size,row,col, &___p); \
		assert(___size == sizeof(type)); \
		*matrix = ___p;	\
	} while (0)

#define type_matrix_map_file(type,fin,row,col,matrix) \
	do { \
		cass_size_t ___size; \
		void **___p;	\
		matrix_map_file(fin,&___size,row,col,&___p); \
		assert(___size == sizeof(type)); \
		*matrix = ___p;	\
	} while (0)

#define type_matrix_dump_file(type,fout,row,col,matrix) \
	matrix_dump_file(fout,sizeof(type),row,col,(void **)matrix)

#endif

