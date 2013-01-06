#ifndef	_BITMAP_H_
#define _BITMAP_H_

#include <cass_type.h>

#undef howmany
#undef roundup
#undef roundup2
#undef powerof2
#undef rounddown

#define powerof2(x)	(((x)&((x)-1))==0)
#define howmany(x, y)	(((x)+((y)-1))/(y))
#define roundup(x, y)	((((x)+((y)-1))/(y))*(y))
#define roundup2(x, y)	(((x)+((y)-1))&(~((y)-1)))
#define rounddown(x, y)	(((x)/(y))*(y))

#define	bitbyte(bit)		((bit) >> 3)
#define bitmask(bit)		(1<<((bit)&0x7))
#define bitsize(nbits)		(((nbits)+7)>>3)
#define bittest(map, bit)	((map)[bitbyte(bit)]&bitmask(bit))
#define bitisset(map, bit)	((map)[bitbyte(bit)]&bitmask(bit))
#define bitisclear(map, bit)	(((map)[bitbyte(bit)]&bitmask(bit))==0)
#define bitset(map, bit)	((map)[bitbyte(bit)] |= bitmask(bit))
#define bitclear(map, bit)	((map)[bitbyte(bit)] &= ~bitmask(bit))

typedef struct bitmap_t {
    uint32_t  size;       /* size in bits */
    uint32_t  count;      /* number of non-zero entries */
    uchar   *vec;
} bitmap_t;

bitmap_t *bitmap_new(uint32_t size);

int	bitmap_free(bitmap_t **map); 

int bitmap_init(bitmap_t *map, uint32_t size); 

int bitmap_clear(bitmap_t *map); 

int bitmap_free_vec(bitmap_t *map); 

uint32_t bitmap_get_count(bitmap_t *map); 

uint32_t bitmap_get_size(bitmap_t *map);

int bitmap_print(bitmap_t *map); 

int bitmap_insert(bitmap_t *map, uint32_t x); 

int	bitmap_contain(bitmap_t *map, uint32_t x); 

int bitmap_union(bitmap_t *mapA, bitmap_t *mapB); 

uint32_t bitmap_getNext(bitmap_t *map, uint32_t cur); 

#endif /* _BITMAP_H_ */
