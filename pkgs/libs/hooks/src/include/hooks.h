/* Copyright (c) 2006-2008 by Princeton University
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
 * THIS SOFTWARE IS PROVIDED BY CHRISTIAN BIENIA ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL CHRISTIAN BIENIA BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file hooks.h
 * \brief The specification of the PARSEC Hooks Instrumentation API
 *
 * This file specifies the PARSEC Hooks Instrumentation API. If enabled each
 * PARSEC workloads calls the hooks library functions at certain predetermined
 * locations. These functions can be used to plug in any type of functionality
 * at the designated locations of each PARSEC workload. Some preliminary
 * functionality is already provided which can be enabled or disabled in the
 * config.h file of the hooks library before compilation.
 *
 * Benchmark programs typically call the hook functions as follows:
 *
 * \code
 * int main() {
 *   __parsec_bench_begin(__parsec_example);
 *   do_init();
 *
 *   __parsec_roi_begin();
 *   //Begin of parallel phase
 *   for(int i=0; i<NTHREADS; i++)
 *     pthread_create(t[i], NULL, worker_func, NULL);
 *
 *   //Wait until all threads have terminated
 *   for(int i=0; i<NTHREADS; i++)
 *     pthread_join(t[i], NULL);
 *   //End of parallel phase
 *   __parsec_roi_end();
 *
 *   do_cleanup();
 *   __parsec_bench_end();
 *   return 0;
 * }
 * \endcode
 *
 * If you modify this file, you might also have to update all benchmark
 * programs of the PARSEC distribution. To change the behavior of the PARSEC
 * hooks, look at files config.h and hooks.c.
 */



#ifndef _PARSEC_HOOKS_HOOKS_H
/** Guard macro to prevent multiple inclusions. */
#define _PARSEC_HOOKS_HOOKS_H 1

#ifdef __cplusplus
extern "C" {
#endif

/*** Type declarations ***/

/** \brief Identifiers for the benchmark programs
 *
 * Each workload has exactly one unique identifier in this enumeration. The
 * benchmark passes it to __parsec_bench_begin() at the beginning of its
 * execution.
 */
enum __parsec_benchmark {
  __parsec_blackscholes,
  __parsec_bodytrack,
  __parsec_canneal,
  __parsec_dedup,
  __parsec_facesim,
  __parsec_ferret,
  __parsec_fluidanimate,
  __parsec_freqmine,
  __parsec_raytrace,
  __parsec_streamcluster,
  __parsec_swaptions,
  __parsec_vips,
  __parsec_x264,
  __splash2_barnes,
  __splash2_cholesky,
  __splash2_fft,
  __splash2_fmm,
  __splash2_lu_cb,
  __splash2_lu_ncb,
  __splash2_ocean_cp,
  __splash2_ocean_ncp,
  __splash2_radiosity,
  __splash2_radix,
  __splash2_raytrace,
  __splash2_volrend,
  __splash2_water_nsquared,
  __splash2_water_spatial
};



/*** Function declarations ***/

/** \brief Beginning of program execution.
 *
 * \param[in] __bench Unique workload identifier.
 *
 * This function is executed exactly once, as soon as the program starts.
 *
 * Its logical counterpart is __parsec_bench_end.
 */
void __parsec_bench_begin(enum __parsec_benchmark __bench);

/** \brief End of program execution.
 *
 * This function is executed exactly once, just before the program ends.
 *
 * Its logical counterpart is __parsec_bench_begin.
 */
void __parsec_bench_end();

/** \brief Beginning of Region-of-Interest.
 *
 * This function is executed exactly once, just before the Region-of-Interest
 * (ROI) is entered. The ROI is the part of the code that should be used for
 * benchmarking and analysis. It contains the entire parallel phase of the
 * program.
 *
 * The logical counterpart of this function is __parsec_roi_end.
 */
void __parsec_roi_begin();

/** \brief End of Region-of-Interest.
 *
 * This function is executed exactly once, immediately after the
 * Region-of-Interest (ROI) is left. The ROI is the part of the code that
 * should be used for benchmarking and analysis. It contains the entire parallel
 * phase of the program.
 *
 * The logical counterpart of this function is __parsec_roi_begin.
 */
void __parsec_roi_end();

#ifdef __cplusplus
} // extern "C"
#endif

#endif //_PARSEC_HOOKS_HOOKS_H

