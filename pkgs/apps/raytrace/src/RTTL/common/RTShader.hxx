#ifndef RTTL_SHADER_HXX
#define RTTL_SHADER_HXX

#include "RTInclude.hxx"
#include "RTVec.hxx"

namespace RTTL {
    
  /* simple class to have something which is capable of handling
     material properties, structure need to be properly aligned for
     fast access */

  class RTMaterial {
  public:
    RTVec3f  m_diffuse;
    float    m_transparency;
    RTVec3f  m_specular;
    float    m_shininess;
    RTVec3f  m_ambient;
    int      m_textureId;

    RTMaterial() {      
        m_diffuse = RTVec3f(0.5f,0.5f,0.5f);
        m_transparency = 0.0f;
        m_specular = RTVec3f(0.0f,0.0f,0.0f);
        m_shininess = 0.0f;
        m_ambient = RTVec3f(0.0f,0.0f,0.0f);
        m_textureId = -1;
    }
    _INLINE sse_f& ambient() const { return *(sse_f*)&m_ambient; }
    _INLINE sse_f& diffuse() const { return *(sse_f*)&m_diffuse; }
    _INLINE sse_f& specular() const { return *(sse_f*)&m_specular; }

    _INLINE static void getDiffuse(const sse_i id4, const RTMaterial *const mat, RTVec_t<3, sse_f> &dest)
    {
      //DBG_PRINT(id4);
      const RTMaterial &mat0 = mat[M128_INT(id4,0)];
      const RTMaterial &mat1 = mat[M128_INT(id4,1)];
      const RTMaterial &mat2 = mat[M128_INT(id4,2)];
      const RTMaterial &mat3 = mat[M128_INT(id4,3)];
      //DBG_PRINT(mat0.m_diffuse);
      //DBG_PRINT(mat1.m_diffuse);
      //DBG_PRINT(mat2.m_diffuse);
      //DBG_PRINT(mat3.m_diffuse);
      dest[0] = _mm_setr_ps(mat0.m_diffuse[0],mat1.m_diffuse[0],mat2.m_diffuse[0],mat3.m_diffuse[0]);
      dest[1] = _mm_setr_ps(mat0.m_diffuse[1],mat1.m_diffuse[1],mat2.m_diffuse[1],mat3.m_diffuse[1]);
      dest[2] = _mm_setr_ps(mat0.m_diffuse[2],mat1.m_diffuse[2],mat2.m_diffuse[2],mat3.m_diffuse[2]);
      //DBG_PRINT(dest);
    }
  };
    
};

#endif
