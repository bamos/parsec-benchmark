/*! notes:

can test the following sceniarios:

a) single texture, uploaded in a single call via glTex(Sub)Image

b) one pbo, written to in rendering, and transferred via gltexsubimage

c) two pbos, one written to, the other one transferred via gltexsubimage


results so far:

- for a) glTex*Sub*Image seems to be significantly faster than
glTexImage even if all pixels are transferred once 'en bloc'

- regarding performance 'c' seems to *not* be any different than
  'b'. as it is more complicated, this suggests using 'b'. note
  however that some additional double- buffering may in fact be
  necessary, i.e., double-buffering the texture, too, potentially
  including asynchronous uploading (writing to one pbo, uploading the
  second one asynchronously). The latter would be quite complicated,
  though, and IMHO not worth the effort.

- 'b' and 'c' seem to be about 2x faster than 'a'.

- 'b' transfer rates on Clovertown 2.6GHz w/ NVidia card: ~375 fps
  (1024x1024, including pixel writing (mm_stream) to framebuffer array)
*/


#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#ifdef THIS_IS_APPLE
#include <glut.h>
#else
#include <GL/glut.h>
#endif
#include <iostream>
#include <sys/times.h>
#include <xmmintrin.h>

using namespace std;

#define DBG(a)

#define GLERROR { GLenum err = glGetError(); if (err != GL_NO_ERROR) cout << "GL Error " << __FILE__ << " " << __LINE__ << endl << gluErrorString(err) << flush; }

#define RES 1024

#define USE_PBOS // about 2x faster than w/o PBOs

//#define PBO_DOUBLE_BUFFER // not appreciably better than without, if at all.

#define FRAME_BUFFER_MODE GL_BGRA

unsigned char framebuffer[RES][RES][4];

int frame = 0;
GLuint textureID;

#ifdef PBO_DOUBLE_BUFFER
GLuint pbo[2];
#else
GLuint pbo[1];
#endif



#define NEED_ARB_WRAPPERS
#ifdef NEED_ARB_WRAPPERS
extern "C" {

#ifdef THIS_IS_APPLE
#  define APIENTRY /* */
#endif

#define ARB_NAME_WRAPPER(a) a##ARB

  GLAPI GLvoid* APIENTRY glMapBuffer(GLenum a, GLenum b)
  {
    return ARB_NAME_WRAPPER(glMapBuffer)(a,b);
  }
  GLAPI GLboolean APIENTRY glUnmapBuffer(GLenum a)
  {
    return ARB_NAME_WRAPPER(glUnmapBuffer)(a);
  }

  GLAPI void APIENTRY glBindBuffer(GLenum a, GLuint b)
  {
    ARB_NAME_WRAPPER(glBindBuffer)(a,b);
  }
  GLAPI void APIENTRY glGenBuffers(GLsizei a, GLuint *b)
  {
    ARB_NAME_WRAPPER(glGenBuffers)(a,b);
  }
  GLAPI void APIENTRY glBufferData(GLenum a, GLsizeiptr b, const GLvoid *c, GLenum d)
  {
    ARB_NAME_WRAPPER(glBufferData)(a,b,c,d);
  }
}
#endif



void initGL()
{
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RES,RES, 0,FRAME_BUFFER_MODE,
           GL_UNSIGNED_BYTE,  NULL);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

#ifdef USE_PBOS
  DBG(cout << "allocing pbos" << endl);
#  ifdef PBO_DOUBLE_BUFFER
  glGenBuffers(2, pbo);
  GLERROR;
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT,pbo[0]);
  GLERROR;
  glBufferData(GL_PIXEL_UNPACK_BUFFER_EXT, 4 * RES * RES, NULL, GL_STREAM_DRAW);
  GLERROR;
//   unsigned char *fb1
//     = (unsigned char *) glMapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
//   GLERROR;
//   glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT);
//   GLERROR;
//   glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT,0);
//   GLERROR;

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT,pbo[1]);
  GLERROR;
  glBufferData(GL_PIXEL_UNPACK_BUFFER_EXT, 4 * RES * RES, NULL, GL_STREAM_DRAW);
  GLERROR;
