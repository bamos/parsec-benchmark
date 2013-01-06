#ifndef RTTL_INCLUDE_HXX
#define RTTL_INCLUDE_HXX

#ifdef _MSC_VER
// The next warning happens in system files.
// 4530 warning is a little bit strange: /EHsc *IS* actually enabled.
#pragma warning(disable: 4530) // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
#endif

//#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>

#ifdef _WIN32
#define access     _access
#define strcasecmp stricmp
#define ftruncate  truncate
#endif //_WIN32

#define getcwd     _getcwd
#define chdir      _chdir
#define fileno     _fileno
#define sleep      Sleep
#define strdup     _strdup

// default includes
#include <assert.h>
#include <limits.h>
#include <float.h>
#if defined(__INTEL_COMPILER)
#include <mathimf.h>
#else
#include <math.h>
#endif

// --------------------------------------------
/* call config.h; if we use CMake, use the cmake-expanded header
   file. if we use a build system that doesn't support this feature,
   include the original source header file */
#define CALLER_IS_COMMON_RTINCLUDE_HXX
#ifdef THIS_IS_CMAKE
    #include "cmake_autoconfig.h"
#else
    #include "non_cmake_config.h"
#endif
#undef CALLER_IS_COMMON_RTINCLUDE_HXX
// --------------------------------------------


#define extern_inline extern inline 



#ifdef THIS_IS_APPLE
    #include <sys/malloc.h>
#else 
    #include <malloc.h>
#endif

#include <iostream>
#include <algorithm>
#include <limits>

#if defined(__GNUC__)
#include <sys/times.h>
#endif



using namespace std;

#define DBG_PRINT(x) cout << #x << " = " << (x) << flush << endl
#define PING cout << __PRETTY_FUNCTION__ << flush << endl
#ifdef __wald__
// force FATAL to core dump (allows debugging...)
#define FATAL(a) { cerr << "FATAL: " << a << "(in " << __PRETTY_FUNCTION__ << ")" << endl; *((int *)0) = 1; exit(1); } 
#else
#define FATAL(a) { cerr << "FATAL: " << a << "(in " << __PRETTY_FUNCTION__ << ")" << endl; exit(1); }
#endif


#define ENTER cout << "entering " << __PRETTY_FUNCTION__ << endl;
#define LEAVE cout << "leaving  " << __PRETTY_FUNCTION__ << endl;
#define PRINT(x) cout << #x << " =" << x << endl
#define WARN(a)     { cerr << "Warning (in " << __PRETTY_FUNCTION__ << ") : " << a << endl;  }
#define DEPRECATED()   cerr << "function '" << __PRETTY_FUNCTION__ << "' IS DEPRECATED" << endl;
#define NOTIMPLEMENTED cout << "function '" << __PRETTY_FUNCTION__ << "' not yet implemented" << endl;
// It is true for Windows, not sure about Linux...
#define NOTINITIALIZED(v) (*(unsigned int*)(&(v)) == 0xcdcdcdcd || *(unsigned int*)(&(v)) == 0xcccccccc)

/* default alignment for memory blocks allocated by malloc_align */


#define DEFAULT_ALIGNMENT 64

#if !defined(__GNUC__)
#define M_PI 3.14159265f
#endif

// Under Linux, __GNUC__ is defined for icc as well (togeather with __INTEL_COMPILER)

#if defined(_MSC_VER)
#define __PRETTY_FUNCTION__ __FILE__ << ":" << __LINE__
#pragma warning(disable: 4267) // conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable: 4996) // use of deprecated function (like sprintf)
#pragma warning(disable: 4068) // unknown pragma
#pragma warning(disable: 4800) // forcing value to bool 'true' or 'false' (performance warning)
#if !defined(__INTEL_COMPILER)
__forceinline int __builtin_expect(int a, int p) { return a; }
#endif
#endif
#if defined(__INTEL_COMPILER)
#pragma warning(disable: 1478) // use of deprecated function (like sprintf)
#pragma warning(disable: 1786) // use of deprecated function (like sprintf)
#endif

#if defined(__INTEL_COMPILER)
    // && !defined(__WIN32) ???
    #define RESTRICT restrict
#else
    #define RESTRICT /* that compiler doesn't support restrict keyword */
#endif


/// Define _INLINE macro.
#ifdef DEBUG
#define _INLINE inline
#else
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
    #define _INLINE __forceinline
#elif defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 900)
    #define _INLINE __forceinline
#elif defined(__GNUC__) && ((__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 2))) && !defined(__INTEL_COMPILER)
    #define _INLINE inline __attribute__((always_inline))
