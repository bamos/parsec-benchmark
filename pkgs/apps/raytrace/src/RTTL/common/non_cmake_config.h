/*
\file config.h

this file should never be included directly, it should _only_ be
called by RTTL/common/RTInclude.hxx

we're adding a little define to make sure that happens...

\note cmake passes bools as ON/OFF text constants. to make that
understandable to the preprocessor we never use boolean cmake
variables directly, and instaed use "CMAKE_${VAR}", plus manually
defining CMAKE_ON=1 and CMAKE_OFF=0. If you get some unexpected
behavior of your cmake bools, check if you're always prefixing them
with "CMAKE_"...
*/


#ifndef CALLER_IS_COMMON_RTINCLUDE_HXX
# error "this file should only be called by RTInclude.hxx"
#endif

#ifdef THIS_IS_CMAKE
# error "non_cmake_config.h should _not_ be used when cmake is being used to build the code"
// note: when using cmake, we have cmake parse
// cmake_autoconfig_template.h, and have it replace all cmake
// variables with the cmake configured values, which are then written
// toa file called cmake_autoconfig.h thus, when using cmake, you
// should only be including cmake_autoconfig.h
#endif

// -------------------------------------------------------
#ifdef _WIN32
#define USE_PBOS 0
#else
#define USE_PBOS 1
/* #    define USE_PBOS 1 */
#endif

// -------------------------------------------------------
#define DO_STATS_BVH 0


// -------------------------------------------------------
static const char *defaultBVHBuilder = "default";
