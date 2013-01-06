#ifndef RTVIEW_CAMERA_HXX
#define RTVIEW_CAMERA_HXX

#include "RTTL/common/RTInclude.hxx"
#include "RTTL/common/RTVec.hxx"
#include "RTTL/common/RTMatrix.hxx"

using namespace RTTL;


class Camera
{
public:

  float angle;
  Matrix4x4 matrix;

  Camera()
  {
  }


  _INLINE void initMatrix(const RTVec3f &origin,const RTVec3f &direction,const RTVec3f &up, const float viewAngle)
  {
    RTVec3f ex = direction ^ up;
    ex.normalize();
    RTVec3f ez = ex ^ direction;
    ez.normalize();
    RTVec3f ey = direction;
    ey.normalize();
    matrix.setCol(0,ey); //dir
    matrix.setCol(1,ex); //right
    matrix.setCol(2,ez); //up
    matrix.setCol(3,origin); //view-pt
    angle = viewAngle;
  }


  _INLINE void rotateGlobal(RTVec3f dir,float angle)
  {
    Matrix4x4 rot;
    rot.setIdentity();
    rot.setRotationPart(dir, angle);
    Matrix4x4 m1 = matrix;

    RTVec3f pos = matrix.getCol(3);
    matrix = rot * m1;
    matrix.setCol(3,pos);
  }

  _INLINE void rotateLocal(RTVec3f dir,float angle)
  {
    Matrix4x4 rot;
    rot.setIdentity();
    rot.setRotationPart(dir, angle);
    Matrix4x4 m1 = matrix;
    matrix = m1 * rot;
  }

  _INLINE void moveLocal(RTVec3f dir)
  {
    Matrix4x4 move;
    move.setIdentity();
    move.setTranslationPart(dir);
    Matrix4x4 m1 = matrix;
    matrix = m1 * move;
  }

  _INLINE RTVec3f getOrigin()
  {
    return matrix.getCol(3);
  }

  _INLINE RTVec3f getDirection()
  {
    return matrix.getCol(0);
  }

  _INLINE RTVec3f getUp()
  {
    return matrix.getCol(2);
  }

  _INLINE void setOrigin(RTVec3f v)
  {
    return matrix.setCol(3,v);
  }

  _INLINE void setDirection(RTVec3f v)
  {
    return matrix.setCol(0,v);
  }

  _INLINE void setUp(RTVec3f v)
  {
    return matrix.setCol(2,v);
  }

  _INLINE float getAngle()
  {
    return angle;
  }
};

#endif
