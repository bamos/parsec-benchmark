#ifndef RTTL__PBO_FRAMEBUFFER_HXX
#define RTTL__PBO_FRAMEBUFFER_HXX

#include "GLTextureFB.hxx"

namespace LRT BEGIN_NAMESPACE


/*! same as parent class (\see OpenGLTextureRGBA8FB), except that we
  do not allocate pixel memory, but actually map it to driver
  mem ...*/
struct PBOFrameBuffer : public OpenGLTextureRGBA8FB
{
  /*! try allocating one -- if that fails, return NULL */
  static PBOFrameBuffer *create();

  GLuint fbPBO;

  PBOFrameBuffer();
  virtual void resize(int newX, int newY);

  virtual void startNewFrame();
  virtual void doneWithFrame();
};

END_NAMESPACE

#endif

