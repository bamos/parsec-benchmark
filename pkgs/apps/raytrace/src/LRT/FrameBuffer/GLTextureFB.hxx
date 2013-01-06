#ifndef LRT__GLTEXTURE_FB_HXX
#define LRT__GLTEXTURE_FB_HXX

#include "../FrameBuffer.hxx"
#include <GL/gl.h>


namespace LRT BEGIN_NAMESPACE

/*! framebuffer that uses an opengl texture for display. this is the
  base class that handles texture allocation, resizing, and display,
  but *not* the uploading (the latter has to be done in subclasses
  that use either glTex(Sub)Image or any other method to upload. this
  particular class */
struct OpenGLTextureRGBA8FB : public RGBAucharFrameBuffer
{
//   static const int FRAME_BUFFER_MODE = GL_RGBA;
  static const int FRAME_BUFFER_MODE = GL_BGRA;

  GLuint fbTextureID;
  vec2i fbTextureRes;
  vec2f fbTexCoords;

  /*! make sure to call resize at least one before accessing the texture ...

    \note might want to pass an initializer for resx,resy to the parent class
  */
  OpenGLTextureRGBA8FB()
  {
    glGenTextures(1, &fbTextureID);
    glBindTexture(GL_TEXTURE_2D, fbTextureID);
    fbTextureRes = vec2i(0,0);
  }

  virtual void resize(int newX, int newY);
  virtual void startNewFrame() { /* do nothing */ };
  virtual void doneWithFrame() = 0; /* *MUST* be implemented by
                       *subclass, becuase this base
                       *class does not handle
                       *uploading the pixels itself */

  /*! display the pixels as a window-aligned textured quad. note that
    this is *not* actually what an application might want -- it might
    want to have the pixels uploaded to a texture, but to place a quad
    with that texture to a totally different position or orientation
    (e.g., the app might have three or four different 'frame buffers'
    that cover different parts of the full viewport). let's not worry
    about that, yet. */
  virtual void display();
};


/*! opengl texture for display, but local buffer that will be uploaded
  at the very end via gltex(sub)image */
struct BufferedOpenGLTextureRGBA8FB : public OpenGLTextureRGBA8FB
{
  int allocedSize;

  BufferedOpenGLTextureRGBA8FB() : OpenGLTextureRGBA8FB(), allocedSize(0)
  {}
  virtual void resize(int newX, int newY);
  virtual void doneWithFrame();
};


END_NAMESPACE


#endif
