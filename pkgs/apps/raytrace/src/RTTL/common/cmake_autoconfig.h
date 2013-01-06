/*
\file config.h

this file should never be included directly, it should _only_ be
called by RTTL/common/RTInclude.hxx

we're adding a little define to make sure that happens...

\note cmake passes bools as ON/OFF text constants. to make that
understandable to the preprocessor we never use boolean cmake
variables directly, and instaed use "CMAKE_", plus manually
defining CMAKE_ON=1 and CMAKE_OFF=0. If you get some unexpected
behavior of your cmake bools, check if you're always prefixing them
with "CMAKE_"...
*/


#ifndef CALLER_IS_COMMON_RTINCLUDE_HXX
# error "this file should only be called by RTInclude.hxx"
#endif

#ifndef THIS_IS_CMAKE
# error "cmake_config.h should _not_ be used when using anything else but cmake to build the codebase"
// note: when using cmake, we have cmake parse
// cmake_autoconfig_template.h, and have it replace all cmake
// variables with the cmake configured values, which are then written
// toa file called cmake_autoconfig.h thus, when using cmake, you
// should only be including cmake_autoconfig.h
#endif


// cmake-to-bool conversion ....
#  define CMAKE_ON 1
#  define CMAKE_OFF 0
#  define CMAKE_1 1
#  define CMAKE_0 0


#define USE_PBOS CMAKE_1
#if 0
#define NEED_ARB_WRAPPERS
#endif
// #define USE_PBOS CMAKE_1
// #if CMAKE_0
// #define NEED_ARB_WRAPPERS
// #endif


#  define DO_STATS_BVH CMAKE_default


namespace RTTL {
static const char *defaultBVHBuilder = "default";
}


#define  NEEDS_PTHREAD_BARRIER_T_WRAPPER
/* #undef  APPLE */
#ifdef APPLE
#  define THIS_IS_APPLE 
// we originally named it like that, even though we should probably stay with "APPLE" only
#endif

