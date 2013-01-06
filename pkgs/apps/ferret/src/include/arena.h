#ifndef _ARENA_H_
#define _ARENA_H_
#if defined(__cplusplus)
extern "C" { 
#endif

typedef struct MemArena MemArena;

MemArena	*mkmemarena(void *(*)(size_t), void *(*)(void*, size_t), void (*)(void*), unsigned long);
void		freememarena(MemArena*);
void		memarenastats(MemArena*);
void		*memarenamalloc(MemArena*, unsigned long);
#if defined(__cplusplus)
}
#endif
#endif
