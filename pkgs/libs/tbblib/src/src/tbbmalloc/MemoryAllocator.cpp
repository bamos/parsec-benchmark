/*
    Copyright 2005-2010 Intel Corporation.  All Rights Reserved.

    This file is part of Threading Building Blocks.

    Threading Building Blocks is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    Threading Building Blocks is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Threading Building Blocks; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    As a special exception, you may use this file as part of a free software
    library without restriction.  Specifically, if other files instantiate
    templates or use macros or inline functions from this file, or you compile
    this file and link it with other files to produce an executable, this
    file does not by itself cause the resulting executable to be covered by
    the GNU General Public License.  This exception does not however
    invalidate any other reasons why the executable file might be covered by
    the GNU General Public License.
*/


#include "TypeDefinitions.h" /* Also includes customization layer Customize.h */

#if USE_PTHREAD
    // Some pthreads documentation says that <pthreads.h> must be first header.
    #include <pthread.h>
    #define TlsSetValue_func pthread_setspecific
    #define TlsGetValue_func pthread_getspecific
    typedef pthread_key_t tls_key_t;
    #include <sched.h>
    inline void do_yield() {sched_yield();}

#elif USE_WINTHREAD
    #define _WIN32_WINNT 0x0400
    #include <windows.h>
    #define TlsSetValue_func TlsSetValue
    #define TlsGetValue_func TlsGetValue
    typedef DWORD tls_key_t;
    inline void do_yield() {SwitchToThread();}

#else
    #error Must define USE_PTHREAD or USE_WINTHREAD

#endif

// intrin.h available since VS2005
#if defined(_MSC_VER) && _MSC_VER >= 1400
#define __TBB_HAS_INTRIN_H 1
#else
#define __TBB_HAS_INTRIN_H 0
#endif

#if __sun || __SUNPRO_CC
#define __asm__ asm 
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <new>        /* for placement new */
#if __TBB_HAS_INTRIN_H
#include <intrin.h>   /* for __cpuid */
#endif

extern "C" {
    void * scalable_malloc(size_t size);
    void   scalable_free(void *object);
    void mallocThreadShutdownNotification(void*);
}

/********* Various compile-time options        **************/

#define MALLOC_TRACE 0

#if MALLOC_TRACE
#define TRACEF(x) printf x
#else
#define TRACEF(x) ((void)0)
#endif /* MALLOC_TRACE */

#define ASSERT_TEXT NULL

//! Define the main synchronization method
/** It should be specified before including LifoList.h */
#define FINE_GRAIN_LOCKS
#include "LifoList.h"

#define COLLECT_STATISTICS MALLOC_DEBUG && defined(MALLOCENV_COLLECT_STATISTICS)
#include "Statistics.h"

#define FREELIST_NONBLOCKING 1

// If USE_MALLOC_FOR_LARGE_OBJECT is nonzero, then large allocations are done via malloc.
// Otherwise large allocations are done using the scalable allocator's block allocator.
// As of 06.Jun.17, using malloc is about 10x faster on Linux.
#if !_WIN32
#define USE_MALLOC_FOR_LARGE_OBJECT 1
#endif

/********* End compile-time options        **************/

namespace rml {

namespace internal {

/******* A helper class to support overriding malloc with scalable_malloc *******/
#if MALLOC_CHECK_RECURSION

inline bool isMallocInitialized();

class RecursiveMallocCallProtector {
    // pointer to an automatic data of holding thread
    static void       *autoObjPtr;
    static MallocMutex rmc_mutex;
    static pthread_t   owner_thread;
/* Under FreeBSD 8.0 1st call to any pthread function including pthread_self
   leads to pthread initialization, that causes malloc calls. As 1st usage of
   RecursiveMallocCallProtector can be before pthread initialized, pthread calls
   can't be used in 1st instance of RecursiveMallocCallProtector.
   RecursiveMallocCallProtector is used 1st time in checkInitialization(),
   so there is a guarantee that on 2nd usage pthread is initialized. 
   No such situation observed with other supported OSes.
 */
#if __FreeBSD__
    static bool        canUsePthread;
#else
    static const bool  canUsePthread = true;
#endif
/*
  The variable modified in checkInitialization,
  so can be read without memory barriers.
 */
    static bool mallocRecursionDetected;

    MallocMutex::scoped_lock* lock_acquired;
    char scoped_lock_space[sizeof(MallocMutex::scoped_lock)+1];

    static uintptr_t absDiffPtr(void *x, void *y) {
        uintptr_t xi = (uintptr_t)x, yi = (uintptr_t)y;
        return xi > yi ? xi - yi : yi - xi;
    }
public:

    RecursiveMallocCallProtector() : lock_acquired(NULL) {
        lock_acquired = new (scoped_lock_space) MallocMutex::scoped_lock( rmc_mutex );
        if (canUsePthread)
            owner_thread = pthread_self();
        autoObjPtr = &scoped_lock_space;
    }
    ~RecursiveMallocCallProtector() {
        if (lock_acquired) {
            autoObjPtr = NULL;
            lock_acquired->~scoped_lock();
        }
    }
    static bool sameThreadActive() {
        if (!autoObjPtr) // fast path
            return false;
        // Some thread has an active recursive call protector; check if the current one.
        // Exact pthread_self based test
        if (canUsePthread)
            if (pthread_equal( owner_thread, pthread_self() )) {
                mallocRecursionDetected = true;
                return true;
            } else
                return false;
        // inexact stack size based test
        const uintptr_t threadStackSz = 2*1024*1024;
        int dummy;
        return absDiffPtr(autoObjPtr, &dummy)<threadStackSz;
    }
    static bool noRecursion() {
        MALLOC_ASSERT(isMallocInitialized(), 
                      "Recursion status can be checked only when initialization was done.");
        return !mallocRecursionDetected;
    }
/* The function is called on 1st scalable_malloc call to check if malloc calls
   scalable_malloc (nested call must set mallocRecursionDetected). */
    static void detectNaiveOverload() {
        if (!malloc_proxy) {
#if __FreeBSD__
/* If !canUsePthread, we can't call pthread_self() before, but now pthread 
   is already on, so can do it. False positives here lead to silent switching 
   from malloc to mmap for all large allocations with bad performance impact. */
            if (!canUsePthread) {
                canUsePthread = true;
                owner_thread = pthread_self();
            }
#endif
            free(malloc(1));
        }
    }
};


MallocMutex RecursiveMallocCallProtector::rmc_mutex;
pthread_t   RecursiveMallocCallProtector::owner_thread;
void       *RecursiveMallocCallProtector::autoObjPtr;
bool        RecursiveMallocCallProtector::mallocRecursionDetected;
#if __FreeBSD__
bool        RecursiveMallocCallProtector::canUsePthread;
#endif

#else

class RecursiveMallocCallProtector {
public:
    RecursiveMallocCallProtector() {}
    ~RecursiveMallocCallProtector() {}
};

#endif  /* MALLOC_CHECK_RECURSION */

/*
 * This number of bins in the TLS that leads to blocks that we can allocate in.
 */
const uint32_t numBlockBinLimit = 31;

 /*
  * The number of bins to cache large objects.
  */
const uint32_t numLargeBlockBins = 1024; // for 1024 max cached size is near 8MB
 
/********* The data structures and global objects        **************/

union BackRefIdx { // composite index to backreference array
    uint32_t t;
    struct {
        uint16_t master;   // index in BackRefMaster
        uint16_t offset;   // offset from beginning of BackRefBlock
    } s;
};

const BackRefIdx invalidIdx = {(uint32_t)-1};

inline bool operator== (const BackRefIdx &idx1, const BackRefIdx &idx2) {
    return idx1.t == idx2.t;
}

struct FreeObject {
    FreeObject  *next;
};

/*
 * The following constant is used to define the size of struct Block, the block header.
 * The intent is to have the size of a Block multiple of the cache line size, this allows us to
 * get good alignment at the cost of some overhead equal to the amount of padding included in the Block.
 */

const int blockHeaderAlignment = 64; // a common size of a cache line

struct Block;

/* The 'next' field in the block header has to maintain some invariants:
 *   it needs to be on a 16K boundary and the first field in the block.
 *   Any value stored there needs to have the lower 14 bits set to 0
 *   so that various assert work. This means that if you want to smash this memory
 *   for debugging purposes you will need to obey this invariant.
 * The total size of the header needs to be a power of 2 to simplify
 * the alignment requirements. For now it is a 128 byte structure.
 * To avoid false sharing, the fields changed only locally are separated 
 * from the fields changed by foreign threads.
 * Changing the size of the block header would require to change
 * some bin allocation sizes, in particular "fitting" sizes (see above).
 */

struct LocalBlockFields {
    Block       *next;            /* This field needs to be on a 16K boundary and the first field in the block
                                     for LIFO lists to work. */
    Block       *previous;        /* Use double linked list to speed up removal */
    unsigned int objectSize;
    unsigned int owner;
    FreeObject  *bumpPtr;         /* Bump pointer moves from the end to the beginning of a block */
    FreeObject  *freeList;
    BackRefIdx   backRefIdx;
    unsigned int allocatedCount;  /* Number of objects allocated (obviously by the owning thread) */
    unsigned int isFull;
};

struct Block : public LocalBlockFields {
    size_t       __pad_local_fields[(blockHeaderAlignment-sizeof(LocalBlockFields))/sizeof(size_t)];
    FreeObject  *publicFreeList;
    Block       *nextPrivatizable;
    size_t       __pad_public_fields[(blockHeaderAlignment-2*sizeof(void*))/sizeof(size_t)];
};

struct Bin {
    Block      *activeBlk;
    Block      *mailbox;
    MallocMutex mailLock;
};

/* 
 * To decrease contention for free blocks, free blocks are split, and access
 * to them is based on process number.
 */
const int numOfFreeBlockLists = 4;

/*
 * This is an array of LIFO linked lists that one can init, push or pop from
 */
static LifoList freeBlockList[numOfFreeBlockLists];

/*
 * When a block that is not completely free is returned for reuse by other threads
 * this is where the block goes.
 *
 * LifoList assumes zero initialization; so below its constructors are omitted,
 * to avoid linking with C++ libraries on Linux.
 */
static char globalBinSpace[sizeof(LifoList)*numBlockBinLimit];
static LifoList* globalSizeBins = (LifoList*)globalBinSpace;

/*
 * Per-thread pool of 16KB blocks. Idea behind it is to not share with other 
 * threads memory that are likely in local cache(s) of our CPU.
 */
class FreeBlockPool {
    Block *head;
    Block *tail;
    int    size;
    void insertBlock(Block *block);
public:
    static const int POOL_HIGH_MARK = 32;
    static const int POOL_LOW_MARK  = 8;

    Block *getBlock();
    void returnBlock(Block *block);
    void releaseAllBlocks();
};

struct TLSData {
    Bin           bin[numBlockBinLimit];
    FreeBlockPool pool;
};


static struct LargeBlockCacheStat {
    uintptr_t age;
    size_t cacheSize;
} loCacheStat;

struct LargeMemoryBlock {
    LargeMemoryBlock *next,          // ptrs in list of cached blocks
                     *prev;
    uintptr_t         age;           // age of block while in cache
    size_t            objectSize;    // the size requested by a client
    size_t            unalignedSize; // the size requested from getMemory
    bool              fromMapMemory;
    BackRefIdx        backRefIdx;    // cached here, used copy is in LargeObjectHdr
};

struct LargeObjectHdr {
    LargeMemoryBlock *memoryBlock;
    /* Backreference points to LargeObjectHdr. 
       Duplicated in LargeMemoryBlock to reuse in subsequent allocations. */
    BackRefIdx       backRefIdx;
};

class CachedBlocksList {
    LargeMemoryBlock *first,
                     *last;
    /* age of an oldest block in the list; equal to last->age, if last defined,
       used for quick cheching it without acquiring the lock. */
    uintptr_t     oldest;
    /* currAge when something was excluded out of list because of the age,
       not because of cache hit */
    uintptr_t     lastCleanedAge;
    /* Current threshold value for the blocks of a particular size. 
       Set on cache miss. */
    uintptr_t     ageThreshold;

