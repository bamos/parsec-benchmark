#ifndef LRT__FRAMEBUFFER_HXX
#define LRT__FRAMEBUFFER_HXX

#define DBG_DISPLAY(a)
// #define DBG_DISPLAY(a) a

#include "RTTL/common/RTInclude.hxx"
#include "RTTL/common/RTVec.hxx"
#define GL_GLEXT_PROTOTYPES
#ifdef THIS_IS_APPLE
#include <glut.h>
#else
#include <GL/glut.h>
#endif
#if defined(_WIN32)
extern "C" {
#include "glext_win.h"
}
#else
#if !defined(sun) && !defined(__sun) && !defined(__sun__)
#include "GL/glext.h"
#endif
#endif


#define BEGIN_NAMESPACE {
#define END_NAMESPACE }

using namespace std;

namespace LRT BEGIN_NAMESPACE

  typedef RTTL::RTVec2i vec2i;
  typedef RTTL::RTVec2f vec2f;

/*! abstract base class of a frame buffer. plain old array of pixels,
  RGBA, RGBA, RGBA, etc. No Zordering or anything like that. note that
  this class just provides an interface for resizing the buffer and
  functionality for fast writing to it. memory allocation will be done
  in the base classes (mapping fb to a PBO, or allocating it,
  depending on implementation)

  \note to allow fast writing, we currently force frame buffer res to
  be multiple of 4 in each direction
*/
struct RGBAucharFrameBuffer
{
  /*! class that encapsulates all framebuffer related options */
  struct Options {
    /*! if set to true, FrameBuffer::create() will try to allocate a
      PBO framebuffer. This may still fail (e.g., if PBO support not
      comiled in, or if HW doesnt support i), but at least it will
      try. If set to false, we're not even trying, and will directly
      revert toa non-PBO framebuffer */
    static bool usePBOs;

    /*! if set to true, FrameBuffer::create() will use a linear region
        in main memory as framebuffer, mostly useful for debugging or
        no display rendering */
    static bool useMemoryFB;

    /*! default resolution of frame buffer. to be used by the
      application ... */
    static vec2i defaultRes;
  };

  vec2i res; /*! resultion (width x height) of frame buffer */
  unsigned char *fb;

  RGBAucharFrameBuffer()
    : res(0,0), fb(NULL)
  {}
  /*! write a block of pixels. */
  _INLINE void writeBlock(const int x0, const int y0,
             const int dx, const int dy,
             const sse_i *four4x8PixelsEach) const;

  _INLINE void prefetchBlock(const int x0, const int y0,
			     const int dx, const int dy) const;
			     

  static RGBAucharFrameBuffer *create();


  /*! resize the frame buffer.

  \note the 'externalFBptr' doesn't make sense in the abstract base
  class, but I don't want to think too hard about any cleaner concepts
  right now ... */
  virtual void resize(int newX, int newY)
  {
    // framebuffer must be at least 32x32, and at least a multiple of
    // 4 in each dimension ... (else can't write in SIMD ...)
    newX = max(32,nextMultipleOf<4>(newX));
    newY = max(32,nextMultipleOf<4>(newY));

    vec2i newRes(newX,newY);
    if (res != newRes) {
      res = newRes;
    }
  };

  /* re-allocate fb pointer. */
  virtual void startNewFrame() = 0;
  virtual void doneWithFrame() = 0;
  virtual void display() = 0;
};


/*! just easier to type... */
typedef RGBAucharFrameBuffer FrameBuffer;



// =======================================================
// =======================================================
// =======================================================
// =======================================================
// IMPLEMENTATION:
// =======================================================
// =======================================================
// =======================================================
// =======================================================


/*! write a rectangular block of pixels into frame buffer

will be the same for both PBO and non-pbo version, just what 'fb'
points to will be different ...
*/
_INLINE void RGBAucharFrameBuffer::writeBlock(const int x0, const int y0,
					     const int dx, const int dy,
					     const sse_i *four4x8PixelsEach)
  const
{
  //   assert(fb);
  //   assert(is_aligned<16>(fb));
  //   assert(is_divisible<4>(dx));
  //   assert(is_divisible<4>(dy));

  unsigned int *const fb_as_int32 = (unsigned int *)fb;
#if 1

  //     unsigned int *const RESTRICT fb_as_int32 = (unsigned int *)fb;
  //     const unsigned int *const RESTRICT fb_as_int32 = (unsigned int *)fb;

  if (__builtin_expect(dx == 8 && dy == 8,1))
    {
      unsigned int *start = (unsigned int*)&fb_as_int32[y0*res.x+x0];
#pragma unroll(8)
      for (int y=0;y<8;y++,start+=res.x)
	{
	  //_mm_stream_si128((sse_i*)&start[0],four4x8PixelsEach[y*2+0]);
	  //_mm_stream_si128((sse_i*)&start[4],four4x8PixelsEach[y*2+1]);
	  _mm_stream_ps((float*)&start[0],cast(four4x8PixelsEach[y*2+0]));
	  _mm_stream_ps((float*)&start[4],cast(four4x8PixelsEach[y*2+1]));

	  //_mm_prefetch((char*)(start+res.x),_MM_HINT_NTA);
	}
      return;
    }
  else
      cout << dx << " " << dy << endl;
#endif
#if 0
  if (dx & 0xf)
    {
      // packet is not a multiple of 16 wide.... copy 4 pixels at a time...
      for (int y=y0; y<y0+dy; y++)
	{

	  float *const line = (float *)&fb_as_int32[y*res.x];
	  for (int x=x0; x<x0+dx; x+=4)
	    {
	      _mm_stream_ps(&line[x],cast(*four4x8PixelsEach++));
	    }
	}
    }
  else
    {
      for (int y=y0; y<y0+dy; y++)
	{
	  float *const line = (float *)&fb_as_int32[y*res.x];

	  for (int x=x0; x<x0+dx; x+=16)
	    {
	      _mm_stream_ps(&line[x+ 0],cast(*four4x8PixelsEach++));
	      _mm_stream_ps(&line[x+ 4],cast(*four4x8PixelsEach++));
	      _mm_stream_ps(&line[x+ 8],cast(*four4x8PixelsEach++));
	      _mm_stream_ps(&line[x+12],cast(*four4x8PixelsEach++));
	    }
	}
    }
#endif
}

_INLINE void RGBAucharFrameBuffer::prefetchBlock(const int x0, const int y0,
						 const int dx, const int dy)
  const
{
  unsigned int *const fb_as_int32 = (unsigned int *)fb;
  if (__builtin_expect(dx == 8 && dy == 8,1))
    {
      unsigned int *start = (unsigned int*)&fb_as_int32[y0*res.x+x0];
#pragma unroll(8)
      //for (int y=0;y<8;y++,start+=res.x) _mm_prefetch((char*)start,_MM_HINT_NTA);
      return;
    }
}



END_NAMESPACE

#endif

