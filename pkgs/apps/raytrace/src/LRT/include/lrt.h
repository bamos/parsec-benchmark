#ifndef LRT__LRT_H
#define LRT__LRT_H

#include "RTTL/API/rt.h"
#include <stdlib.h>
#include <sys/types.h>

extern "C" {
  /*! \file lrt.h main header file for the low-level garfield api. 

    @ingroup lrt_api */


  /**  @ingroup lrt_api */
  /*@{*/

  typedef void LRTvoid;
  typedef int LRTint;
  typedef float RTfloat;
  typedef float LRTfloat;
  typedef unsigned int LRTuint;
  typedef unsigned int LRThandle;
  typedef void *LRTFrameBufferHandle; /* i'm lazy ... this is supposed ot be a LRThandle, but I don't want to do the int-to-pointer mapping right now */
  typedef void *LRTCamera;

  /*! eventually, that'll store a rendering context (what rendering
    algorithm is specified, which accel structures to use, what
    tessellation depth, whatever -- right now, ignore it */
  typedef void *LRTContext; 

  /*! a camera type */
  typedef void *RTCamera;

  typedef enum {
    LRT_UCHAR_RGBA = 0
  } LRTFrameBufferFormat;


  /*@}*/


  /**  @ingroup lrt_api */
  /*@{*/

  /*! initialize the ray tracer. takes all LRT-related command line
    arguments out of the list passed to it. \note will also parse the environment */
  void lrtInit(int *argc, char **argv);


  // =======================================================
  /*! create a frame buffer that uses an OpenGL texture to store the
    pixels (no other data is stored).  See \ref lrtDestroyFB and \ref
    lrtDisplayFB for how to destroy and display such a frame
    buffer. resizing right now happens via "destoy+new" ...

    \note if specified on the command line (or in the env), this call
    may try to use a PBO frame buffer

  */
  LRTFrameBufferHandle  lrtCreateTextureFB(LRTuint width,
                                           LRTuint height);

  /*! display the frame buffer via a OpenGL 2D quad with coordinates
    [0,0],[1,1]. it's the app's responsiblity to correctly
    position/align this quad on the screen, and to use the correct
    aspect ratio ... 
  
    \param fbHandle must be a valid frame buffer handle as, for
    example, returned via lrtCreateTextureFB
  */
  LRTvoid  
  lrtDisplayFB(LRTFrameBufferHandle fbHandle);

  /*!destroy a frame buffer */
  LRTvoid   
  lrtDestroyFB(LRTFrameBufferHandle fbHandle);



  /*!creates a rendering context*/
  LRTContext lrtCreateContext();

  /*!destroys a rendering context*/
  LRTvoid lrtDestroyContext(LRTContext context);

  /*!creates a rendering context*/
  LRTCamera lrtCreateCamera();

  /*!destroys a rendering context*/
  LRTvoid lrtDestroyCamera(LRTCamera camera);

  /*!sets number of rendering threads in given context*/
  LRTvoid lrtSetRenderThreads(LRTContext context,LRTuint threads);

  /*!Build a context, has to be called before lrtRenderFrame*/
  LRTvoid lrtBuildContext(LRTContext context);

  /*!  render a frame into a frame buffer object. use camera and
    context as specified; if any of those is NULL, we'll use the
    default camera resp default context instead */
  LRTvoid lrtRenderFrame(LRTFrameBufferHandle fb,
                         LRTContext context, 
                         LRTCamera camera
                         );
  
  /*! set camera to be at point 'eye', look at point 'center' and ues 'up' as an upvector */
  LRTvoid  lrtLookAt(LRTCamera camera, 
		     RTfloat eyeX, RTfloat eyeY, RTfloat eyeZ, 
		     RTfloat centerX, RTfloat centerY, RTfloat centerZ, 
		     RTfloat upX, RTfloat upY, RTfloat upZ,
		     RTfloat angle, RTfloat aspect);

} // extern C

/*@{*/


#endif