    MallocMutex   lock;
    /* CachedBlocksList should be placed in zero-initialized memory,
       ctor not needed. */
    CachedBlocksList();
public:
    inline void push(LargeMemoryBlock* ptr);
    inline LargeMemoryBlock* pop();
    void releaseLastIfOld(uintptr_t currAge, size_t size);
};

/*
 * Array of bins with lists of recently freed large objects cached for re-use.
 */
static char globalCachedBlockBinsSpace[sizeof(CachedBlocksList)*numLargeBlockBins];
static CachedBlocksList* globalCachedBlockBins = (CachedBlocksList*)globalCachedBlockBinsSpace;

/********* End of the data structures                    **************/

/********** Various numeric parameters controlling allocations ********/

/*
 * blockSize - the size of a block, it must be larger than maxSegregatedObjectSize.
 *
 */
const uintptr_t blockSize = 16*1024;

/*
 * There are bins for all 8 byte aligned objects less than this segregated size; 8 bins in total
 */
const uint32_t minSmallObjectIndex = 0;
const uint32_t numSmallObjectBins = 8;
const uint32_t maxSmallObjectSize = 64;

/*
 * There are 4 bins between each couple of powers of 2 [64-128-256-...]
 * from maxSmallObjectSize till this size; 16 bins in total
 */
const uint32_t minSegregatedObjectIndex = minSmallObjectIndex+numSmallObjectBins;
const uint32_t numSegregatedObjectBins = 16;
const uint32_t maxSegregatedObjectSize = 1024;

/*
 * And there are 5 bins with the following allocation sizes: 1792, 2688, 3968, 5376, 8064.
 * They selected to fit 9, 6, 4, 3, and 2 sizes per a block, and also are multiples of 128.
 * If sizeof(Block) changes from 128, these sizes require close attention!
 */
const uint32_t minFittingIndex = minSegregatedObjectIndex+numSegregatedObjectBins;
const uint32_t numFittingBins = 5;

const uint32_t fittingAlignment = 128;

#define SET_FITTING_SIZE(N) ( (blockSize-sizeof(Block))/N ) & ~(fittingAlignment-1)
const uint32_t fittingSize1 = SET_FITTING_SIZE(9);
const uint32_t fittingSize2 = SET_FITTING_SIZE(6);
const uint32_t fittingSize3 = SET_FITTING_SIZE(4);
const uint32_t fittingSize4 = SET_FITTING_SIZE(3);
const uint32_t fittingSize5 = SET_FITTING_SIZE(2);
#undef SET_FITTING_SIZE

/*
 * The total number of thread-specific Block-based bins
 */
const uint32_t numBlockBins = minFittingIndex+numFittingBins;

/*
 * Objects of this size and larger are considered large objects.
 */
const uint32_t minLargeObjectSize = fittingSize5 + 1;

/*
 * Block::objectSize value used to mark blocks allocated by startupAlloc
 */
const unsigned int startupAllocObjSizeMark = ~(unsigned int)0;

/*
 * Difference between object sizes in large block bins
 */
const uint32_t largeBlockCacheStep = 8*1024;

/*
 * Large blocks cache cleanup frequency.
 * It should be power of 2 for the fast checking.
 */
const unsigned cacheCleanupFreq = 256;

/*
 * Get virtual memory in pieces of this size: 0x0100000 is 1 megabyte decimal
 */
static size_t mmapRequestSize = 0x0100000;

/********** End of numeric parameters controlling allocations *********/

#if !MALLOC_DEBUG
#if __INTEL_COMPILER || _MSC_VER
#define NOINLINE(decl) __declspec(noinline) decl
#define ALWAYSINLINE(decl) __forceinline decl
#elif __GNUC__
#define NOINLINE(decl) decl __attribute__ ((noinline))
#define ALWAYSINLINE(decl) decl __attribute__ ((always_inline))
#else
#define NOINLINE(decl) decl
#define ALWAYSINLINE(decl) decl
#endif

static NOINLINE( Block* getPublicFreeListBlock(Bin* bin) );
static NOINLINE( void moveBlockToBinFront(Block *block) );
static NOINLINE( void processLessUsedBlock(Block *block) );

static ALWAYSINLINE( Bin* getAllocationBin(size_t size) );
static ALWAYSINLINE( void checkInitialization() );

#undef ALWAYSINLINE
#undef NOINLINE
#endif /* !MALLOC_DEBUG */


/*********** Code to provide thread ID and a thread-local void pointer **********/

typedef intptr_t ThreadId;

static ThreadId ThreadIdCount;

static tls_key_t TLS_pointer_key;
static tls_key_t Tid_key;

static inline ThreadId  getThreadId(void)
{
    ThreadId result;
    result = reinterpret_cast<ThreadId>(TlsGetValue_func(Tid_key));
    if( !result ) {
        RecursiveMallocCallProtector scoped;
        // Thread-local value is zero -> first call from this thread,
        // need to initialize with next ID value (IDs start from 1)
        result = AtomicIncrement(ThreadIdCount); // returned new value!
        TlsSetValue_func( Tid_key, reinterpret_cast<void*>(result) );
    }
    return result;
}

static inline TLSData* getThreadMallocTLS() {
    TLSData *result;
    result = (TLSData *)TlsGetValue_func( TLS_pointer_key );
// The assert below is incorrect: with lazy initialization, it fails on the first call of the function.
//    MALLOC_ASSERT( result, "Memory allocator not initialized" );
    return result;
}

static inline void  setThreadMallocTLS( TLSData * newvalue ) {
    RecursiveMallocCallProtector scoped;
    TlsSetValue_func( TLS_pointer_key, newvalue );
}

/*********** End code to provide thread ID and a TLS pointer **********/

/*********** Code to acquire memory from the OS or other executive ****************/

#if USE_DEFAULT_MEMORY_MAPPING
#include "MapMemory.h"
#else
/* assume MapMemory and UnmapMemory are customized */
#endif

#if USE_MALLOC_FOR_LARGE_OBJECT

// (get|free)RawMemory only necessary for the USE_MALLOC_FOR_LARGE_OBJECT case
static inline void* getRawMemory (size_t size, bool useMapMem = false)
{
    void *object;

    if (useMapMem) 
        object = MapMemory(size);
    else
#if MALLOC_CHECK_RECURSION
    if (RecursiveMallocCallProtector::noRecursion())
        object = malloc(size);
    else if ( rml::internal::original_malloc_found )
        object = (*rml::internal::original_malloc_ptr)(size);
    else
        object = MapMemory(size);
#else
    object = malloc(size);
#endif /* MALLOC_CHECK_RECURSION */
    return object;
}

static inline void freeRawMemory (void *object, size_t size, bool useMapMem)
{
    if (useMapMem)
        UnmapMemory(object, size);
    else
#if MALLOC_CHECK_RECURSION
    if (RecursiveMallocCallProtector::noRecursion())
        free(object);
    else if ( rml::internal::original_malloc_found )
        (*rml::internal::original_free_ptr)(object);
    else
        UnmapMemory(object, size);
#else
    free(object);
#endif /* MALLOC_CHECK_RECURSION */
}

#else /* USE_MALLOC_FOR_LARGE_OBJECT */

static inline void* getRawMemory (size_t size, bool = false) { return MapMemory(size); }

static inline void freeRawMemory (void *object, size_t size, bool) {
    UnmapMemory(object, size);
}

#endif /* USE_MALLOC_FOR_LARGE_OBJECT */

/********* End memory acquisition code ********************************/

/********* Now some rough utility code to deal with indexing the size bins. **************/

/*
 * Given a number return the highest non-zero bit in it. It is intended to work with 32-bit values only.
 * Moreover, on IPF, for sake of simplicity and performance, it is narrowed to only serve for 64 to 1023.
 * This is enough for current algorithm of distribution of sizes among bins.
 */
#if _WIN64 && _MSC_VER>=1400 && !__INTEL_COMPILER
extern "C" unsigned char _BitScanReverse( unsigned long* i, unsigned long w );
#pragma intrinsic(_BitScanReverse)
#endif
static inline unsigned int highestBitPos(unsigned int n)
{
    unsigned int pos;
#if __ARCH_x86_32||__ARCH_x86_64

# if __linux__||__APPLE__||__FreeBSD__||__sun||__MINGW32__
    __asm__ ("bsr %1,%0" : "=r"(pos) : "r"(n));
# elif (_WIN32 && (!_WIN64 || __INTEL_COMPILER))
    __asm
    {
        bsr eax, n
        mov pos, eax
    }
# elif _WIN64 && _MSC_VER>=1400
    _BitScanReverse((unsigned long*)&pos, (unsigned long)n);
# else
#   error highestBitPos() not implemented for this platform
# endif

#elif __ARCH_ipf || __ARCH_other
    static unsigned int bsr[16] = {0,6,7,7,8,8,8,8,9,9,9,9,9,9,9,9};
    MALLOC_ASSERT( n>=64 && n<1024, ASSERT_TEXT );
    pos = bsr[ n>>6 ];
#else
#   error highestBitPos() not implemented for this platform
#endif /* __ARCH_* */
    return pos;
}

unsigned int getCPUid()
{
    unsigned int id;

#if (__ARCH_x86_32||__ARCH_x86_64) && (__linux__||__APPLE__||__FreeBSD__||__sun||__MINGW32__)
    int res;
 #if __ARCH_x86_32
    /* EBX used for PIC support. Having EAX in output operands 
       prevents ICC from crash like in __TBB_ICC_ASM_VOLATILE_BROKEN. */
    int _eax, _ecx, _edx;
    __asm__ ("xchgl %%ebx, %1\n\t"
             "cpuid\n\t"
             "xchgl %%ebx, %1\n\t"
             : "=a" (_eax), "=r" (res)
             : "a" (1) : "ecx", "edx");
 #else
    __asm__ ("cpuid\n\t"
             : "=b" (res)
             : "a" (1) );
 #endif // __ARCH_x86_32
    id = (res >> 24) & 0xff;
#elif _WIN32 || _WIN64
 #if __TBB_HAS_INTRIN_H
    int CPUInfo[4];
    __cpuid(CPUInfo, 1);
    id = (CPUInfo[1] >> 24) & 0xff;
 #else
    int res;
    _asm {
        push ebx
        push ecx
        mov  eax,1
        cpuid
        mov  res,ebx
        pop  ecx
        pop  ebx
    }
    id = (res >> 24) & 0xff;
 #endif
# else
    id = getThreadId();
#endif
    return id;
}

/*
 * Depending on indexRequest, for a given size return either the index into the bin
 * for objects of this size, or the actual size of objects in this bin.
 */
template<bool indexRequest>
static unsigned int getIndexOrObjectSize (unsigned int size)
{
    if (size <= maxSmallObjectSize) { // selection from 4/8/16/24/32/40/48/56/64
         /* Index 0 holds up to 8 bytes, Index 1 16 and so forth */
        return indexRequest ? (size - 1) >> 3 : alignUp(size,8);
    }
    else if (size <= maxSegregatedObjectSize ) { // 80/96/112/128 / 160/192/224/256 / 320/384/448/512 / 640/768/896/1024
        unsigned int order = highestBitPos(size-1); // which group of bin sizes?
        MALLOC_ASSERT( 6<=order && order<=9, ASSERT_TEXT );
        if (indexRequest)
            return minSegregatedObjectIndex - (4*6) - 4 + (4*order) + ((size-1)>>(order-2));
        else {
            unsigned int alignment = 128 >> (9-order); // alignment in the group
            MALLOC_ASSERT( alignment==16 || alignment==32 || alignment==64 || alignment==128, ASSERT_TEXT );
            return alignUp(size,alignment);
        }
    }
    else {
        if( size <= fittingSize3 ) {
            if( size <= fittingSize2 ) {
                if( size <= fittingSize1 )
                    return indexRequest ? minFittingIndex : fittingSize1; 
                else
                    return indexRequest ? minFittingIndex+1 : fittingSize2;
            } else
                return indexRequest ? minFittingIndex+2 : fittingSize3;
        } else {
            if( size <= fittingSize5 ) {
                if( size <= fittingSize4 )
                    return indexRequest ? minFittingIndex+3 : fittingSize4;
                else
                    return indexRequest ? minFittingIndex+4 : fittingSize5;
            } else {
                MALLOC_ASSERT( 0,ASSERT_TEXT ); // this should not happen
                return ~0U;
            }
        }
    }
}

static unsigned int getIndex (unsigned int size)
{
    return getIndexOrObjectSize</*indexRequest*/true>(size);
}

static unsigned int getObjectSize (unsigned int size)
{
    return getIndexOrObjectSize</*indexRequest*/false>(size);
}

/*
 * Initialization code.
 *
 */

/*
 * Big Blocks are the blocks we get from the OS or some similar place using getMemory above.
 * They are placed on the freeBlockList once they are acquired.
 */

static inline void *alignBigBlock(void *unalignedBigBlock)
{
    void *alignedBigBlock;
    /* align the entireHeap so all blocks are aligned. */
    alignedBigBlock = alignUp(unalignedBigBlock, blockSize);
    return alignedBigBlock;
}

/* Divide the big block into smaller bigBlocks that hold this many blocks.
 * This is done since we really need a lot of blocks on the freeBlockList or there will be
 * contention problems.
 */
const unsigned int blocksPerBigBlock = 16/numOfFreeBlockLists;

/* Returns 0 if unsuccessful, otherwise 1. */
static int mallocBigBlock()
{
    void *unalignedBigBlock;
    void *alignedBigBlock;
    void *bigBlockCeiling;
    Block *splitBlock;
    void *splitEdge;
    size_t bigBlockSplitSize;

    unalignedBigBlock = getRawMemory(mmapRequestSize, /*useMapMem=*/true);

    if (!unalignedBigBlock) {
        TRACEF(( "[ScalableMalloc trace] in mallocBigBlock, getMemory returns 0\n" ));
        /* We can't get any more memory from the OS or executive so return 0 */
        return 0;
    }

    alignedBigBlock = alignBigBlock(unalignedBigBlock);
    bigBlockCeiling = (void*)((uintptr_t)unalignedBigBlock + mmapRequestSize);

    bigBlockSplitSize = blocksPerBigBlock * blockSize;

    splitBlock = (Block*)alignedBigBlock;

    // distribute alignedBigBlock between all freeBlockList elements
    for (unsigned currListIdx = 0;
         ((uintptr_t)splitBlock + blockSize) <= (uintptr_t)bigBlockCeiling;
         currListIdx = (currListIdx+1) % numOfFreeBlockLists) {
        splitEdge = (void*)((uintptr_t)splitBlock + bigBlockSplitSize);
        if( splitEdge > bigBlockCeiling) {
            splitEdge = alignDown(bigBlockCeiling, blockSize);
        }
        splitBlock->bumpPtr = (FreeObject*)splitEdge;
        MALLOC_ITT_SYNC_RELEASING(freeBlockList+currListIdx);
        freeBlockList[currListIdx].push((void**) splitBlock);
        splitBlock = (Block*)splitEdge;
    }

    TRACEF(( "[ScalableMalloc trace] in mallocBigBlock returning 1\n" ));
    return 1;
}

/*
 * The malloc routines themselves need to be able to occasionally malloc some space,
 * in order to set up the structures used by the thread local structures. This
 * routine preforms that fuctions.
 */

/*
 * Forward Refs
 */
static Block *getEmptyBlock(size_t size);
static BackRefIdx newBackRef();
static void setBackRef(BackRefIdx backRefIdx, void *newPtr);
static void removeBackRef(BackRefIdx backRefIdx);

static MallocMutex bootStrapLock;

static Block      *bootStrapBlock = NULL;
static Block      *bootStrapBlockUsed = NULL;
static FreeObject *bootStrapObjectList = NULL; 

static void *bootStrapMalloc(size_t size)
{
    FreeObject *result;

    MALLOC_ASSERT( size == sizeof(TLSData), ASSERT_TEXT );

    { // Lock with acquire
        MallocMutex::scoped_lock scoped_cs(bootStrapLock);

        if( bootStrapObjectList) {
            result = bootStrapObjectList;
            bootStrapObjectList = bootStrapObjectList->next;
        } else {
            if (!bootStrapBlock) {
                bootStrapBlock = getEmptyBlock(size);
                if (!bootStrapBlock) return NULL;
            }
            result = bootStrapBlock->bumpPtr;
            bootStrapBlock->bumpPtr = (FreeObject *)((uintptr_t)bootStrapBlock->bumpPtr - bootStrapBlock->objectSize);
            if ((uintptr_t)bootStrapBlock->bumpPtr < (uintptr_t)bootStrapBlock+sizeof(Block)) {
                bootStrapBlock->bumpPtr = NULL;
                bootStrapBlock->next = bootStrapBlockUsed;
                bootStrapBlockUsed = bootStrapBlock;
                bootStrapBlock = NULL;
            }
        }
    } // Unlock with release

    memset (result, 0, size);
    return (void*)result;
}

static void bootStrapFree(void* ptr)
{
    MALLOC_ASSERT( ptr, ASSERT_TEXT );
    { // Lock with acquire
        MallocMutex::scoped_lock scoped_cs(bootStrapLock);
        ((FreeObject*)ptr)->next = bootStrapObjectList;
        bootStrapObjectList = (FreeObject*)ptr;
    } // Unlock with release
}

/********* End rough utility code  **************/

/********* Thread and block related code      *************/

#if MALLOC_DEBUG>1
/* The debug version verifies the TLSBin as needed */
static void verifyTLSBin (Bin* bin, size_t size)
{
    Block* temp;
    Bin*   tlsBin = getThreadMallocTLS()->bin;
    uint32_t index = getIndex(size);
    uint32_t objSize = getObjectSize(size);

    MALLOC_ASSERT( bin == tlsBin+index, ASSERT_TEXT );

    if (tlsBin[index].activeBlk) {
        MALLOC_ASSERT( tlsBin[index].activeBlk->owner == getThreadId(), ASSERT_TEXT );
        MALLOC_ASSERT( tlsBin[index].activeBlk->objectSize == objSize, ASSERT_TEXT );

        for (temp = tlsBin[index].activeBlk->next; temp; temp=temp->next) {
            MALLOC_ASSERT( temp!=tlsBin[index].activeBlk, ASSERT_TEXT );
            MALLOC_ASSERT( temp->owner == getThreadId(), ASSERT_TEXT );
            MALLOC_ASSERT( temp->objectSize == objSize, ASSERT_TEXT );
            MALLOC_ASSERT( temp->previous->next == temp, ASSERT_TEXT );
            if (temp->next) {
                MALLOC_ASSERT( temp->next->previous == temp, ASSERT_TEXT );
            }
        }
        for (temp = tlsBin[index].activeBlk->previous; temp; temp=temp->previous) {
            MALLOC_ASSERT( temp!=tls[index].activeBlk, ASSERT_TEXT );
            MALLOC_ASSERT( temp->owner == getThreadId(), ASSERT_TEXT );
            MALLOC_ASSERT( temp->objectSize == objSize, ASSERT_TEXT );
            MALLOC_ASSERT( temp->next->previous == temp, ASSERT_TEXT );
            if (temp->previous) {
                MALLOC_ASSERT( temp->previous->next == temp, ASSERT_TEXT );
            }
        }
    }
}
#else
inline static void verifyTLSBin (Bin*, size_t) {}
#endif /* MALLOC_DEBUG>1 */

/*
 * Add a block to the start of this tls bin list.
 */
static void pushTLSBin (Bin* bin, Block* block)
{
    /* The objectSize should be defined and not a parameter
       because the function is applied to partially filled blocks as well */
    unsigned int size = block->objectSize;
    Block* activeBlk;

    MALLOC_ASSERT( block->owner == getThreadId(), ASSERT_TEXT );
    MALLOC_ASSERT( block->objectSize != 0, ASSERT_TEXT );
    MALLOC_ASSERT( block->next == NULL, ASSERT_TEXT );
    MALLOC_ASSERT( block->previous == NULL, ASSERT_TEXT );

    MALLOC_ASSERT( bin, ASSERT_TEXT );
    verifyTLSBin(bin, size);
    activeBlk = bin->activeBlk;

    block->next = activeBlk;
    if( activeBlk ) {
        block->previous = activeBlk->previous;
        activeBlk->previous = block;
        if( block->previous )
            block->previous->next = block;
    } else {
        bin->activeBlk = block;
    }

    verifyTLSBin(bin, size);
}

/*
 * Take a block out of its tls bin (e.g. before removal).
 */
static void outofTLSBin (Bin* bin, Block* block)
{
    unsigned int size = block->objectSize;

    MALLOC_ASSERT( block->owner == getThreadId(), ASSERT_TEXT );
    MALLOC_ASSERT( block->objectSize != 0, ASSERT_TEXT );

    MALLOC_ASSERT( bin, ASSERT_TEXT );
    verifyTLSBin(bin, size);

    if (block == bin->activeBlk) {
        bin->activeBlk = block->previous? block->previous : block->next;
    }
    /* Delink the block */
    if (block->previous) {
        MALLOC_ASSERT( block->previous->next == block, ASSERT_TEXT );
        block->previous->next = block->next;
    }
    if (block->next) {
        MALLOC_ASSERT( block->next->previous == block, ASSERT_TEXT );
        block->next->previous = block->previous;
    }
    block->next = NULL;
    block->previous = NULL;

    verifyTLSBin(bin, size);
}

/*
 * Return the bin for the given size. If the TLS bin structure is absent, create it.
 */
static Bin* getAllocationBin(size_t size)
{
    TLSData* tls = getThreadMallocTLS();
    if( !tls ) {
        MALLOC_ASSERT( sizeof(TLSData) >= sizeof(Bin) * numBlockBins + sizeof(FreeBlockPool), ASSERT_TEXT );
        tls = (TLSData*) bootStrapMalloc(sizeof(TLSData));
        if ( !tls ) return NULL;
        /* the block contains zeroes after bootStrapMalloc, so bins are initialized */
#if MALLOC_DEBUG
        for (int i = 0; i < numBlockBinLimit; i++) {
            MALLOC_ASSERT( tls->bin[i].activeBlk == 0, ASSERT_TEXT );
            MALLOC_ASSERT( tls->bin[i].mailbox == 0, ASSERT_TEXT );
        }
#endif
        setThreadMallocTLS(tls);
    }
    MALLOC_ASSERT( tls, ASSERT_TEXT );
    return tls->bin + getIndex(size);
}

const float emptyEnoughRatio = 1.0 / 4.0; /* "Reactivate" a block if this share of its objects is free. */

static unsigned int emptyEnoughToUse (Block *mallocBlock)
{
    const float threshold = (blockSize - sizeof(Block)) * (1-emptyEnoughRatio);

    if (mallocBlock->bumpPtr) {
        /* If we are still using a bump ptr for this block it is empty enough to use. */
        STAT_increment(mallocBlock->owner, getIndex(mallocBlock->objectSize), examineEmptyEnough);
        mallocBlock->isFull = 0;
        return 1;
    }

    /* allocatedCount shows how many objects in the block are in use; however it still counts
       blocks freed by other threads; so prior call to privatizePublicFreeList() is recommended */
    mallocBlock->isFull = (mallocBlock->allocatedCount*mallocBlock->objectSize > threshold)? 1: 0;
#if COLLECT_STATISTICS
    if (mallocBlock->isFull)
        STAT_increment(mallocBlock->owner, getIndex(mallocBlock->objectSize), examineNotEmpty);
    else
        STAT_increment(mallocBlock->owner, getIndex(mallocBlock->objectSize), examineEmptyEnough);
#endif
    return 1-mallocBlock->isFull;
}

/* Restore the bump pointer for an empty block that is planned to use */
static void restoreBumpPtr (Block *block)
{
    MALLOC_ASSERT( block->allocatedCount == 0, ASSERT_TEXT );
    MALLOC_ASSERT( block->publicFreeList == NULL, ASSERT_TEXT );
    STAT_increment(block->owner, getIndex(block->objectSize), freeRestoreBumpPtr);
    block->bumpPtr = (FreeObject *)((uintptr_t)block + blockSize - block->objectSize);
    block->freeList = NULL;
    block->isFull = 0;
}

#if !(FREELIST_NONBLOCKING)
static MallocMutex publicFreeListLock; // lock for changes of publicFreeList
#endif

const uintptr_t UNUSABLE = 0x1;
inline bool isSolidPtr( void* ptr )
{
    return (UNUSABLE|(uintptr_t)ptr)!=UNUSABLE;
}
inline bool isNotForUse( void* ptr )
{
    return (uintptr_t)ptr==UNUSABLE;
}

static void freePublicObject (Block *block, FreeObject *objectToFree)
{
    Bin* theBin;
    FreeObject *publicFreeList;

    MALLOC_ITT_SYNC_RELEASING(&block->publicFreeList);
#if FREELIST_NONBLOCKING
    FreeObject *temp = block->publicFreeList;
    do {
        publicFreeList = objectToFree->next = temp;
        temp = (FreeObject*)AtomicCompareExchange(
                                (intptr_t&)block->publicFreeList,
                                (intptr_t)objectToFree, (intptr_t)publicFreeList );
        // no backoff necessary because trying to make change, not waiting for a change
    } while( temp != publicFreeList );
#else
    STAT_increment(getThreadId(), ThreadCommonCounters, lockPublicFreeList);
    {
        MallocMutex::scoped_lock scoped_cs(publicFreeListLock);
        publicFreeList = objectToFree->next = block->publicFreeList;
        block->publicFreeList = objectToFree;
    }
#endif

    if( publicFreeList==NULL ) {
        // if the block is abandoned, its nextPrivatizable pointer should be UNUSABLE
        // otherwise, it should point to the bin the block belongs to.
        // reading nextPrivatizable is thread-safe below, because:
        // 1) the executing thread atomically got publicFreeList==NULL and changed it to non-NULL;
        // 2) only owning thread can change it back to NULL,
        // 3) but it can not be done until the block is put to the mailbox
        // So the executing thread is now the only one that can change nextPrivatizable
        if( !isNotForUse(block->nextPrivatizable) ) {
            MALLOC_ASSERT( block->nextPrivatizable!=NULL, ASSERT_TEXT );
            MALLOC_ASSERT( block->owner!=0, ASSERT_TEXT );
            theBin = (Bin*) block->nextPrivatizable;
            MallocMutex::scoped_lock scoped_cs(theBin->mailLock);
            block->nextPrivatizable = theBin->mailbox;
            theBin->mailbox = block;
        } else {
            MALLOC_ASSERT( block->owner==0, ASSERT_TEXT );
        }
    }
    STAT_increment(getThreadId(), ThreadCommonCounters, freeToOtherThread);
    STAT_increment(block->owner, getIndex(block->objectSize), freeByOtherThread);
}

static void privatizePublicFreeList (Block *mallocBlock)
{
    FreeObject *temp, *publicFreeList;

    MALLOC_ASSERT( mallocBlock->owner == getThreadId(), ASSERT_TEXT );
#if FREELIST_NONBLOCKING
    temp = mallocBlock->publicFreeList;
    do {
        publicFreeList = temp;
        temp = (FreeObject*)AtomicCompareExchange(
                                (intptr_t&)mallocBlock->publicFreeList,
                                0, (intptr_t)publicFreeList);
        // no backoff necessary because trying to make change, not waiting for a change
    } while( temp != publicFreeList );
#else
    STAT_increment(mallocBlock->owner, ThreadCommonCounters, lockPublicFreeList);
    {
        MallocMutex::scoped_lock scoped_cs(publicFreeListLock);
        publicFreeList = mallocBlock->publicFreeList;
        mallocBlock->publicFreeList = NULL;
    }
    temp = publicFreeList;
#endif
    MALLOC_ITT_SYNC_ACQUIRED(&mallocBlock->publicFreeList);

    MALLOC_ASSERT( publicFreeList && publicFreeList==temp, ASSERT_TEXT ); // there should be something in publicFreeList!
    if( !isNotForUse(temp) ) { // return/getPartialBlock could set it to UNUSABLE
        MALLOC_ASSERT( mallocBlock->allocatedCount <= (blockSize-sizeof(Block))/mallocBlock->objectSize, ASSERT_TEXT );
        /* other threads did not change the counter freeing our blocks */
        mallocBlock->allocatedCount--;
        while( isSolidPtr(temp->next) ){ // the list will end with either NULL or UNUSABLE
            temp = temp->next;
            mallocBlock->allocatedCount--;
        }
        MALLOC_ASSERT( mallocBlock->allocatedCount < (blockSize-sizeof(Block))/mallocBlock->objectSize, ASSERT_TEXT );
        /* merge with local freeList */
        temp->next = mallocBlock->freeList;
        mallocBlock->freeList = publicFreeList;
        STAT_increment(mallocBlock->owner, getIndex(mallocBlock->objectSize), allocPrivatized);
    }
}

static Block* getPublicFreeListBlock (Bin* bin)
{
    Block* block;
    MALLOC_ASSERT( bin, ASSERT_TEXT );
// the counter should be changed    STAT_increment(getThreadId(), ThreadCommonCounters, lockPublicFreeList);
    {
        MallocMutex::scoped_lock scoped_cs(bin->mailLock);
        block = bin->mailbox;
        if( block ) {
            MALLOC_ASSERT( block->owner == getThreadId(), ASSERT_TEXT );
            MALLOC_ASSERT( !isNotForUse(block->nextPrivatizable), ASSERT_TEXT );
            bin->mailbox = block->nextPrivatizable;
            block->nextPrivatizable = (Block*) bin;
        }
    }
    if( block ) {
        MALLOC_ASSERT( isSolidPtr(block->publicFreeList), ASSERT_TEXT );
        privatizePublicFreeList(block);
    }
    return block;
}

static Block *getPartialBlock(Bin* bin, unsigned int size)
{
    Block *result;
    MALLOC_ASSERT( bin, ASSERT_TEXT );
    unsigned int index = getIndex(size);
    result = (Block *) globalSizeBins[index].pop();
    if (result) {
        MALLOC_ITT_SYNC_ACQUIRED(globalSizeBins+index);
        result->next = NULL;
        result->previous = NULL;
        MALLOC_ASSERT( result->publicFreeList!=NULL, ASSERT_TEXT );
        /* There is not a race here since no other thread owns this block */
        MALLOC_ASSERT( result->owner == 0, ASSERT_TEXT );
        result->owner = getThreadId();
        // It is safe to change nextPrivatizable, as publicFreeList is not null
        MALLOC_ASSERT( isNotForUse(result->nextPrivatizable), ASSERT_TEXT );
        result->nextPrivatizable = (Block*)bin;
        // the next call is required to change publicFreeList to 0
        privatizePublicFreeList(result);
        if( result->allocatedCount ) {
            // check its fullness and set result->isFull
            emptyEnoughToUse(result);
        } else {
            restoreBumpPtr(result);
        }
        MALLOC_ASSERT( !isNotForUse(result->publicFreeList), ASSERT_TEXT );
        STAT_increment(result->owner, index, allocBlockPublic);
    }
    return result;
}

static void returnPartialBlock(Bin* bin, Block *block)
{
    unsigned int index = getIndex(block->objectSize);
    MALLOC_ASSERT( bin, ASSERT_TEXT );
    MALLOC_ASSERT( block->owner==getThreadId(), ASSERT_TEXT );
    STAT_increment(block->owner, index, freeBlockPublic);
    // need to set publicFreeList to non-zero, so other threads
    // will not change nextPrivatizable and it can be zeroed.
    if ((intptr_t)block->nextPrivatizable==(intptr_t)bin) {
        void* oldval;
#if FREELIST_NONBLOCKING
        oldval = (void*)AtomicCompareExchange((intptr_t&)block->publicFreeList, (intptr_t)UNUSABLE, 0);
#else
        STAT_increment(block->owner, ThreadCommonCounters, lockPublicFreeList);
        {
            MallocMutex::scoped_lock scoped_cs(publicFreeListLock);
            if ( (oldval=block->publicFreeList)==NULL )
                (uintptr_t&)(block->publicFreeList) = UNUSABLE;
        }
#endif
        if ( oldval!=NULL ) {
            // another thread freed an object; we need to wait until it finishes.
            // I believe there is no need for exponential backoff, as the wait here is not for a lock;
            // but need to yield, so the thread we wait has a chance to run.
            int count = 256;
            while( (intptr_t)const_cast<Block* volatile &>(block->nextPrivatizable)==(intptr_t)bin ) {
                if (--count==0) {
                    do_yield();
                    count = 256;
                }
            }
        }
    } else {
        MALLOC_ASSERT( isSolidPtr(block->publicFreeList), ASSERT_TEXT );
    }
    MALLOC_ASSERT( block->publicFreeList!=NULL, ASSERT_TEXT );
    // now it is safe to change our data
    block->previous = NULL;
    block->owner = 0;
    // it is caller responsibility to ensure that the list of blocks
    // formed by nextPrivatizable pointers is kept consistent if required.
    // if only called from thread shutdown code, it does not matter.
    (uintptr_t&)(block->nextPrivatizable) = UNUSABLE;
    MALLOC_ITT_SYNC_RELEASING(globalSizeBins+index);
    globalSizeBins[index].push((void **)block);
}

static void cleanBlockHeader(Block *block)
{
    block->next = NULL;
    block->previous = NULL;
    block->freeList = NULL;
    block->allocatedCount = 0;
    block->isFull = 0;

    block->publicFreeList = NULL;
}

static void initEmptyBlock(Block *block, size_t size)
{
    // Having getIndex and getObjectSize called next to each other
    // allows better compiler optimization as they basically share the code.
    unsigned int index      = getIndex(size);
    unsigned int objectSize = getObjectSize(size); 
    Bin* tlsBin = getThreadMallocTLS()->bin;

    cleanBlockHeader(block);
    block->objectSize = objectSize;
    block->owner = getThreadId();
    // bump pointer should be prepared for first allocation - thus mode it down to objectSize
    block->bumpPtr = (FreeObject *)((uintptr_t)block + blockSize - objectSize);

    // each block should have the address where the head of the list of "privatizable" blocks is kept
    // the only exception is a block for boot strap which is initialized when TLS is yet NULL
    block->nextPrivatizable = tlsBin? (Block*)(tlsBin + index) : NULL;
    TRACEF(( "[ScalableMalloc trace] Empty block %p is initialized, owner is %d, objectSize is %d, bumpPtr is %p\n",
             block, block->owner, block->objectSize, block->bumpPtr ));
}

void FreeBlockPool::insertBlock(Block *block)
{
    size++;
    block->next = head;
    head = block;
    if (!tail)
        tail = block;
}

Block *FreeBlockPool::getBlock()
{
    Block *result = head;
    if (head) {
        size--;
        head = head->next;
        if (!head)
            tail = NULL;
    }
    return result;
}

void FreeBlockPool::returnBlock(Block *block)
{
    MALLOC_ASSERT( size <= POOL_HIGH_MARK, ASSERT_TEXT );
    if (size == POOL_HIGH_MARK) {
        // release cold blocks and add hot one
        Block *headToFree = head, 
              *tailToFree = tail;
        for (int i=0; i<POOL_LOW_MARK-2; i++)
            headToFree = headToFree->next;
        tail = headToFree;
        headToFree = headToFree->next;
        tail->next = NULL;
        size = POOL_LOW_MARK-1;
        for (Block *currBl = headToFree; currBl; currBl = currBl->next)
            removeBackRef(currBl->backRefIdx);
        unsigned myFreeList = getCPUid()%numOfFreeBlockLists;
        MALLOC_ITT_SYNC_RELEASING(freeBlockList+myFreeList);
        freeBlockList[myFreeList].pushList((void **)headToFree, (void **)tailToFree);
    }
    insertBlock(block);
}

void FreeBlockPool::releaseAllBlocks()
{
    if (head) {
        for (Block *currBl = head; currBl; currBl = currBl->next)
            removeBackRef(currBl->backRefIdx);
        unsigned myFreeList = getCPUid()%numOfFreeBlockLists;
        MALLOC_ITT_SYNC_RELEASING(freeBlockList+myFreeList);
        freeBlockList[myFreeList].pushList((void**)head, (void**)tail);
    }
}

/* Return an empty uninitialized block in a non-blocking fashion. */
static Block *getRawBlock(bool startup)
{
    Block *result;
    Block *bigBlock;
    const unsigned myFreeList = startup? 0 : getCPUid()%numOfFreeBlockLists;
    unsigned currListIdx = myFreeList;
    
    result = NULL;

    do {
        if (bigBlock = (Block *) freeBlockList[currListIdx].pop()) {
            MALLOC_ITT_SYNC_ACQUIRED(freeBlockList+currListIdx);
            break;
        }
        currListIdx = (currListIdx+1) % numOfFreeBlockLists;
    } while (currListIdx != myFreeList);

    while (!bigBlock) {
        /* We are out of blocks so go to the OS and get another one */
        if (!mallocBigBlock()) {
            return NULL;
        }
        bigBlock = (Block *) freeBlockList[myFreeList].pop();
        if (bigBlock)
            MALLOC_ITT_SYNC_ACQUIRED(freeBlockList+myFreeList);
    }

    // check alignment
    MALLOC_ASSERT( isAligned( bigBlock, blockSize ), ASSERT_TEXT );
    MALLOC_ASSERT( isAligned( bigBlock->bumpPtr, blockSize ), ASSERT_TEXT );
    // block should be at least as big as blockSize; otherwise the previous block can be damaged.
    MALLOC_ASSERT( (uintptr_t)bigBlock->bumpPtr >= (uintptr_t)bigBlock + blockSize, ASSERT_TEXT );
    bigBlock->bumpPtr = (FreeObject *)((uintptr_t)bigBlock->bumpPtr - blockSize);
    result = (Block *)bigBlock->bumpPtr;
    if ( result!=bigBlock ) {
        TRACEF(( "[ScalableMalloc trace] Pushing partial rest of block back on.\n" ));
        MALLOC_ITT_SYNC_RELEASING(freeBlockList+myFreeList);
        freeBlockList[myFreeList].push((void **)bigBlock);
    }

    return result;
}

/* Return an empty uninitialized block in a non-blocking fashion. */
static Block *getEmptyBlock(size_t size)
{
    Block *result = NULL;
    TLSData* tls = getThreadMallocTLS();
    if (tls)
        result = tls->pool.getBlock();
    if (!result) {
        BackRefIdx backRefIdx = newBackRef();
        if (backRefIdx == invalidIdx || !(result = getRawBlock(/*startup=*/false)))
            return NULL;
        setBackRef(backRefIdx, result);
        result->backRefIdx = backRefIdx;
    }
    if (result) {
        initEmptyBlock(result, size);
        STAT_increment(result->owner, getIndex(result->objectSize), allocBlockNew);
    }
    return result;
}

/* We have a block give it back to the malloc block manager */
static void returnEmptyBlock (Block *block, bool poolTheBlock)
{
    // it is caller's responsibility to ensure no data is lost before calling this
    MALLOC_ASSERT( block->allocatedCount==0, ASSERT_TEXT );
    MALLOC_ASSERT( block->publicFreeList==NULL, ASSERT_TEXT );
    MALLOC_ASSERT( !poolTheBlock || block->next == NULL, ASSERT_TEXT );
    MALLOC_ASSERT( !poolTheBlock || block->previous == NULL, ASSERT_TEXT );
    STAT_increment(block->owner, getIndex(block->objectSize), freeBlockBack);

    cleanBlockHeader(block);

    block->nextPrivatizable = NULL;

    block->objectSize = 0;
    block->owner = (unsigned)-1;
    // for an empty block, bump pointer should point right after the end of the block
    block->bumpPtr = (FreeObject *)((uintptr_t)block + blockSize);
    if (poolTheBlock) {
        MALLOC_ASSERT(getThreadMallocTLS(), "Is TLS still not initialized?");
        getThreadMallocTLS()->pool.returnBlock(block);
    }
    else {
        removeBackRef(block->backRefIdx);
        unsigned myFreeList = getCPUid()%numOfFreeBlockLists;
        MALLOC_ITT_SYNC_RELEASING(freeBlockList+myFreeList);
        freeBlockList[myFreeList].push((void **)block);
    }
}

inline static Block* getActiveBlock( Bin* bin )
{
    MALLOC_ASSERT( bin, ASSERT_TEXT );
    return bin->activeBlk;
}

inline static void setActiveBlock (Bin* bin, Block *block)
{
    MALLOC_ASSERT( bin, ASSERT_TEXT );
    MALLOC_ASSERT( block->owner == getThreadId(), ASSERT_TEXT );
    // it is the caller responsibility to keep bin consistence (i.e. ensure this block is in the bin list)
    bin->activeBlk = block;
}

inline static Block* setPreviousBlockActive( Bin* bin )
{
    MALLOC_ASSERT( bin && bin->activeBlk, ASSERT_TEXT );
    Block* temp = bin->activeBlk->previous;
    if( temp ) {
        MALLOC_ASSERT( temp->isFull == 0, ASSERT_TEXT );
        bin->activeBlk = temp;
    }
    return temp;
}

#if MALLOC_CHECK_RECURSION

/*
 * It's a special kind of allocation that can be used when malloc is 
 * not available (either during startup or when malloc was already called and
 * we are, say, inside pthread_setspecific's call). 
 * Block can contain objects of different sizes, 
 * allocations are performed by moving bump pointer and increasing of object counter, 
 * releasing is done via counter of objects allocated in the block 
 * or moving bump pointer if releasing object is on a bound.
 */

struct StartupBlock : public Block {
    size_t availableSize() {
        return blockSize - ((uintptr_t)bumpPtr - (uintptr_t)this);
    }
};

static MallocMutex startupMallocLock;
static StartupBlock *firstStartupBlock;

static StartupBlock *getNewStartupBlock()
{
    BackRefIdx backRefIdx = newBackRef();
    if (backRefIdx == invalidIdx) return NULL;

    StartupBlock *block = (StartupBlock *)getRawBlock(/*startup=*/true);
    if (!block) return NULL;

    cleanBlockHeader(block);
    setBackRef(backRefIdx, block);
    block->backRefIdx = backRefIdx;
    // use startupAllocObjSizeMark to mark objects from startup block marker
    block->objectSize = startupAllocObjSizeMark;
    block->bumpPtr = (FreeObject *)((uintptr_t)block + sizeof(StartupBlock));
    return block;
}

/* TODO: Function is called when malloc nested call is detected, so simultaneous
   usage from different threads are unprobable, so block pre-allocation 
   can be not useful, and the code might be simplified. */
static FreeObject *startupAlloc(size_t size)
{
    FreeObject *result;
    StartupBlock *newBlock = NULL;
    bool newBlockUnused = false;

    /* Objects must be aligned on their natural bounds, 
       and objects bigger than word on word's bound. */
    size = alignUp(size, sizeof(size_t));
    // We need size of an object to implement msize.
    size_t reqSize = size + sizeof(size_t);
    // speculatively allocates newBlock to later use or return it as unused
    if (!firstStartupBlock || firstStartupBlock->availableSize() < reqSize)
        if (!(newBlock = getNewStartupBlock()))
            return NULL;

    {
        MallocMutex::scoped_lock scoped_cs(startupMallocLock);
    
        if (!firstStartupBlock || firstStartupBlock->availableSize() < reqSize) {
            if (!newBlock && !(newBlock = getNewStartupBlock()))
                return NULL;
            newBlock->next = (Block*)firstStartupBlock;
            if (firstStartupBlock)
                firstStartupBlock->previous = (Block*)newBlock;
            firstStartupBlock = newBlock;
        } else
            newBlockUnused = true;
        result = firstStartupBlock->bumpPtr;
        firstStartupBlock->allocatedCount++;
        firstStartupBlock->bumpPtr = 
            (FreeObject *)((uintptr_t)firstStartupBlock->bumpPtr + reqSize);
    }
    if (newBlock && newBlockUnused)
        returnEmptyBlock(newBlock, /*poolTheBlock=*/false);

    // keep object size at the negative offset
    *((size_t*)result) = size;
    return (FreeObject*)((size_t*)result+1);
}

static size_t startupMsize(void *ptr) { return *((size_t*)ptr - 1); }

static void startupFree(StartupBlock *block, void *ptr)
{
    Block* blockToRelease = NULL;
    {
        MallocMutex::scoped_lock scoped_cs(startupMallocLock);
    
        MALLOC_ASSERT(firstStartupBlock, ASSERT_TEXT);
        MALLOC_ASSERT(startupAllocObjSizeMark==block->objectSize 
                      && block->allocatedCount>0, ASSERT_TEXT);
        MALLOC_ASSERT((uintptr_t)ptr>=(uintptr_t)block+sizeof(StartupBlock)
                      && (uintptr_t)ptr+startupMsize(ptr)<=(uintptr_t)block+blockSize, 
                      ASSERT_TEXT);
        if (0 == --block->allocatedCount) {
            if (block == firstStartupBlock)
                firstStartupBlock = (StartupBlock*)firstStartupBlock->next;
            if (block->previous)
                block->previous->next = block->next;
            if (block->next)
                block->next->previous = block->previous;
            blockToRelease = block;
        } else if ((uintptr_t)ptr + startupMsize(ptr) == (uintptr_t)block->bumpPtr) {
            // last object in the block released
            FreeObject *newBump = (FreeObject*)((size_t*)ptr - 1);
            MALLOC_ASSERT((uintptr_t)newBump>(uintptr_t)block+sizeof(StartupBlock), 
                          ASSERT_TEXT);
            block->bumpPtr = newBump;
        }
    }
    if (blockToRelease) {
        blockToRelease->previous = blockToRelease->next = NULL;
        returnEmptyBlock(blockToRelease, /*poolTheBlock=*/false);
    }
}

#endif /* MALLOC_CHECK_RECURSION */

/********* End thread related code  *************/

/********* backreferences ***********************/
/* Each 16KB block and each large memory object header contains BackRefIdx 
 * that points out in some BackRefBlock which points back to this block or header.
 */
struct BackRefBlock {
    BackRefBlock *nextForUse;     // the next in the chain of blocks with free items
    FreeObject   *bumpPtr;        // bump pointer moves from the end to the beginning of the block
    FreeObject   *freeList;
    int           allocatedCount; // the number of objects allocated
    int           myNum;          // the index in the parent array
    MallocMutex   blockMutex;
    bool          addedToForUse;  // this block is already added to the listForUse chain

