#ifndef RTL__MEMORY_FRAMEBUFFER_HXX
#define RTL__MEMORY_FRAMEBUFFER_HXX

#include "../FrameBuffer.hxx"

namespace LRT BEGIN_NAMESPACE

// =======================================================
/*! simply using a continous region in memory as framebuffer,
    mostly useful for debugging and no display rendering
*/
struct MemoryFrameBuffer : public FrameBuffer
{

  /*! try allocating one -- if that fails, return NULL */
  static MemoryFrameBuffer *create() {
    return new MemoryFrameBuffer;
  }

  MemoryFrameBuffer() {
    fb = NULL;
  }
  virtual void resize(int newX, int newY)
  {
    FrameBuffer::resize(newX,newY);
    if (fb) aligned_free(fb);
    fb = aligned_malloc<unsigned char>(4*res.x*res.y);
  }

  virtual void startNewFrame() {}
  virtual void doneWithFrame() {}
  virtual void display() {}
};

END_NAMESPACE

#endif
