#include "GLTextureFB.hxx"
#include "../include/lrt.h"

#define DBG(a) /* turned off */

namespace LRT BEGIN_NAMESPACE

/*! same as in parent class, except that we'll also make sure to
  resize the texture if necessary

*/
void OpenGLTextureRGBA8FB::resize(int newX, int newY)
{
  /* force framebuffer to be a multiple of 4x4 .... else can't
     reasonably write to it ... */
  RGBAucharFrameBuffer::resize(newX,newY);

  if (fbTextureRes.x < res.x || fbTextureRes.y < res.y)
    {
      fbTextureRes = vec2i(16,16);
      while (fbTextureRes.x < res.x)
    fbTextureRes.x *= 2;
      while (fbTextureRes.y < res.y)
    fbTextureRes.y *= 2;

      cout << "resizing to " << fbTextureRes << endl;
      glEnable(GL_TEXTURE_2D);
      glTexImage2D(GL_TEXTURE_2D,
           0,
           GL_RGBA,
           fbTextureRes.x,
           fbTextureRes.y,
           0,
           FRAME_BUFFER_MODE,
           GL_UNSIGNED_BYTE,
           NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
  fbTexCoords.x = res.x / float(fbTextureRes.x);
  fbTexCoords.y = res.y / float(fbTextureRes.y);
  DBG(PING; cout << res << " " << fbTextureRes << " " << fbTexCoords << endl);
}



/*! in this case, first upload all pixels via gltexsubimage, then
  display via a properly sized and textured quad.

  \note We're rendering a (0,0)-(1,1) quad, so make sure to
  correctly set the viewport etc
*/
void OpenGLTextureRGBA8FB::display()
{
  DBG(PING; cout << res << endl);
  glBindTexture(GL_TEXTURE_2D, fbTextureID);
  glEnable(GL_TEXTURE_2D);

  //   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glShadeModel(GL_FLAT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glPolygonMode(GL_FRONT, GL_FILL);
  //     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glColor4f(1, 1, 1, 1);

  glMatrixMode(GL_PROJECTION);
  glViewport(0, 0, (GLsizei)res.x, (GLsizei)res.y);
  glLoadIdentity();

  //     gluOrtho2D(0, 1, 0, 1);
  glOrtho(0,1,0,1,-1,1);
  //   glOrtho(0, w, 0, h, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();


  //   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  //    glDisable(GL_TEXTURE_2D);
  //     glColor4f(1, 0, 0, 1);

  glBegin(GL_QUADS);
  {
#if 0
    glTexCoord2f(0, 0);
    glVertex2f(0,0);

    glTexCoord2f(fbTexCoords.x, 0);
    glVertex2f(1,0);

    glTexCoord2f(fbTexCoords.x, fbTexCoords.y);
    glVertex2f(1,1);

    glTexCoord2f(0, fbTexCoords.y);
    glVertex2f(0,1);
#else
    glTexCoord2f(0, 0);
    glVertex2f(0,1);

    glTexCoord2f(fbTexCoords.x, 0);
    glVertex2f(1,1);

    glTexCoord2f(fbTexCoords.x, fbTexCoords.y);
    glVertex2f(1,0);

    glTexCoord2f(0, fbTexCoords.y);
    glVertex2f(0,0);
#endif
  }
  glEnd();
}





// -------------------------------------------------------
/*! resize to new dimensions. call parent class to resize the
  texture if required, then make sure we have a valid fb pointer
  pointing to enough pixel memory to write to

  \note: will automatically resize framebuffer to next multple of 16
  in each dimension
*/
void BufferedOpenGLTextureRGBA8FB::resize(int newX, int newY)
{
  OpenGLTextureRGBA8FB::resize(newX,newY);

  long size = nextMultipleOf<16*16*4>(res.x*res.y*4);
  if (size >= allocedSize) {
    if (fb) {
      cout << "freeing" << endl;
      aligned_free(fb);
    }
    cout << "resizing fb buffer to " << res << endl;
    cout << "alloc'ing " << size << " (" << res.x*res.y*4 <<")" << endl;
    fb = aligned_malloc<unsigned char>(size);
    // we are writing in either 4-wide or 16-wide blocks of pixels ...
    allocedSize = size;
    assert(is_aligned<16>(fb));
  }
}

/*! upload framebuffer to texture mem. may be slow (cpu-gpu
  bandwidth) */
void BufferedOpenGLTextureRGBA8FB::doneWithFrame()
{
  glBindTexture(GL_TEXTURE_2D, fbTextureID);
  glEnable(GL_TEXTURE_2D);
  // in this buffered version, upload all pixels in one block at the
  // end -- horrible scalability, though; your entire app will be
  // blocked while uploading ...
  DBG(PING);
  glTexSubImage2D(GL_TEXTURE_2D,0,
          0,0,res.x,res.y,
          FRAME_BUFFER_MODE,
          GL_UNSIGNED_BYTE,
          (unsigned char *)fb);
}



END_NAMESPACE