    BackRefBlock(BackRefBlock *blockToUse, int myNum) :
        nextForUse(NULL), bumpPtr((FreeObject*)((uintptr_t)blockToUse + blockSize - sizeof(void*))),
        freeList(NULL), allocatedCount(0), myNum(myNum), addedToForUse(false) {}
};

// max number of backreference pointers in 16KB block
static const int BR_MAX_CNT = (blockSize-sizeof(BackRefBlock))/sizeof(void*);

struct BackRefMaster {
/* A 16KB block can hold up to ~2K back pointers to 16KB blocks or large objects,
 * so it can address at least 32MB. The array of 64KB holds 8K pointers
 * to such blocks, addressing ~256 GB.
 */
    static const size_t bytes = 64*1024;
    static const int dataSz;

    BackRefBlock  *active;         // if defined, use it for allocations
    BackRefBlock  *listForUse;     // the chain of data blocks with free items
    int            lastUsed;       // index of the last used block
    BackRefBlock  *backRefBl[1];   // the real size of the array is dataSz

    BackRefBlock *findFreeBlock();
    bool          addActiveBackRefBlock();
};

const int BackRefMaster::dataSz
    = 1+(BackRefMaster::bytes-sizeof(BackRefMaster))/sizeof(BackRefBlock*);

static MallocMutex backRefMutex;
static BackRefMaster *backRefMaster;

static bool initBackRefMaster()
{
    // MapMemory forced because the function runs during startup 
    backRefMaster = (BackRefMaster*)getRawMemory(BackRefMaster::bytes, 
                                                 /*useMapMem=*/true);
    if (NULL == backRefMaster)
        return false;
    backRefMaster->listForUse = NULL;
    backRefMaster->lastUsed = 0;
    
    if (!backRefMaster->addActiveBackRefBlock()) {
        freeRawMemory(backRefMaster, BackRefMaster::bytes, /*useMapMem=*/true);
        return false;
    }
    return true;
}

bool BackRefMaster::addActiveBackRefBlock()
{
    BackRefBlock *newBl = (BackRefBlock*)getRawBlock(/*startup=*/true);

    if (!newBl) return false;
    active = newBl;
    memset(active, 0, blockSize);
    new (active) BackRefBlock(active, lastUsed);
    backRefBl[lastUsed] = active;
    return true;
}

BackRefBlock *BackRefMaster::findFreeBlock()
{
    if (active->allocatedCount < BR_MAX_CNT)
        return active;
        
    if (listForUse) {                                   // use released list
        active = listForUse;
        listForUse = listForUse->nextForUse;
        MALLOC_ASSERT(active->addedToForUse, ASSERT_TEXT);
        active->addedToForUse = false;
    } else if (lastUsed-1 < backRefMaster->dataSz) {    // allocate new data node
        lastUsed++;
        if (!backRefMaster->addActiveBackRefBlock()) {
            lastUsed--;
            return NULL;
        }
    } else  // no free blocks, give up
        return NULL;
    return active;
}

static inline void *getBackRef(BackRefIdx backRefIdx)
{
    // !backRefMaster means no initialization done, so it can't be valid memory
    if (!backRefMaster || backRefIdx.s.master > backRefMaster->lastUsed
        || backRefIdx.s.offset >= BR_MAX_CNT) 
        return NULL;
    return *(void**)((uintptr_t)backRefMaster->backRefBl[backRefIdx.s.master]
                     + sizeof(BackRefBlock)+backRefIdx.s.offset*sizeof(void*));
}

static void setBackRef(BackRefIdx backRefIdx, void *newPtr)
{
    MALLOC_ASSERT(backRefIdx.s.master<=backRefMaster->lastUsed && backRefIdx.s.offset<BR_MAX_CNT,
                  ASSERT_TEXT);
    *(void**)((uintptr_t)backRefMaster->backRefBl[backRefIdx.s.master]
              + sizeof(BackRefBlock) + backRefIdx.s.offset*sizeof(void*)) = newPtr;
}

static BackRefIdx newBackRef()
{
    BackRefBlock *blockToUse;
    void **toUse;
    BackRefIdx res;

    do {
        { // global lock taken to find a block
            MallocMutex::scoped_lock lock(backRefMutex);

            MALLOC_ASSERT(backRefMaster, ASSERT_TEXT);
            if (! (blockToUse = backRefMaster->findFreeBlock()))
                return invalidIdx;
        }
        toUse = NULL;
        { // the block is locked to find a reference
            MallocMutex::scoped_lock lock(blockToUse->blockMutex);

            if (blockToUse->freeList) {
                toUse = (void**)blockToUse->freeList;
                blockToUse->freeList = blockToUse->freeList->next;
            } else if (blockToUse->allocatedCount < BR_MAX_CNT) {
                toUse = (void**)blockToUse->bumpPtr;
                blockToUse->bumpPtr = 
                    (FreeObject*)((uintptr_t)blockToUse->bumpPtr - sizeof(void*));
                if (blockToUse->allocatedCount == BR_MAX_CNT-1) {
                    MALLOC_ASSERT((uintptr_t)blockToUse->bumpPtr
                                  < (uintptr_t)blockToUse+sizeof(BackRefBlock),
                                  ASSERT_TEXT);
                    blockToUse->bumpPtr = NULL;
                }
            }
            if (toUse)
                blockToUse->allocatedCount++;
        } // end of lock scope
    } while (!toUse);
    res.s.master = blockToUse->myNum;
    res.s.offset = 
        ((uintptr_t)toUse - ((uintptr_t)blockToUse + sizeof(BackRefBlock)))/sizeof(void*);
    return res;
}

static void removeBackRef(BackRefIdx backRefIdx)
{
    MALLOC_ASSERT(backRefIdx.s.master<=backRefMaster->lastUsed 
                  && backRefIdx.s.offset<BR_MAX_CNT, ASSERT_TEXT);
    BackRefBlock *currBlock = backRefMaster->backRefBl[backRefIdx.s.master];
    FreeObject *freeObj = (FreeObject*)((uintptr_t)currBlock + sizeof(BackRefBlock)
                                        + backRefIdx.s.offset*sizeof(void*));
    {
        MallocMutex::scoped_lock lock(currBlock->blockMutex);

        freeObj->next = currBlock->freeList;
        currBlock->freeList = freeObj;
        currBlock->allocatedCount--;
    }
    // TODO: do we need double-check here?
    if (!currBlock->addedToForUse && currBlock!=backRefMaster->active) {
        MallocMutex::scoped_lock lock(backRefMutex);

        if (!currBlock->addedToForUse && currBlock!=backRefMaster->active) {
            currBlock->nextForUse = backRefMaster->listForUse;
            backRefMaster->listForUse = currBlock;
            currBlock->addedToForUse = true;
        }
    }
}

/********* End of backreferences ***********************/

/********* Library initialization *************/

//! Value indicating the state of initialization.
/* 0 = initialization not started.
 * 1 = initialization started but not finished.
 * 2 = initialization finished.
 * In theory, we only need values 0 and 2. But value 1 is nonetheless
 * useful for detecting errors in the double-check pattern.
 */
static int mallocInitialized;   // implicitly initialized to 0
static MallocMutex initAndShutMutex;

inline bool isMallocInitialized() { return 2 == mallocInitialized; }

/*
 * Allocator initialization routine;
 * it is called lazily on the very first scalable_malloc call.
 */
static void initMemoryManager()
{
    TRACEF(( "[ScalableMalloc trace] sizeof(Block) is %d (expected 128); sizeof(uintptr_t) is %d\n",
             sizeof(Block), sizeof(uintptr_t) ));
    MALLOC_ASSERT( 2*blockHeaderAlignment == sizeof(Block), ASSERT_TEXT );
    MALLOC_ASSERT( sizeof(FreeObject) == sizeof(void*), ASSERT_TEXT );

// TODO: add error handling, and on error do something better than exit(1)
    if (!mallocBigBlock() || !initBackRefMaster()) {
        fprintf (stderr, "The memory manager cannot access sufficient memory to initialize; exiting \n");
        exit(1);
    }
// Create keys for thread-local storage and for thread id
#if USE_WINTHREAD
    TLS_pointer_key = TlsAlloc();
    Tid_key = TlsAlloc();
#else
    int status1 = pthread_key_create( &TLS_pointer_key, mallocThreadShutdownNotification );
    int status2 = pthread_key_create( &Tid_key, NULL );
    if ( status1 || status2 ) {
        fprintf (stderr, "The memory manager cannot create tls key during initialization; exiting \n");
        exit(1);
    }
#endif /* USE_WINTHREAD */
#if COLLECT_STATISTICS
    initStatisticsCollection();
#endif
}

//! Ensures that initMemoryManager() is called once and only once.
/** Does not return until initMemoryManager() has been completed by a thread.
    There is no need to call this routine if mallocInitialized==2 . */
static void checkInitialization()
{
    if (mallocInitialized==2) return;
    MallocMutex::scoped_lock lock( initAndShutMutex );
    if (mallocInitialized!=2) {
        MALLOC_ASSERT( mallocInitialized==0, ASSERT_TEXT );
        mallocInitialized = 1;
        RecursiveMallocCallProtector scoped;
        initMemoryManager();
#ifdef  MALLOC_EXTRA_INITIALIZATION
        MALLOC_EXTRA_INITIALIZATION;
#endif
#if MALLOC_CHECK_RECURSION
        RecursiveMallocCallProtector::detectNaiveOverload();
#endif
        MALLOC_ASSERT( mallocInitialized==1, ASSERT_TEXT );
        mallocInitialized = 2;
    }
    MALLOC_ASSERT( mallocInitialized==2, ASSERT_TEXT ); /* It can't be 0 or I would have initialized it */
}

/********* End library initialization *************/

/********* The malloc show begins     *************/


/********* Allocation of large objects ************/

/*
 * Large Objects are the only objects in the system that begin 
 * on a 16K byte boundary since the blocks used for smaller objects 
 * have the Block structure at each 16K boundary.
 */
static uintptr_t cleanupCacheIfNeed();

void CachedBlocksList::push(LargeMemoryBlock *ptr)
{   
    ptr->prev = NULL;
    ptr->age  = cleanupCacheIfNeed ();

    MallocMutex::scoped_lock scoped_cs(lock);
    ptr->next = first;
    first = ptr;
    if (ptr->next) ptr->next->prev = ptr;
    if (!last) {
        MALLOC_ASSERT(0 == oldest, ASSERT_TEXT);
        oldest = ptr->age;
        last = ptr;
    }
}

LargeMemoryBlock *CachedBlocksList::pop()
{   
    uintptr_t currAge = cleanupCacheIfNeed();
    LargeMemoryBlock *result=NULL;
    {
        MallocMutex::scoped_lock scoped_cs(lock);
        if (first) {
            result = first;
            first = result->next;
            if (first)  
                first->prev = NULL;
            else {
                last = NULL;
                oldest = 0;
            }
        } else {
            /* If cache miss occured, set ageThreshold to twice the difference 
               between current time and last time cache was cleaned. */
            ageThreshold = 2*(currAge - lastCleanedAge);
        }
    }
    return result;
}

void CachedBlocksList::releaseLastIfOld(uintptr_t currAge, size_t size)
{
    LargeMemoryBlock *toRelease = NULL;
 
    /* oldest may be more recent then age, that's why cast to signed type
       was used. age overflow is also processed correctly. */
    if (last && (intptr_t)(currAge - oldest) > ageThreshold) {
        MallocMutex::scoped_lock scoped_cs(lock);
        // double check
        if (last && (intptr_t)(currAge - last->age) > ageThreshold) {
            do {
                last = last->prev;
            } while (last && (intptr_t)(currAge - last->age) > ageThreshold);
            if (last) {
                toRelease = last->next;
                oldest = last->age;
                last->next = NULL;
            } else {
                toRelease = first;
                first = NULL;
                oldest = 0;
            }
            MALLOC_ASSERT( toRelease, ASSERT_TEXT );
            lastCleanedAge = toRelease->age;
        } 
        else 
            return;
    }
    while ( toRelease ) {
        LargeMemoryBlock *helper = toRelease->next;
        removeBackRef(toRelease->backRefIdx);
        freeRawMemory(toRelease, size, toRelease->fromMapMemory);
        toRelease = helper;
    }
}

/* A predicate checks whether an object starts on blockSize boundary */
static inline unsigned int isLargeObject(void *object)
{
    return isAligned(object, blockSize);
}

static uintptr_t cleanupCacheIfNeed ()
{
    /* loCacheStat.age overflow is OK, as we only want difference between 
     * its current value and some recent.
     *
     * Both malloc and free should increment loCacheStat.age, as in 
     * a different case multiple cached blocks would have same age,
     * and accuracy of predictors suffers.
     */
    uintptr_t currAge = (uintptr_t)AtomicIncrement((intptr_t&)loCacheStat.age);

    if ( 0 == currAge % cacheCleanupFreq ) {
        size_t objSize;
        int i;

        for (i = numLargeBlockBins-1, 
             objSize = (numLargeBlockBins-1)*largeBlockCacheStep+blockSize; 
             i >= 0; 
             i--, objSize-=largeBlockCacheStep) {
            /* cached block size on iteration is
             * i*largeBlockCacheStep+blockSize, it seems iterative
             * computation of it improves performance.
             */
            // release from cache blocks that are older than ageThreshold
            globalCachedBlockBins[i].releaseLastIfOld(currAge, objSize);
        }
    }
    return currAge;
}

static LargeMemoryBlock* getCachedLargeBlock (size_t size)
{
    MALLOC_ASSERT( size%largeBlockCacheStep==0, ASSERT_TEXT );
    LargeMemoryBlock *lmb = NULL;
    // blockSize is the minimal alignment and thus the minimal size of a large object.
    size_t idx = (size-blockSize)/largeBlockCacheStep;
    if (idx<numLargeBlockBins) {
        lmb = globalCachedBlockBins[idx].pop();
        if (lmb) {
            MALLOC_ITT_SYNC_ACQUIRED(globalCachedBlockBins+idx);
            STAT_increment(getThreadId(), ThreadCommonCounters, allocCachedLargeBlk);
        }
    }
    return lmb;
}

static inline void* mallocLargeObject (size_t size, size_t alignment, 
                                       bool startupAlloc = false)
{
    LargeMemoryBlock* lmb;
    size_t headersSize = sizeof(LargeMemoryBlock)+sizeof(LargeObjectHdr);
    size_t allocationSize = alignUp(size+headersSize+alignment, largeBlockCacheStep);

    if (startupAlloc || !(lmb = getCachedLargeBlock(allocationSize))) {
        BackRefIdx backRefIdx;

        if ((backRefIdx = newBackRef()) == invalidIdx) return NULL;
        lmb = (LargeMemoryBlock*)getRawMemory(allocationSize, /*useMapMem=*/startupAlloc);
        if (!lmb) return NULL;
        lmb->fromMapMemory = startupAlloc;
        lmb->backRefIdx = backRefIdx;
        lmb->unalignedSize = allocationSize;
        STAT_increment(getThreadId(), ThreadCommonCounters, allocNewLargeObj);
    }

    void *alignedArea = (void*)alignUp((uintptr_t)lmb+headersSize, alignment);
    LargeObjectHdr *header = (LargeObjectHdr*)alignedArea-1;
    header->memoryBlock = lmb;
    header->backRefIdx = lmb->backRefIdx;
    setBackRef(header->backRefIdx, header);
 
    lmb->objectSize = size;

    MALLOC_ASSERT( isLargeObject(alignedArea), ASSERT_TEXT );
    return alignedArea;
}

static bool freeLargeObjectToCache (LargeMemoryBlock* largeBlock)
{
    size_t size = largeBlock->unalignedSize;
    size_t idx = (size-blockSize)/largeBlockCacheStep;
    if (idx<numLargeBlockBins) {
        MALLOC_ASSERT( size%largeBlockCacheStep==0, ASSERT_TEXT );
        MALLOC_ITT_SYNC_RELEASING(globalCachedBlockBins+idx);
        globalCachedBlockBins[idx].push(largeBlock);

        STAT_increment(getThreadId(), ThreadCommonCounters, cacheLargeBlk);
        return true;
    }
    return false;
}

static inline void freeLargeObject (void *object)
{
    LargeObjectHdr *header = (LargeObjectHdr*)object - 1;

    // overwrite backRefIdx to simplify double free detection
    header->backRefIdx = invalidIdx;
    if (!freeLargeObjectToCache(header->memoryBlock)) {
        removeBackRef(header->memoryBlock->backRefIdx);
        freeRawMemory(header->memoryBlock, header->memoryBlock->unalignedSize, 
                      /*useMapMem=*/ header->memoryBlock->fromMapMemory);
        STAT_increment(getThreadId(), ThreadCommonCounters, freeLargeObj);
    }
}

/*********** End allocation of large objects **********/


static FreeObject *allocateFromFreeList(Block *mallocBlock)
{
    FreeObject *result;

    if (!mallocBlock->freeList) {
        return NULL;
    }

    result = mallocBlock->freeList;
    MALLOC_ASSERT( result, ASSERT_TEXT );

    mallocBlock->freeList = result->next;
    MALLOC_ASSERT( mallocBlock->allocatedCount < (blockSize-sizeof(Block))/mallocBlock->objectSize, ASSERT_TEXT );
    mallocBlock->allocatedCount++;
    STAT_increment(mallocBlock->owner, getIndex(mallocBlock->objectSize), allocFreeListUsed);

    return result;
}

static FreeObject *allocateFromBumpPtr(Block *mallocBlock)
{
    FreeObject *result = mallocBlock->bumpPtr;
    if (result) {
        mallocBlock->bumpPtr =
            (FreeObject *) ((uintptr_t) mallocBlock->bumpPtr - mallocBlock->objectSize);
        if ( (uintptr_t)mallocBlock->bumpPtr < (uintptr_t)mallocBlock+sizeof(Block) ) {
            mallocBlock->bumpPtr = NULL;
        }
        MALLOC_ASSERT( mallocBlock->allocatedCount < (blockSize-sizeof(Block))/mallocBlock->objectSize, ASSERT_TEXT );
        mallocBlock->allocatedCount++;
        STAT_increment(mallocBlock->owner, getIndex(mallocBlock->objectSize), allocBumpPtrUsed);
    }
    return result;
}

inline static FreeObject* allocateFromBlock( Block *mallocBlock )
{
    FreeObject *result;

    MALLOC_ASSERT( mallocBlock->owner == getThreadId(), ASSERT_TEXT );

    /* for better cache locality, first looking in the free list. */
    if ( (result = allocateFromFreeList(mallocBlock)) ) {
        return result;
    }
    MALLOC_ASSERT( !mallocBlock->freeList, ASSERT_TEXT );

    /* if free list is empty, try thread local bump pointer allocation. */
    if ( (result = allocateFromBumpPtr(mallocBlock)) ) {
        return result;
    }
    MALLOC_ASSERT( !mallocBlock->bumpPtr, ASSERT_TEXT );

    /* the block is considered full. */
    mallocBlock->isFull = 1;
    return NULL;
}

static void moveBlockToBinFront(Block *block)
{
    Bin* bin = getAllocationBin(block->objectSize);
    /* move the block to the front of the bin */
    outofTLSBin(bin, block);
    pushTLSBin(bin, block);
}

static void processLessUsedBlock(Block *block)
{
    Bin* bin = getAllocationBin(block->objectSize);
    if (block != getActiveBlock(bin)) {
        /* We are not actively using this block; return it to the general block pool */
        outofTLSBin(bin, block);
        returnEmptyBlock(block, /*poolTheBlock=*/true);
    } else {
        /* all objects are free - let's restore the bump pointer */
        restoreBumpPtr(block);
    }
}

/*
 * All aligned allocations fall into one of the following categories:
 *  1. if both request size and alignment are <= maxSegregatedObjectSize,
 *       we just align the size up, and request this amount, because for every size
 *       aligned to some power of 2, the allocated object is at least that aligned.
 * 2. for bigger size, check if already guaranteed fittingAlignment is enough.
 * 3. if size+alignment<minLargeObjectSize, we take an object of fittingSizeN and align
 *       its address up; given such pointer, scalable_free could find the real object.
 * 4. otherwise, aligned large object is allocated.
 */
static void *allocateAligned(size_t size, size_t alignment)
{
    MALLOC_ASSERT( isPowerOfTwo(alignment), ASSERT_TEXT );

    void *result;
    if (size<=maxSegregatedObjectSize && alignment<=maxSegregatedObjectSize)
        result = scalable_malloc(alignUp(size? size: sizeof(size_t), alignment));
    else if (alignment<=fittingAlignment)
        result = scalable_malloc(size);
    else if (size+alignment < minLargeObjectSize) {
        void *unaligned = scalable_malloc(size+alignment);
        if (!unaligned) return NULL;
        result = alignUp(unaligned, alignment);
    } else {
        /* This can be the first allocation call. */
        checkInitialization();
        /* To correctly detect kind of allocation in scalable_free we need 
           to distinguish memory allocated as large object.
           This is done via alignment that is greater than can be found in Block.
        */ 
        result = mallocLargeObject(size, blockSize>alignment? blockSize: alignment);
    }

    MALLOC_ASSERT( isAligned(result, alignment), ASSERT_TEXT );
    return result;
}

static void *reallocAligned(void *ptr, size_t size, size_t alignment = 0)
{
    void *result;
    size_t copySize;

    if (isLargeObject(ptr)) {
        LargeMemoryBlock* lmb = ((LargeObjectHdr *)ptr - 1)->memoryBlock;
        copySize = lmb->unalignedSize-((uintptr_t)ptr-(uintptr_t)lmb);
        if (size <= copySize && (0==alignment || isAligned(ptr, alignment))) {
            lmb->objectSize = size;
            return ptr;
        } else {
            copySize = lmb->objectSize;
            result = alignment ? allocateAligned(size, alignment) : scalable_malloc(size);
        }
    } else {
        Block* block = (Block *)alignDown(ptr, blockSize);
        copySize = block->objectSize;
        if (size <= copySize && (0==alignment || isAligned(ptr, alignment))) {
            return ptr;
        } else {
            result = alignment ? allocateAligned(size, alignment) : scalable_malloc(size);
        }
    }
    if (result) {
        memcpy(result, ptr, copySize<size? copySize: size);
        scalable_free(ptr);
    }
    return result;
}

/* A predicate checks if an object is properly placed inside its block */
static inline bool isProperlyPlaced(const void *object, const Block *block)
{
    return 0 == ((uintptr_t)block + blockSize - (uintptr_t)object) % block->objectSize;
}

/* Finds the real object inside the block */
static inline FreeObject *findAllocatedObject(const void *address, const Block *block)
{
    // calculate offset from the end of the block space
    uintptr_t offset = (uintptr_t)block + blockSize - (uintptr_t)address;
    MALLOC_ASSERT( offset<blockSize-sizeof(Block), ASSERT_TEXT );
    // find offset difference from a multiple of allocation size
    offset %= block->objectSize;
    // and move the address down to where the real object starts.
    return (FreeObject*)((uintptr_t)address - (offset? block->objectSize-offset: 0));
}

} // namespace internal
} // namespace rml

