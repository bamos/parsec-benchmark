#ifndef RTL__TILED_FRAMEBUFFER_HXX
#define RTL__TILED_FRAMEBUFFER_HXX

#include "../FrameBuffer.hxx"

namespace LRT BEGIN_NAMESPACE

// =======================================================
/*! a tiled frame buffer. note that this does not necessarily mean we
    are doing any kind of load balancing here (i.e., there's not
    necessarily a fixed number of tiles available, nor does any tile
    implicitly or explicitly correspond to any region of the frame
    buffer), but that this is solely a ways for a client to request a
    buffer of pixels that it can then, lateron, return to the frame
    buffer for (async?) uploading (via \see returnTile)
*/
struct TiledBF : public FrameBuffer
{
  /*! size of screen tiles. should be a power of two. most
    implementations will also require internal FB resolitions to be a
    mulitple of tile size */
  static int tileSize;

  typedef int Handle;
  typedef RTVec2i vec2i;

  struct ClientData
  {
    sse_i *buffer[2];
    int currentBufferID;
    Handle handle;
  };
  typedef sse_i Tile;

  /*! called by the master thread. prepare rendering a new frame. do
    all necessary initializations etc */
  virtual void startNewFrame();

  /*! called by the master thread. do whatever is necessary to finish
    the thread (e.g., upload pixels to gpu, sync PBOs, whatever */
  virtual void doneWithFrame();


  /*! called by a pixel produced (i.e., a render client). register
    this new pixel producer, give it a handle by which it can request
    new tiles via \see requestNewTile. */
  virtual Handle registerNewClient();
  /*! request a new "tile" of pixels to write to. note that this does
    not necessarily mean we are doing any kind of load balancing here
    (i.e., there's not necessarily a fixed number of tiles available,
    nor does any tile implicitly or explicitly correspond to any
    region of the frame buffer), but that this is solely a ways for a
    client to request a buffer of pixels that it can then, lateron,
    return to the frame buffer for (async?) uploading (via \see returnTile)*/
  virtual Tile *requestNewTile(const Handle &handle);

  /*! return a tile of pixels as requested by \see requestNewTile.

  \param pos start coordinate of tile in frame buffer

  \param size size of tile (width x height) of tile
  */
  virtual void returnTile(Tile *tile, vec2i pos, vec2i size);
};


// =======================================================
/*! frame buffer for multiple 'clients', i.e., for multiple threads
  producing pixels at the same tile. has the be thread-safe, of
  course.
*/


// =======================================================
/*! a tiled framebuffer writing to an opengl texture. tiles are
  uploaded immediately via gltexsubimage. not thread-safe. */
struct SingleThreadedTiledGLFB : public MultiClientFB
{
  virtual void startNewFrame()

  glBindTexture(GL_TEXTURE_2D, fbTextureID);
  glEnable(GL_TEXTURE_2D);
  // in this buffered version, upload all pixels in one block at the
  // end -- horrible scalability, though; your entire app will be
  // blocked while uploading ...
  glTexSubImage2D(GL_TEXTURE_2D,0,
          0,0,res.x,res.y,
          FRAME_BUFFER_MODE,
          GL_UNSIGNED_BYTE,
          (unsigned char *)fb);

};

END_NAMESPACE

#endif
