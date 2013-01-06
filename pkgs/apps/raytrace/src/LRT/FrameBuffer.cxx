#include "LRT/include/lrt.h"
#include "FrameBuffer.hxx"
#include "FrameBuffer/PBOFrameBuffer.hxx"
#include "FrameBuffer/GLTextureFB.hxx"
#include "FrameBuffer/MemoryFrameBuffer.hxx"

#define DBG(a) /* do nothing */

namespace LRT BEGIN_NAMESPACE

bool FrameBuffer::Options::usePBOs = true;
bool FrameBuffer::Options::useMemoryFB = false;
vec2i FrameBuffer::Options::defaultRes(512,512);



RGBAucharFrameBuffer *RGBAucharFrameBuffer::create()
{
  RGBAucharFrameBuffer *frameBuffer = NULL;

  if (Options::useMemoryFB) {
    cout << "Using memory framebuffer..." << endl;
    return new LRT::MemoryFrameBuffer;
  }

  if (Options::usePBOs)
    {
      // try allocating one ..
      frameBuffer = LRT::PBOFrameBuffer::create();
      if (frameBuffer)
	return frameBuffer;
      // something went wrong with a PBO framebuffer (inluding
      // having it turned off manually ...
      cout << "PBO: could not allocate PBO frame buffer." << endl;
      cout << "PBO: falling back to non-PBO framebuffer" << endl;
    }
  else
    cout << "using non-PBO framebuffer" << endl;

  frameBuffer = new LRT::BufferedOpenGLTextureRGBA8FB;

  if (frameBuffer == NULL)
    throw "panic: could not allocate _any_ kind of framebuffer ...";

  return frameBuffer;
}




END_NAMESPACE


using LRT::RGBAucharFrameBuffer;

LRTFrameBufferHandle lrtCreateTextureFB(LRTuint width,
                                        LRTuint height)
{
  LRT::FrameBuffer *fb = RGBAucharFrameBuffer::create();
  fb->resize(width,height);
  return fb;
}

LRTvoid lrtDestroyFB(LRTFrameBufferHandle fbHandle)
{
  RGBAucharFrameBuffer *fb = reinterpret_cast<RGBAucharFrameBuffer *>(fbHandle);
  if (fb) delete fb;
}
  
LRTvoid lrtDisplayFB(LRTFrameBufferHandle fbHandle)
{
  RGBAucharFrameBuffer *fb = reinterpret_cast<RGBAucharFrameBuffer *>(fbHandle);
  if (fb) fb->display();
}



