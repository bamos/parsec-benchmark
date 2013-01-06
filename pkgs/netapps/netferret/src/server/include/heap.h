#if defined(__cplusplus)
extern "C" {
#endif

typedef struct Heap Heap;

Heap		*mkheap(int (*)(void*, void*), int, int);
void		freeheap(Heap*);
void		heapreset(Heap*);
void		*heapmin(Heap*);
int		heapsize(Heap*);
int		heapmaxsize(Heap*);
int		heapinsert(Heap*, void*);
int		heapextractmin(Heap*, void*);

#undef howmany
#undef roundup
#undef roundup2
#undef powerof2
#undef rounddown

#define powerof2(x)	(((x)&((x)-1))==0)
#define howmany(x, y)	(((x)+((y)-1))/(y))
#define roundup(x, y)	((((x)+((y)-1))/(y))*(y))
#define roundup2(x, y)	(((x)+((y)-1))&(~((y)-1)))
#define	rounddown(x, y)	(((x)/(y))*(y))

#define	bitbyte(bit)		((bit) >> 3)
#define bitmask(bit)		(1<<((bit)&0x7))
#define bitsize(nbits)		(((nbits)+7)>>3)
#define bittest(map, bit)	((map)[bitbyte(bit)]&bitmask(bit))
#define bitisset(map, bit)	((map)[bitbyte(bit)]&bitmask(bit))
#define bitisclear(map, bit)	(((map)[bitbyte(bit)]&bitmask(bit))==0)
#define bitset(map, bit)	((map)[bitbyte(bit)] |= bitmask(bit))
#define bitclear(map, bit)	((map)[bitbyte(bit)] &= ~bitmask(bit))

#if defined(__cplusplus)
}
#endif
