/* Copyright (c) 2006-2007 by Princeton University
 * All rights reserved.
 * Author: Christian Bienia
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Princeton University nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY PRINCETON UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL PRINCETON UNIVERSITY BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file config.h
 * \brief The configuration file for PARSEC Hooks Instrumentation API.
 *
 * This file serves as a central point to enable or disable functionality of
 * the hooks library functions. To enable functionality simply define the
 * corresponding macro to be 1 and 0 otherwise.
 *
 * The Hooks API is specified in file hooks.h and the functionality is
 * implemented in file hooks.c.
 */

#ifndef _PARSEC_HOOKS_CONFIG_H
/** Guard macro to prevent multiple inclusions. */
#define _PARSEC_HOOKS_CONFIG_H 1



/*** Configuration ***/

/** \brief Measure execution time of Region-of-Interest.
 *
 * If this macro is defined to 1 code is added to the hook functions to
 * measure the execution time of the Region-of-Interest. Measuring execution
 * time of the whole program is inaccurate because it includes the
 * initialization and shutdown phase.
 *
 * This functionality is enabled by default.
 */
#define ENABLE_TIMING 1

/* Affinity control at the command line. */
#if defined(__linux__) || defined(__linux) || defined(linux__) || defined(linux)
/** \brief Enable process affinity control via environment variables
 *
 * If this macro is defined to 1 code is added to the hook functions which
 * allows thread affinity control of all PARSEC workloads via two environment 
 * variables. This functionality can be used to restrict which CPUs the
 * workloads can use at runtime. At the moment thread affinity control is only
 * supported on Linux systems.
 * 
 * The name of the environment variables is determined by the macros 
 * __PARSEC_CPU_NUM and __PARSEC_CPU_BASE.
 *
 * This functionality is enabled by default on Linux systems.
 */
//NOTE: Requires sched_setaffinity() (Linux only)
#define ENABLE_SETAFFINITY 1
/** \brief Name of environment variable to control number of CPUs.
 *
 * This macro defines the name of one of the two environment variables which
 * can be used for thread affinity control at the command line. It determines
 * the total number of CPUs to use. The base CPU can be set with the environment
 * variable defined by __PARSEC_CPU_BASE.
 *
 * Define the macro ENABLE_SETAFFINITY to enable this feature.
 */
#define __PARSEC_CPU_NUM "PARSEC_CPU_NUM"
/** \brief Name of environment variable to set base CPU.
 *
 * This macro defines the name of one of the two environment variables which
 * can be used for thread affinity control at the command line. It determines
 * the base CPU to use. The total number of CPUs can be set with the environment
 * variable defined by __PARSEC_CPU_NUM.
 *
 * Define the macro ENABLE_SETAFFINITY to enable this feature.
 */
#define __PARSEC_CPU_BASE "PARSEC_CPU_BASE"
#endif //__LINUX__

/** \brief Execute SIMICS magic instruction at beginning and end of
 * Region-of-Interest.
 *
 * If this macro is defined to 1 a `magic instruction' recognized by the Simics
 * simulator is inserted at the beginning and end of the Region-of-Interest.
 * It can be used to interrupt simulation at these locations. To use this
 * feature you need to make sure that the correct Simics header file is included
 * in file config.h
 *
 * This functionality is disabled by default.
 */
#define ENABLE_SIMICS_MAGIC 0
/* Path to Simics directory, if you want to use its magic instructions */
#if ENABLE_SIMICS_MAGIC
#include "/opt/virtutech/simics-latest/src/include/simics/magic-instruction.h"
#endif

/** \brief Execute PTLsim trigger calls at beginning and end of
 * Region-of-Interest.
 *
 * If this macro is defined to 1 the PTLsim trigger functions are called
 * at the beginning and end of the Region-of-Interest. The hooks library will
 * execute ptlcall_switch_to_sim() at the beginning of the Region-of-Interest
 * and ptlcall_switch_to_native() at its end.
 *
 * PTLsim uses these functions to switch between simulation and native
 * execution. To use this feature you need to make sure that the correct
 * PTLsim header file is included in file config.h
 *
 * This functionality is disabled by default.
 */
#define ENABLE_PTLSIM_TRIGGER 0
/* Path to PTLsim directory, if you want to use its trigger functions */
#if ENABLE_PTLSIM_TRIGGER
#include "/opt/PTLsim/ptlcalls.h"
#endif



/** \brief Prefix for all output.
 *
 * This macro defines the prefix to use for all output generated by the hooks
 * library. A descriptive prefix makes it easier to distinguish hooks library
 * output from program output at runtime.
 *
 * By default the string "[HOOKS]" is used.
 */
#define HOOKS_PREFIX "[HOOKS]"

/** \brief Version number of this implementation of the PARSEC hooks
 *
 * This macro defines a version string which is part of the output that is
 * printed by the hooks library at the beginning of program execution if the
 * PARSEC workload was compiled with hook support enabled. A unique version
 * string makes it easier to distinguish which version of the hooks library the
 * program has been linked to.
 */
#define HOOKS_VERSION "1.2"

#endif //_PARSEC_HOOKS_CONFIG_H

