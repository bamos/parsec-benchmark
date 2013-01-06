#ifndef __WDONG_ARRAY__
#define __WDONG_ARRAY__

#define ARRAY_DEFAULT_INC	256

#define ARRAY_TYPE(type)			\
	struct {				\
		cass_size_t inc;			\
		cass_size_t size;			\
		cass_size_t len;			\
		type *data;			\
	}

#define ARRAY_WRAPPER(data, len) {0, len, len, data}

#define ARRAY_SET_INC(array,_inc)		\
	do {					\
		/* assert(((_inc) & (_inc - 1)) == 0); */\
		(array).inc = _inc;		\
	} while (0)

#define ARRAY_INIT(array)			\
	do {					\
		(array).inc = ARRAY_DEFAULT_INC;\
	       	(array).size = 0;		\
		(array).len = 0;		\
		(array).data = NULL;		\
	} while (0)

#define ARRAY_INIT_SIZE(array,_size)		\
	do {					\
		(array).inc = ARRAY_DEFAULT_INC;\
		(array).size = _size;		\
		(array).len = 0;		\
		(array).data = NULL;		\
		if (_size > 0){			\
		(array).data = malloc(_size * sizeof(*(array).data));\
		assert((array).data != NULL);	\
		}				\
	} while(0)

#define ARRAY_INIT_SIZE_PP(array, type, _size)		\
	do {					\
		(array).inc = ARRAY_DEFAULT_INC;\
		(array).size = _size;		\
		(array).len = 0;		\
		(array).data = NULL;		\
		if (_size > 0){			\
		(array).data = (type *)malloc(_size * sizeof(*(array).data));\
		assert((array).data != NULL);	\
		}				\
	} while(0)

#define ARRAY_SIZE(array)			\
	((array).size)

#define ARRAY_INC(array)			\
	((array).inc)

#define ARRAY_RAW_SIZE(array)			\
	((cass_size_t)((array).len * sizeof(*(array).data)))

#define ARRAY_BEGIN_READ_RAW(array, _data, _len) \
	do { _data = (array).data; _len = (array).len; } while(0)

#define ARRAY_END_READ_RAW(array)

#define ARRAY_BEGIN_WRITE_RAW(array, _data, _len) \
	do { _data = (array).data; _len = (array).len; } while(0)

#define ARRAY_END_WRITE_RAW(array, _len)		\
	do {(array).len = _len; assert(_len <= (array).size); } while (0)

#define ARRAY_CLEANUP(array)			\
	do { if ((array).data != NULL) free((array).data); (array).data = NULL; (array).size = (array).len = 0; } while (0)

#define ARRAY_LEN(array)			\
	((array).len)

/*
#define ARRAY_GET(array,n)			\
	({ assert(n < (array).len); (array).data[n];})
	*/

#define ARRAY_GET(array,n)			\
	((array).data[n])

#define ARRAY_SET(array,n,d)			\
	do { assert(n < (array).len); (array).data[n] = d; } while (0)

#define ARRAY_EXPAND(array,_size)		\
	do {					\
		if ((array).size >= _size) break;\
		(array).size = _size;		\
		(array).size += (array).inc - 1;\
		(array).size &= ~((array).inc - 1);\
		if ((array).data != NULL) \
		(array).data = realloc((array).data, (array).size * sizeof(*(array).data)); \
		else \
		(array).data = malloc((array).size * sizeof(*(array).data)); \
		assert((array).data != NULL);	\
	} while(0)

#define ARRAY_APPEND(array,d)		\
	do {					\
		int len = (array).len;		\
		(array).len++;			\
		ARRAY_EXPAND(array,(array).len);\
		(array).data[len] = d;		\
	} while(0)

#define ARRAY_APPEND_UNSAFE(array,d)		\
	do {					\
		(array).data[(array).len++] = d;\
	} while (0)


#define ARRAY_TRUNC(array)			\
	do {					\
		(array).len = 0;		\
	} while(0)

#define ARRAY_MERGE(array1,array2)		\
	do {					\
		int len = (array1).len;		\
		assert(sizeof(*(array1).data) == sizeof(*(array2).data)); \
		(array1).len += (array2).len;	\
		ARRAY_EXPAND(array1, ((array1).len));	\
		memcpy((array1).data + len, (array2).data, ARRAY_RAW_SIZE(array2));\
	} while (0)

#define ARRAY_MERGE_RAW(array1,data,_len)	\
	do {					\
		int len = (array1).len;		\
		assert(sizeof(*(array1).data) == sizeof(*data));\
		(array1).len += _len / sizeof(*(array1).data);	\
		ARRAY_EXPAND(array1, ((array1).len));		\
		memcpy((array1).data + len, data, _len * sizeof(*data));\
	} while (0)

#define ARRAY_BEGIN_FOREACH(array, cursor)						\
	do {										\
		cass_size_t __array_foreach_index;			\
		for (__array_foreach_index = 0; __array_foreach_index < (array).len; __array_foreach_index++) { \
			cursor = (array).data[__array_foreach_index];

#define ARRAY_END_FOREACH						\
		}							\
	} while (0);

#define ARRAY_BEGIN_FOREACH_P(array,cursor)						\
	do {										\
		cass_size_t __array_foreach_index;			\
		for (__array_foreach_index = 0; __array_foreach_index < (array).len; __array_foreach_index++) { \
			cursor = (array).data + __array_foreach_index;

#define ARRAY_END_FOREACH_P						\
		}							\
	} while (0);

#endif

