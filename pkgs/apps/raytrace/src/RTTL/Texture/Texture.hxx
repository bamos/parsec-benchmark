/*! \file Mesh.hxx defines basic mesh containers for storing geometry ... */

#ifndef RTTL_TEXTURE_HXX
#define RTTL_TEXTURE_HXX

#include "../common/RTInclude.hxx"
#include "../common/RTVec.hxx"

#include <vector>
#include <map>


namespace RTTL {

  enum {
    RT_TEXTURE_FORMAT_RGB_UCHAR,
    RT_TEXTURE_FORMAT_RGBA_UCHAR,
    RT_TEXTURE_FORMAT_RGBA_FLOAT
  };

  static const sse_f sse_one = _mm_set_ps1(1.0f);
  static const sse_f sse_scale = _mm_set_ps1(1.f/255.f);
    
  template <int CHANNELS,typename DataType>    
  class RTTextureObject {
  public:

    typedef DataType Texel[CHANNELS];
    
    RTTextureObject(const int sizeX, const int sizeY) : m_sizeX(sizeX), m_sizeY(sizeY), m_fsizeX((float)sizeX), m_fsizeY((float)sizeY)
    {
      m_textureMem = aligned_malloc<Texel>(m_sizeX*m_sizeY);
    }

    ~RTTextureObject()
    {
      if(m_textureMem)
          aligned_free(m_textureMem);
    }

    template<int COMPONENT>
    _INLINE void getTexel(sse_f tu, sse_f tv, RTVec_t<CHANNELS, sse_f> &t)
    {
    }


    _INLINE Texel *getTexelPtr() const
    {
      return m_textureMem;
    }

  private:
    int m_sizeX;
    int m_sizeY;
    int m_fsizeX;
    int m_fsizeY;
    Texel *m_textureMem;
  };

  typedef RTTextureObject<4,unsigned char> RTTextureObject_RGBA_UCHAR;
  typedef RTTextureObject<4,float> RTTextureObject_RGBA_FLOAT;

};

#endif