using namespace rml::internal;

/*
 * When a thread is shutting down this routine should be called to remove all the thread ids
 * from the malloc blocks and replace them with a NULL thread id.
 *
 */
#if MALLOC_TRACE
static unsigned int threadGoingDownCount = 0;
#endif

/*
 * for pthreads, the function is set as a callback in pthread_key_create for TLS bin.
 * it will be automatically called at thread exit with the key value as the argument.
 *
 * for Windows, it should be called directly e.g. from DllMain; the argument can be NULL
 * one should include "TypeDefinitions.h" for the declaration of this function.
*/
extern "C" void mallocThreadShutdownNotification(void* arg)
{
    TLSData *tls;
    Block *threadBlock;
    Block *threadlessBlock;
    unsigned int index;

    {
        MallocMutex::scoped_lock lock( initAndShutMutex );
        if ( mallocInitialized == 0 ) return;
    }

    TRACEF(( "[ScalableMalloc trace] Thread id %d blocks return start %d\n",
             getThreadId(),  threadGoingDownCount++ ));
#ifdef USE_WINTHREAD
    tls = getThreadMallocTLS();
#else
    tls = (TLSData*)arg;
#endif
    if (tls) {
        Bin *tlsBin = tls->bin;
        tls->pool.releaseAllBlocks();

        for (index = 0; index < numBlockBins; index++) {
            if (tlsBin[index].activeBlk==NULL)
                continue;
            threadlessBlock = tlsBin[index].activeBlk->previous;
            while (threadlessBlock) {
                threadBlock = threadlessBlock->previous;
                if (threadlessBlock->allocatedCount==0 && threadlessBlock->publicFreeList==NULL) {
                    /* we destroy the thread, so not use its block pool */
                    returnEmptyBlock(threadlessBlock, /*poolTheBlock=*/false);
                } else {
                    returnPartialBlock(tlsBin+index, threadlessBlock);
                }
                threadlessBlock = threadBlock;
            }
            threadlessBlock = tlsBin[index].activeBlk;
            while (threadlessBlock) {
                threadBlock = threadlessBlock->next;
                if (threadlessBlock->allocatedCount==0 && threadlessBlock->publicFreeList==NULL) {
                    /* we destroy the thread, so not use its block pool */
                    returnEmptyBlock(threadlessBlock, /*poolTheBlock=*/false);
                } else {
                    returnPartialBlock(tlsBin+index, threadlessBlock);
                }
                threadlessBlock = threadBlock;
            }
            tlsBin[index].activeBlk = 0;
        }
        bootStrapFree(tls);
        setThreadMallocTLS(NULL);
    }

    TRACEF(( "[ScalableMalloc trace] Thread id %d blocks return end\n", getThreadId() ));
}

