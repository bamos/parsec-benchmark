#ifdef __WDONG_ARRAY__
#ifndef __WDONG_HEAP__
#define __WDONG_HEAP__

/* works directly on an array.  Need array.h included */

#define HEAP_EMPTY(heap)	((heap).len == 0)
#define HEAP_HEAD(heap)		(heap).data[0]

#define HEAP_ENQUEUE(heap, d, ge)			\
	do {					\
		int _i_ = (heap).len, _j_;	\
		(heap).len++;			\
		ARRAY_EXPAND(heap,(heap).len);\
		while (_i_ > 0) {		\
			_j_ = _i_ >> 1;		\
			if (ge(&d, &(heap).data[_j_])) break; \
			(heap).data[_i_] = (heap).data[_j_]; \
			_i_ = _j_;		\
		}				\
		(heap).data[_i_] = d;		\
	} while(0)

#define HEAP_ENQUEUE_UPDATE(heap, d, ge, update)\
	do {					\
		int _i_ = (heap).len, _j_;	\
		(heap).len++;			\
		ARRAY_EXPAND(heap,(heap).len);\
		while (_i_ > 0) {		\
			_j_ = _i_ >> 1;		\
			if (ge(&d, &(heap).data[_j_])) break; \
			(heap).data[_i_] = (heap).data[_j_]; \
			update(&(heap).data[_i_], _i_); \
			_i_ = _j_;		\
		}				\
		(heap).data[_i_] = d;		\
		update(&(heap).data[_i_], _i_);	\
	} while(0)

#define HEAP_DEQUEUE(heap, ge)			\
	do {					\
		int _i_, _l_, _r_,  _e_;	\
		assert((heap).len > 0);		\
		(heap).len--;			\
		_e_ = (heap).len;		\
		_i_ = 0;			\
		for (;;) {			\
			_l_ = (_i_ << 1) + 1;	\
			if (_l_ >= _e_) break;	\
			_r_ = _l_ + 1;		\
			if ((_r_ < _e_) && (ge(&(heap).data[_l_], &(heap).data[_r_]))) _l_  = _r_; \
			if (ge(&(heap).data[_l_], &(heap).data[_e_])) break; \
			(heap).data[_i_] = (heap).data[_l_]; \
			_i_ = _l_;		\
		}				\
		(heap).data[_i_] = (heap).data[_e_]; \
	} while (0)

#define HEAP_DEQUEUE_UPDATE(heap, ge, update)	\
	do {					\
		int _i_, _l_, _r_,  _e_;	\
		assert((heap).len > 0);		\
		(heap).len--;			\
		_e_ = (heap).len;		\
		_i_ = 0;			\
		update(&(heap).data[0], -1);	\
		for (;;) {			\
			_l_ = (_i_ << 1) + 1;	\
			if (_l_ >= _e_) break;	\
			_r_ = _l_ + 1;		\
			if ((_r_ < _e_) && (ge(&(heap).data[_l_], &(heap).data[_r_]))) _l_  = _r_; \
			if (ge(&(heap).data[_l_], &(heap).data[_e_])) break; \
			(heap).data[_i_] = (heap).data[_l_]; \
			update(&(heap).data[_i_], _i_);	\
			_i_ = _l_;		\
		}				\
		(heap).data[_i_] = (heap).data[_e_]; \
		update(&(heap).data[_i_], _i_);	\
	} while (0)

#endif
#endif