//   unsigned char *fb2
//     = (unsigned char *) glMapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
//   GLERROR;
//   glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT);
//   GLERROR;
//   glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT,0);
//   GLERROR;
#  else
  glGenBuffers(1, pbo);
  GLERROR;
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT,pbo[0]);
  GLERROR;
  glBufferData(GL_PIXEL_UNPACK_BUFFER_EXT, 4 * RES * RES, NULL, GL_STREAM_DRAW);
  GLERROR;
#  endif
#endif
}

void render()
{
#ifdef USE_PBOS
  DBG(cout << endl;
      cout << "render: bind texture" << endl);
  glBindTexture(GL_TEXTURE_2D, textureID);
  GLERROR;

  DBG(cout << "bind buffer" << endl);
#ifdef PBO_DOUBLE_BUFFER
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, pbo[frame % 2]);
  GLERROR;
#else
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, pbo[0]);
  GLERROR;
#endif

  DBG(cout << "map buffer" << endl);
  unsigned char *fb
    = (unsigned char *) glMapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
  GLERROR;
  if (fb == NULL)
    {
      cerr << "could not get pbo ... pbos probably not supported on this hw" << endl;
    }
#endif
  for (int y=0;y<RES;y++)
    {
      union {
    unsigned char pixel[4];
    float f_pixel;
      };
      pixel[0] = 255;
      pixel[1] = (y*256)/1024;
      pixel[2] = frame % 256;
      pixel[3] = 255;

      __m128 pix = _mm_set_ps1(f_pixel);

#ifdef USE_PBOS
      float *line = (float *)&fb[y*4*RES];
#else
      float *line = (float*)&framebuffer[y][0][0];
#endif

      for (int x=0;x<RES;x+=4)
    {
      _mm_stream_ps(&line[x],pix);

    }
    }

#ifdef USE_PBOS
  DBG(cout << "unmap" << endl);
  glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_EXT);
  GLERROR;

  DBG(cout << "binding texture" << endl);
  glBindTexture(GL_TEXTURE_2D, textureID);
  GLERROR;

  DBG(cout << "binding pbo" << endl);
#ifdef PBO_DOUBLE_BUFFER
//   if (frame < 3)
//     glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, pbo[frame%2]); // read from *old* buffer
//   else
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, pbo[1-(frame%2)]); // read from *old* buffer
  GLERROR;
#else
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, pbo[0]);
  GLERROR;
#endif

  DBG(cout << "uploading pixels" << endl);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, RES,RES,
           FRAME_BUFFER_MODE, GL_UNSIGNED_BYTE,0);
  GLERROR;
  DBG(cout << "binding 0" << endl);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
  GLERROR;
#else
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, RES,RES,
           FRAME_BUFFER_MODE, GL_UNSIGNED_BYTE,&framebuffer[0][0][0]);
#endif
}





#define NUM 100

void display(void)
{
  static long start_time;
  if (frame % NUM == 0)
    start_time = times(NULL);
  render();



  glBindTexture(GL_TEXTURE_2D, textureID);
  glEnable(GL_TEXTURE_2D);
  glShadeModel(GL_FLAT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glPolygonMode(GL_FRONT, GL_FILL);

  glMatrixMode(GL_PROJECTION);
  glViewport(0, 0, RES,RES);
  glLoadIdentity();
  glOrtho(0,1,0,1,-1,1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glBegin(GL_QUADS);
  {
    glTexCoord2f(0, 0);
    glVertex2f(0,0);

    glTexCoord2f(1, 0);
    glVertex2f(1,0);

    glTexCoord2f(1,1);
    glVertex2f(1,1);

    glTexCoord2f(0, 1);
    glVertex2f(0,1);
  }
  glEnd();

  static long end_time; // = times(NULL);
  if (frame % NUM == (NUM-1))
    {
      end_time = times(NULL);

      cout << "\rfps = " << (NUM / (float(end_time-start_time)*.01)) << flush;
    }
  glutSwapBuffers();
  glutPostRedisplay();
  ++frame;
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  glutInitWindowSize(RES,RES);
  glutInitWindowPosition(0, 0);
  glutCreateWindow("pbo test");
  glutDisplayFunc(display);

  initGL();

  glutMainLoop();
}