extern "C" void mallocProcessShutdownNotification(void)
{
#if COLLECT_STATISTICS
    ThreadId nThreads = ThreadIdCount;
    for( int i=1; i<=nThreads && i<MAX_THREADS; ++i )
        STAT_print(i);
#endif
}

/**** Check if an object was allocated by scalable_malloc ****/

/* 
 * Bad dereference caused by a foreign pointer is possible only here, not earlier in call chain.
 * Separate function isolates SEH code, as it has bad influence on compiler optimization.
 */
static inline BackRefIdx safer_dereference (BackRefIdx *ptr)
{
    BackRefIdx id;
#if _MSC_VER
    __try {
#endif
        id = *ptr;
#if _MSC_VER
    } __except( GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION? 
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {
        id = invalidIdx;
    }
#endif
    return id;
}

static inline bool isRecognized (void* ptr)
{
    void* expected;
    BackRefIdx* idx;

    if (isLargeObject(ptr)) {
        expected = (LargeObjectHdr*)ptr - 1;
        idx = &((LargeObjectHdr*)expected)->backRefIdx;
    } else {
        expected = alignDown(ptr, blockSize);
        idx = &((Block*)expected)->backRefIdx;
    }
    return expected == getBackRef(safer_dereference(idx));
}

/********* The malloc code          *************/

extern "C" void * scalable_malloc(size_t size)
{
    Bin* bin;
    Block * mallocBlock;
    FreeObject *result;

    if (!size) size = sizeof(size_t);

#if MALLOC_CHECK_RECURSION
    if (RecursiveMallocCallProtector::sameThreadActive()) {
        result = size<minLargeObjectSize? startupAlloc(size) : 
              (FreeObject*)mallocLargeObject(size, blockSize, /*startupAlloc=*/ true);
        if (!result) errno = ENOMEM;
        return result;
    }
#endif

    /* This returns only after malloc is initialized */
    checkInitialization();

    /*
     * Use Large Object Allocation
     */
    if (size >= minLargeObjectSize) {
        result = (FreeObject*)mallocLargeObject(size, blockSize);
        if (!result) errno = ENOMEM;
        return result;
    }

    /*
     * Get an element in thread-local array corresponding to the given size;
     * It keeps ptr to the active block for allocations of this size
     */
    bin = getAllocationBin(size);
    if ( !bin ) {
        errno = ENOMEM;
        return NULL;
    }

    /* Get the block of you want to try to allocate in. */
    mallocBlock = getActiveBlock(bin);

    if (mallocBlock) {
        do {
            if( (result = allocateFromBlock(mallocBlock)) ) {
                return result;
            }
            // the previous block, if any, should be empty enough
        } while( (mallocBlock = setPreviousBlockActive(bin)) );
    }
    MALLOC_ASSERT( !(bin->activeBlk) || bin->activeBlk->isFull==1, ASSERT_TEXT );

    /*
     * else privatize publicly freed objects in some block and allocate from it
     */
    mallocBlock = getPublicFreeListBlock( bin );
    if (mallocBlock) {
        if (emptyEnoughToUse(mallocBlock)) {
            /* move the block to the front of the bin */
            outofTLSBin(bin, mallocBlock);
            pushTLSBin(bin, mallocBlock);
        }
        MALLOC_ASSERT( mallocBlock->freeList, ASSERT_TEXT );
        if ( (result = allocateFromFreeList(mallocBlock)) ) {
            return result;
        }
        /* Else something strange happened, need to retry from the beginning; */
        TRACEF(( "[ScalableMalloc trace] Something is wrong: no objects in public free list; reentering.\n" ));
        return scalable_malloc(size);
    }

    /*
     * no suitable own blocks, try to get a partial block that some other thread has discarded.
     */
    mallocBlock = getPartialBlock(bin, size);
    while (mallocBlock) {
        pushTLSBin(bin, mallocBlock);
// guaranteed by pushTLSBin: MALLOC_ASSERT( *bin==mallocBlock || (*bin)->previous==mallocBlock, ASSERT_TEXT );
        setActiveBlock(bin, mallocBlock);
        if( (result = allocateFromBlock(mallocBlock)) ) {
            return result;
        }
        mallocBlock = getPartialBlock(bin, size);
    }

    /*
     * else try to get a new empty block
     */
    mallocBlock = getEmptyBlock(size);
    if (mallocBlock) {
        pushTLSBin(bin, mallocBlock);
// guaranteed by pushTLSBin: MALLOC_ASSERT( *bin==mallocBlock || (*bin)->previous==mallocBlock, ASSERT_TEXT );
        setActiveBlock(bin, mallocBlock);
        if( (result = allocateFromBlock(mallocBlock)) ) {
            return result;
        }
        /* Else something strange happened, need to retry from the beginning; */
        TRACEF(( "[ScalableMalloc trace] Something is wrong: no objects in empty block; reentering.\n" ));
        return scalable_malloc(size);
    }
    /*
     * else nothing works so return NULL
     */
    TRACEF(( "[ScalableMalloc trace] No memory found, returning NULL.\n" ));
    errno = ENOMEM;
    return NULL;
}

/********* End the malloc code      *************/

/********* The free code            *************/

extern "C" void scalable_free (void *object) {
    Block *block;
    ThreadId myTid;
    FreeObject *objectToFree;

    if (!object) {
        return;
    }
    MALLOC_ASSERT(isRecognized(object), "Invalid pointer in scalable_free detected.");

    if (isLargeObject(object)) {
        freeLargeObject(object);
        return;
    } 

    block = (Block *)alignDown(object, blockSize);/* mask low bits to get the block */
    MALLOC_ASSERT( block->allocatedCount, ASSERT_TEXT );

#if MALLOC_CHECK_RECURSION
    if (block->objectSize == startupAllocObjSizeMark) {
        startupFree((StartupBlock *)block, object);
        return;
    }
#endif

    myTid = getThreadId();

    // Due to aligned allocations, a pointer passed to scalable_free
    // might differ from the address of internally allocated object.
    // Small objects however should always be fine.    
    if (block->objectSize <= maxSegregatedObjectSize)
        objectToFree = (FreeObject*)object;
    // "Fitting size" allocations are suspicious if aligned higher than naturally
    else {
        if ( ! isAligned(object,2*fittingAlignment) )
        // TODO: the above check is questionable - it gives false negatives in ~50% cases,
        //       so might even be slower in average than unconditional use of findAllocatedObject.
        // here it should be a "real" object
            objectToFree = (FreeObject*)object;
        else
        // here object can be an aligned address, so applying additional checks
            objectToFree = findAllocatedObject(object, block);
        MALLOC_ASSERT( isAligned(objectToFree,fittingAlignment), ASSERT_TEXT );
    }
    MALLOC_ASSERT( isProperlyPlaced(objectToFree, block), ASSERT_TEXT );

    if (myTid == block->owner) {
        objectToFree->next = block->freeList;
        block->freeList = objectToFree;
        block->allocatedCount--;
        MALLOC_ASSERT( block->allocatedCount < (blockSize-sizeof(Block))/block->objectSize, ASSERT_TEXT );
#if COLLECT_STATISTICS
        if (getActiveBlock(getAllocationBin(block->objectSize)) != block)
            STAT_increment(myTid, getIndex(block->objectSize), freeToInactiveBlock);
        else
            STAT_increment(myTid, getIndex(block->objectSize), freeToActiveBlock);
#endif
        if (block->isFull) {
            if (emptyEnoughToUse(block))
                moveBlockToBinFront(block);
        } else {
            if (block->allocatedCount==0 && block->publicFreeList==NULL)
                processLessUsedBlock(block);
        }
    } else { /* Slower path to add to the shared list, the allocatedCount is updated by the owner thread in malloc. */
        freePublicObject (block, objectToFree);
    }
}

/*
 * A variant that provides additional memory safety, by checking whether the given address
 * was obtained with this allocator, and if not redirecting to the provided alternative call.
 */
extern "C" void safer_scalable_free (void *object, void (*original_free)(void*)) 
{
    if (!object)
        return;

    if (isRecognized(object))
        scalable_free(object);
    else if (original_free)
        original_free(object);
}

/********* End the free code        *************/

/********* Code for scalable_realloc       ***********/

/*
 * From K&R
 * "realloc changes the size of the object pointed to by p to size. The contents will
 * be unchanged up to the minimum of the old and the new sizes. If the new size is larger,
 * the new space is uninitialized. realloc returns a pointer to the new space, or
 * NULL if the request cannot be satisfied, in which case *p is unchanged."
 *
 */
extern "C" void* scalable_realloc(void* ptr, size_t size)
{
    /* corner cases left out of reallocAligned to not deal with errno there */
    if (!ptr) {
        return scalable_malloc(size);
    }
    if (!size) {
        scalable_free(ptr);
        return NULL;
    }
    void* tmp = reallocAligned(ptr, size, 0);
    if (!tmp) errno = ENOMEM;
    return tmp;
}

/*
 * A variant that provides additional memory safety, by checking whether the given address
 * was obtained with this allocator, and if not redirecting to the provided alternative call.
 */
extern "C" void* safer_scalable_realloc (void* ptr, size_t sz, void* original_realloc) 
{
    if (!ptr) {
        return scalable_malloc(sz);
    }
    if (isRecognized(ptr)) {
        if (!sz) {
            scalable_free(ptr);
            return NULL;
        }
        void* tmp = reallocAligned(ptr, sz, 0);
        if (!tmp) errno = ENOMEM;
        return tmp;
    }
#if USE_WINTHREAD
    else if (original_realloc && sz) {
            orig_ptrs *original_ptrs = static_cast<orig_ptrs*>(original_realloc);
            if ( original_ptrs->orig_msize ){
                size_t oldSize = original_ptrs->orig_msize(ptr);
                void *newBuf = scalable_malloc(sz);
                if (newBuf) {
                    memcpy(newBuf, ptr, sz<oldSize? sz : oldSize);
                    if ( original_ptrs->orig_free ){
                        original_ptrs->orig_free( ptr );
                    }
                }
                return newBuf;
             }
    }
#else
    else if (original_realloc) {
        typedef void* (*realloc_ptr_t)(void*,size_t);
        realloc_ptr_t original_realloc_ptr;
        (void *&)original_realloc_ptr = original_realloc;
        return original_realloc_ptr(ptr,sz);
    }
#endif
    return NULL;
}

/********* End code for scalable_realloc   ***********/

/********* Code for scalable_calloc   ***********/

/*
 * From K&R
 * calloc returns a pointer to space for an array of nobj objects, 
 * each of size size, or NULL if the request cannot be satisfied. 
 * The space is initialized to zero bytes.
 *
 */

extern "C" void * scalable_calloc(size_t nobj, size_t size)
{
    size_t arraySize = nobj * size;
    void* result = scalable_malloc(arraySize);
    if (result)
        memset(result, 0, arraySize);
    return result;
}

/********* End code for scalable_calloc   ***********/

/********* Code for aligned allocation API **********/

extern "C" int scalable_posix_memalign(void **memptr, size_t alignment, size_t size)
{
    if ( !isPowerOfTwoMultiple(alignment, sizeof(void*)) )
        return EINVAL;
    void *result = allocateAligned(size, alignment);
    if (!result)
        return ENOMEM;
    *memptr = result;
    return 0;
}

extern "C" void * scalable_aligned_malloc(size_t size, size_t alignment)
{
    if (!isPowerOfTwo(alignment) || 0==size) {
        errno = EINVAL;
        return NULL;
    }
    void* tmp = allocateAligned(size, alignment);
    if (!tmp) 
        errno = ENOMEM;
    return tmp;
}

extern "C" void * scalable_aligned_realloc(void *ptr, size_t size, size_t alignment)
{
    /* corner cases left out of reallocAligned to not deal with errno there */
    if (!isPowerOfTwo(alignment)) {
        errno = EINVAL;
        return NULL;
    }
    if (!ptr) {
        return allocateAligned(size, alignment);
    }
    if (!size) {
        scalable_free(ptr);
        return NULL;
    }

    void* tmp = reallocAligned(ptr, size, alignment);
    if (!tmp) errno = ENOMEM;
    return tmp;
}

extern "C" void * safer_scalable_aligned_realloc(void *ptr, size_t size, size_t alignment, void* orig_function)
{
    /* corner cases left out of reallocAligned to not deal with errno there */
    if (!isPowerOfTwo(alignment)) {
        errno = EINVAL;
        return NULL;
    }
    if (!ptr) {
        return allocateAligned(size, alignment);
    }
    if (isRecognized(ptr)) {
        if (!size) {
            scalable_free(ptr);
            return NULL;
        }
        void* tmp = reallocAligned(ptr, size, alignment);
        if (!tmp) errno = ENOMEM;
        return tmp;
    }
#if USE_WINTHREAD
    else {
        orig_ptrs *original_ptrs = static_cast<orig_ptrs*>(orig_function);
        if (size) {
            if ( original_ptrs->orig_msize ){
                size_t oldSize = original_ptrs->orig_msize(ptr);
                void *newBuf = allocateAligned(size, alignment);
                if (newBuf) {
                    memcpy(newBuf, ptr, size<oldSize? size : oldSize);
                    if ( original_ptrs->orig_free ){
                        original_ptrs->orig_free( ptr );
                    }
                }
                return newBuf;
            }else{
                //We can't do anything with this. Just keeping old pointer
                return NULL;
            }
        } else {
            if ( original_ptrs->orig_free ){
                original_ptrs->orig_free( ptr );
            }
            return NULL;
        }
    }
#endif
    return NULL;
}

extern "C" void scalable_aligned_free(void *ptr)
{
    scalable_free(ptr);
}

/********* end code for aligned allocation API **********/

/********* Code for scalable_msize       ***********/

/*
 * Returns the size of a memory block allocated in the heap.
 */
extern "C" size_t scalable_msize(void* ptr)
{
    if (ptr) {
        MALLOC_ASSERT(isRecognized(ptr), "Invalid pointer in scalable_msize detected.");
        if (isLargeObject(ptr)) {
            LargeMemoryBlock* lmb = ((LargeObjectHdr*)ptr - 1)->memoryBlock;
            return lmb->objectSize;
        } else {
            Block* block = (Block *)alignDown(ptr, blockSize);
#if MALLOC_CHECK_RECURSION
            size_t size = block->objectSize? block->objectSize : startupMsize(ptr);
#else
            size_t size = block->objectSize;
#endif
            MALLOC_ASSERT(size>0 && size<minLargeObjectSize, ASSERT_TEXT);
            return size;
        }
    }
    errno = EINVAL;
    // Unlike _msize, return 0 in case of parameter error.
    // Returning size_t(-1) looks more like the way to troubles.
    return 0;
}

/*
 * A variant that provides additional memory safety, by checking whether the given address
 * was obtained with this allocator, and if not redirecting to the provided alternative call.
 */
extern "C" size_t safer_scalable_msize (void *object, size_t (*original_msize)(void*)) 
{
    if (object) {
        // Check if the memory was allocated by scalable_malloc
        if (isRecognized(object))
            return scalable_msize(object);
        else if (original_msize)
            return original_msize(object);
    }
    // object is NULL or unknown
    errno = EINVAL;
    return 0;
}

/********* End code for scalable_msize   ***********/
