#include "include/lrt.h"
#include "FrameBuffer/GLTextureFB.hxx"

namespace LRT
{
#define LRTAPIENTRY

  struct TileRenderer
  {
    /*! tile size used for load balancing. screen gets subdivided into
      tiles of tileSize x tileSize pixels, and threads then
      dynamically load balance on these tiles */
    int tileSize;
  };

  LRTint   LRTAPIENTRY
  lrtRenderToTexture(LRTuint width,
             LRTuint height,
             LRTFrameBufferFormat texture_format,
             LRTfloat *texcoord_u,
             LRTfloat *texcoord_v
             )
  {
    // let's hard-code to RGBA uchar texture format for now. lateron,
    // we should have a virtual base class (with 'storeTile()' etc,
    // and a way of switching the format if the app requires
    // that. (note: still not (fully) sure how to handle frame buffers
    // with more than rgb(a) in them, though (i.e., ID buffer, depth
    // buffer, etc)
//     GLTextureFB<RGBAuchar> *glTextureFB
//       = GLTextureFB<RGBAuchar>::get();

//     int textureID = glTextureFB.allocTexture(&texCoord_u,&texCoord_v);

    static GLuint textureID = (GLuint)-1;
    static vec2i textureRes(-1,-1);
    static vec2f texCoords;

    if (textureID == (GLuint)-1 || (LRTuint)textureRes.x < width || (LRTuint)textureRes.y < height)
      {
    if (textureID == (GLuint)-1)
      glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    textureRes = vec2i(1,1);
    while ((LRTuint)textureRes.x < width)  textureRes.x *= 2;
    while ((LRTuint)textureRes.y < height) textureRes.y *= 2;

    cout << "new framebuffer texture res " << textureRes << endl;
    glEnable(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D,
             0,
             GL_RGBA,
//               GL_BGRA,
             textureRes.x,
             textureRes.y,
             0,
//              GL_BGRA,
             GL_RGBA,
             GL_UNSIGNED_BYTE,
             NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // enabling this texture unit
    glEnable(GL_TEXTURE_2D);
      }

    // RENDER HERE !!!!
    {
      const int tileSize = 32;
      unsigned char tile[tileSize][tileSize][4];

      for (int i=0;i<tileSize;i++)
    for (int j=0;j<tileSize;j++)
      {
        tile[i][j][3] = 255;
        tile[i][j][2] = 255 * i / tileSize;
        tile[i][j][1] = 255 * j / tileSize;
        tile[i][j][0] = 255;
      }
      for (int ii=0;ii<(int)width;ii+=tileSize)
    for (int jj=0;jj<(int)height;jj+=tileSize)
      {
        glBindTexture(GL_TEXTURE_2D, textureID);

        glEnable(GL_TEXTURE_2D);
        glTexSubImage2D(GL_TEXTURE_2D,0,
                ii,jj,
                tileSize,tileSize,
//                 GL_BGRA,
                  GL_RGBA,
                GL_UNSIGNED_BYTE,
                (unsigned char *)tile);

      }
    }

    texCoords.x = width / float(textureRes.x);
    texCoords.y = height / float(textureRes.y);
    *texcoord_u = texCoords.x;
    *texcoord_v = texCoords.y;
    return textureID;
  };

};
