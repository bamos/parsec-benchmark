#ifndef TIMER_HXX
#define TIMER_HXX

#include "RTInclude.hxx"

#if defined(__GNUC__)
#include <unistd.h>
#include <sys/time.h>
#endif

// See http://www.informit.com/guides/content.asp?g=dotnet&seqNum=474&rl=1

struct Timer
{
public:
    Timer() {
#if !defined(__GNUC__)
        // icc or VS on Windows machine.
        LARGE_INTEGER CPUFrequency;
        QueryPerformanceFrequency(&CPUFrequency);
        _tfreqi = 1.0 / (double)CPUFrequency.QuadPart;
        _toverhead = 0;
        start();
        _toverhead = cycles();
#endif
    }

  _INLINE void start() {
#if defined(__GNUC__)
      // All compilers on Linux machine.
      gettimeofday(&t1, NULL);
#else
#if 0
      static int CPUInfo[4] = {-1};
      __cpuid(CPUInfo, 0); // serialization
      _tstart = __rdtsc();
#else
      LARGE_INTEGER count;
      QueryPerformanceCounter(&count);
      _tstart = count.QuadPart;
#endif
#endif
  }

  _INLINE unsigned long long cycles() const {
#if defined(__GNUC__)
      struct timeval t2;
      gettimeofday(&t2, NULL);
      return 1000000 * (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec);
#else
#if 0
      static int CPUInfo[4] = {-1};
      __cpuid(CPUInfo, 0); // serialization
      unsigned long long _tend = __rdtsc();
#else
      LARGE_INTEGER count;
      QueryPerformanceCounter(&count);
      unsigned long long _tend = count.QuadPart;
#endif
      return _tend - _tstart - _toverhead;
#endif
  }

_INLINE float seconds() const {
    return stop();
}

  _INLINE float stop() const {
#if defined(__GNUC__)
      struct timeval t2;
      gettimeofday(&t2, NULL);
//       return (float)(((double)t2.tv_sec - t1.tv_sec) + ((double)t2.tv_usec - t1.tv_usec) / 1000000.0);
      return (float)((t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000000.f);
#else
      return (float)(((double)(cycles())) *_tfreqi);
#endif
  }

private:
#if defined(__GNUC__)
  struct timeval t1;
#else
    unsigned long long _tstart, _toverhead;
    double _tfreqi;
#endif

};
#endif