#else
    #define _INLINE inline
#endif
#endif

//// Define _ALIGN macro.
#if defined(_MSC_VER)
    #define _ALIGN(sz) __declspec(align(sz))
#elif defined(__GNUC__)
    #define _ALIGN(sz) __attribute__((aligned(sz)))
#elif defined(__INTEL_COMPILER)
    #define _ALIGN(sz) __declspec(align(sz))
#endif

/// Defined if all SSE ops are emulated (SSE-less implementation).
#define RT_EMULATE_SSE
#ifdef  RT_EMULATE_SSE
#include "RTEmulatedSSE.hxx"

#define _MM_SHUFFLE(fp3,fp2,fp1,fp0) (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))

#else
// "Real" SSE

#ifdef _MSC_VER
#include <intrin.h>
#endif
#include <xmmintrin.h>
#include <emmintrin.h>

#if defined(__GNUC__) && !defined(__INTEL_COMPILER)

_INLINE float  abs(float v)  { return fabsf(v); }
_INLINE double abs(double v) { return fabs(v);  }
_INLINE long   abs(long v)   { return labs(v);  }

// GNU C++ follows strict standard interpretation in which
// __m128/i are primary data types for which
// operator overloading is prohibited.
// So, we treat __m128/i as classes.
struct sse_f {
    sse_f() {}
    sse_f(__m128 a) : t(a) {}
    operator __m128() const {
        return t;
    }
private:
    __m128 t;
};
#define __m128 sse_f
struct sse_i {
    sse_i() {}
    sse_i(__m128i a) : t(a) {}
    operator __m128i() const {
        return t;
    }
private:
    __m128i t;
};
#define __m128i sse_i
#else
// All other compilers (icc, mvc)
typedef __m128  sse_f;
typedef __m128i sse_i;
#endif
typedef __m64 sse_i64;
#endif

_INLINE void *malloc_align(const int size,const int alignment = DEFAULT_ALIGNMENT) {
#if defined (__GNUC__) && !defined (__INTEL_COMPILER)
    return memalign(alignment,size);
#else
    return _mm_malloc(size,alignment);
#endif
}

_INLINE void free_align(void *ptr) {
#if defined (__GNUC__) && !defined (__INTEL_COMPILER)
    free(ptr);
#else
    _mm_free(ptr);
#endif
}



template< typename T, int alignment = DEFAULT_ALIGNMENT > class Align
{
public:
   typedef T value_type;
   typedef value_type* pointer;
   typedef const value_type* const_pointer;
   typedef value_type& reference;
   typedef const value_type& const_reference;
   typedef size_t size_type;
   typedef std::ptrdiff_t difference_type;

  // Make VS 2008 happy.
  Align() {}
  Align(T const&) {}

  pointer allocate( size_type n ) {
      void* mem = malloc_align(n * sizeof( value_type ), alignment );
       return reinterpret_cast< pointer >( mem );
   }
  _INLINE void deallocate( pointer p, size_type ) {
     if (p)
       free_align( p );
   }
  _INLINE void construct( pointer p, const T& t ) {
      new( p ) T( t );
   }
  _INLINE void destroy( pointer p ) {
      p->~T();
   }
  template <class U>
  class rebind
  {
    public: typedef Align<U> other;
  };

  size_type max_size() const {
    return static_cast<size_type>(-1) / sizeof(value_type);
  }

};

/*! tests whether a given pointer is aligned to an N byte boundary */
template<size_t N, class T>
inline bool is_aligned(T *t)
{
  long l = (long)(((unsigned char *)t) - ((unsigned char *)NULL));
  return (l % N) == 0;
}

/*! tests whether a given integral type is divisible by N */
template<size_t N, class T>
inline bool is_divisible(T t)
{
  return (t % N) == 0;
}

/*! tests whether a given integral type is divisible by N */
template<int N, class T>
inline T nextMultipleOf(T t)
{
  return ((t+N-1) / N) * N;
}

/*! allocate an array of N elemnts of type T, such that it is aligned
  to 16 bytes */
template<class T>
T *aligned_malloc(int N)
{ return (T*)malloc_align(N * sizeof(T)); };

/*! free an array allocatedby aligned_malloc */
template<class T>
void aligned_free(T *t)
{ return free_align(t); };

_INLINE float MBytes(unsigned int n) { return (float)n / 1024.0f; }
_INLINE float KBytes(unsigned int n) { return (float)n / 1024.0f; }

#include <sstream>
#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#endif

#endif
