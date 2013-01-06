#ifndef RTTL__MATRIX_HXX
#define RTTL__MATRIX_HXX

namespace RTTL {
#if 0
} /*! just to keep emacs happy w/ indenting ... */
#endif

class Matrix4x4
{
public:
  float mat[4][4];

  _INLINE float &operator()(const int i, const int j)
  { return mat[i][j]; };

  _INLINE RTVec4f getRow(int row) {
    RTVec4f r;
    r[0] = mat[row][0];
    r[1] = mat[row][1];
    r[2] = mat[row][2];
    r[3] = mat[row][3];
    return r;
  }

  _INLINE RTVec3f getCol(int col) {
    RTVec3f r;
    r[0] = mat[0][col];
    r[1] = mat[1][col];
    r[2] = mat[2][col];
    return r;
  }

  _INLINE void setCol(int col, const RTVec3f &r) {
    mat[0][col] = r[0];
    mat[1][col] = r[1];
    mat[2][col] = r[2];
  }

  _INLINE void setIdentity()
  {
    for (int i=0;i<4;i++)
      for (int j=0;j<4;j++)
    mat[i][j] = (i==j);
  }

  _INLINE void setTranslationPart(const RTVec3f &translation)
  {
    mat[0][3] = translation[0];
    mat[1][3] = translation[1];
    mat[2][3] = translation[2];
  }

  _INLINE void setRotationPart(const RTVec3f &axis, const float angle)
  {
    RTVec4f q;
    float ha = 0.5f*angle*M_PI/180.0f;
    float sine = sinf(ha);
    q[3] = cosf(ha);
    q[0] = sine*axis[0];
    q[1] = sine*axis[1];
    q[2] = sine*axis[2];

    float tx  = 2.0*q[0];
    float ty  = 2.0*q[1];
    float tz  = 2.0*q[2];
    float twx = tx*q[3];
    float twy = ty*q[3];
    float twz = tz*q[3];
    float txx = tx*q[0];
    float txy = ty*q[0];
    float txz = tz*q[0];
    float tyy = ty*q[1];
    float tyz = tz*q[1];
    float tzz = tz*q[2];

    mat[0][0] = 1.0-(tyy+tzz);
    mat[0][1] = txy-twz;
    mat[0][2] = txz+twy;
    mat[1][0] = txy+twz;
    mat[1][1] = 1.0-(txx+tzz);
    mat[1][2] = tyz-twx;
    mat[2][0] = txz-twy;
    mat[2][1] = tyz+twx;
    mat[2][2] = 1.0-(txx+tyy);
  }

};

_INLINE Matrix4x4 operator*(Matrix4x4 &a, Matrix4x4 &b)
{
  Matrix4x4 res;
  for (int i=0;i<4;i++)
    for (int j=0;j<4;j++) {
      res(i,j) = 0;
      for (int k=0;k<4;k++)
    res(i,j) += a(i,k)*b(k,j);
    }
  return res;
}

#if 0
{ /*! just to keep emacs happy w/ indenting ... */
#endif
} /* end namespace */
#endif /* RTTL_MATIRX_HXX */
