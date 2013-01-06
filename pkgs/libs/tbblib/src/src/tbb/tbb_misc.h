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

#ifndef _TBB_tbb_misc_H
#define _TBB_tbb_misc_H

#include "tbb/tbb_stddef.h"
#include "tbb/tbb_machine.h"

#if _WIN32||_WIN64
#if _XBOX
    #define NONET
    #define NOD3D
    #include <xtl.h>
#else
#include <windows.h>
#endif
#elif defined(__linux__)
#include <sys/sysinfo.h>
#elif defined(__sun)
#include <sys/sysinfo.h>
#include <unistd.h>
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#elif defined(__FreeBSD__)
#include <unistd.h>
#endif

namespace tbb {
namespace internal {

const size_t MByte = 1<<20;

#if !defined(__TBB_WORDSIZE)
    const size_t ThreadStackSize = 1*MByte;
#elif __TBB_WORDSIZE<=4
    const size_t ThreadStackSize = 2*MByte;
#else
    const size_t ThreadStackSize = 4*MByte;
#endif

#if defined(__TBB_DetectNumberOfWorkers)

static inline int DetectNumberOfWorkers() {
    return __TBB_DetectNumberOfWorkers(); 
}

#else /* !__TBB_DetectNumberOfWorkers */

#if _WIN32||_WIN64

#if _XBOX

// This port uses only 2 hardware threads for TBB on XBOX 360. 
// Others are left to sound etc.
// Change the following mask to allow TBB use more HW threads.
static const int XBOX360_HARDWARE_THREAD_MASK = 0x0C;

static inline int DetectNumberOfWorkers() 
{
 char a[XBOX360_HARDWARE_THREAD_MASK];  //compile time assert - at least one bit should be set always
 a[0]=0;
 
 return ((XBOX360_HARDWARE_THREAD_MASK >> 0) & 1) +
        ((XBOX360_HARDWARE_THREAD_MASK >> 1) & 1) +
        ((XBOX360_HARDWARE_THREAD_MASK >> 2) & 1) +
        ((XBOX360_HARDWARE_THREAD_MASK >> 3) & 1) +
        ((XBOX360_HARDWARE_THREAD_MASK >> 4) & 1) +
        ((XBOX360_HARDWARE_THREAD_MASK >> 5) & 1) + 1;  //+1 - tbb is creating DetectNumberOfWorkers()-1 threads in arena 
}

static inline int GetHardwareThreadIndex(int workerThreadIndex)
{
 workerThreadIndex %= DetectNumberOfWorkers()-1;
 int m = XBOX360_HARDWARE_THREAD_MASK;
 int index = 0;
 int skipcount = workerThreadIndex;
 while (true)
  {
   if ((m & 1)!=0) 
    {
     if (skipcount==0) break;
     skipcount--;
    }
   m >>= 1;
   index++;
  }
 return index; 
}

#else /* !_XBOX */

static inline int DetectNumberOfWorkers() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return static_cast<int>(si.dwNumberOfProcessors);
}

#endif /* !_XBOX */

#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__sun) 
static inline int DetectNumberOfWorkers() {
    long number_of_workers;

#if (defined(__FreeBSD__) || defined(__sun)) && defined(_SC_NPROCESSORS_ONLN) 
    number_of_workers = sysconf(_SC_NPROCESSORS_ONLN);

// In theory, sysconf should work everywhere.
// But in practice, system-specific methods are more reliable
#elif defined(__linux__)
    number_of_workers = get_nprocs();
#elif defined(__APPLE__)
    int name[2] = {CTL_HW, HW_AVAILCPU};
    int ncpu;
    size_t size = sizeof(ncpu);
    sysctl( name, 2, &ncpu, &size, NULL, 0 );
    number_of_workers = ncpu;
#else
#error DetectNumberOfWorkers: Method to detect the number of online CPUs is unknown
#endif

// Fail-safety strap
    if ( number_of_workers < 1 ) {
        number_of_workers = 1;
    }
    
    return number_of_workers;
}

#else
#error DetectNumberOfWorkers: OS detection method is unknown
#endif /* os kind */

#endif /* !__TBB_DetectNumberOfWorkers */

//! Throws std::runtime_error with what() returning error_code description prefixed with aux_info
void handle_win_error( int error_code );

//! True if environment variable with given name is set and not 0; otherwise false.
bool GetBoolEnvironmentVariable( const char * name );

//! Print TBB version information on stderr
void PrintVersion();

//! Print extra TBB version information on stderr
void PrintExtraVersionInfo( const char* category, const char* description );

//! A callback routine to print RML version information on stderr
void PrintRMLVersionInfo( void* arg, const char* server_info );

// For TBB compilation only; not to be used in public headers
#if defined(min) || defined(max)
#undef min
#undef max
#endif

//! Utility template function returning lesser of the two values.
/** Provided here to avoid including not strict safe <algorithm>.\n
    In case operands cause signed/unsigned or size mismatch warnings it is caller's
    responsibility to do the appropriate cast before calling the function. **/
template<typename T1, typename T2>
T1 min ( const T1& val1, const T2& val2 ) {
    return val1 < val2 ? val1 : val2;
}

//! Utility template function returning greater of the two values.
/** Provided here to avoid including not strict safe <algorithm>.\n
    In case operands cause signed/unsigned or size mismatch warnings it is caller's
    responsibility to do the appropriate cast before calling the function. **/
template<typename T1, typename T2>
T1 max ( const T1& val1, const T2& val2 ) {
    return val1 < val2 ? val2 : val1;
}

//------------------------------------------------------------------------
// FastRandom
//------------------------------------------------------------------------

/** Defined in tbb_main.cpp **/
unsigned GetPrime ( unsigned seed );

//! A fast random number generator.
/** Uses linear congruential method. */
class FastRandom {
    unsigned x, a;
public:
    //! Get a random number.
    unsigned short get() {
        return get(x);
    }
    //! Get a random number for the given seed; update the seed for next use.
    unsigned short get( unsigned& seed ) {
        unsigned short r = (unsigned short)(seed>>16);
        seed = seed*a+1;
        return r;
    }
    //! Construct a random number generator.
    FastRandom( unsigned seed ) {
        x = seed;
        a = GetPrime( seed );
    }
};

} // namespace internal
} // namespace tbb

#endif /* _TBB_tbb_misc_H */
