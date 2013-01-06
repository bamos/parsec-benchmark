/*! \file Triangle.hxx defines most typical triangle intersection code... */

#ifndef RTTL_TRIANGLE_HXX
#define RTTL_TRIANGLE_HXX

#include "../common/RTInclude.hxx"
#include "../common/RTBox.hxx"
#include "../common/RTRay.hxx"

using namespace RTTL;

// todo: use RTVec<3,sse_f> instead of explicit sse code, common origin case

template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS>
_INLINE void IntersectTriangleMoellerTrumbore(const RTVec3f &v0,
                          const RTVec3f &v1,
                          const RTVec3f &v2,
                          const int triangleID,
                          const int shaderID,
                          RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> &packet,
                          const int first_active,
                          const int last_active)
{
#if 0
  // only works once the v's are all sse_f's ...
  const sse_f ax = splat4<0>(v0);
  const sse_f ay = splat4<1>(v0);
  const sse_f az = splat4<2>(v0);

  const sse_f e0 = v1 - v0;
  const sse_f e0_x = splat4<0>(e0);
  const sse_f e0_y = splat4<1>(e0);
  const sse_f e0_z = splat4<2>(e0);

  const sse_f e1 = v2 - v0;
  const sse_f e1_x = splat4<0>(e1);
  const sse_f e1_y = splat4<1>(e1);
  const sse_f e1_z = splat4<2>(e1);
#else
  const sse_f ax = convert(v0[0]);
  const sse_f ay = convert(v0[1]);
  const sse_f az = convert(v0[2]);

  const sse_f bx = convert(v1[0]);
  const sse_f by = convert(v1[1]);
  const sse_f bz = convert(v1[2]);

  const sse_f cx = convert(v2[0]);
  const sse_f cy = convert(v2[1]);
  const sse_f cz = convert(v2[2]);

  const sse_f e0_x = bx - ax;
  const sse_f e0_y = by - ay;
  const sse_f e0_z = bz - az;

  const sse_f e1_x = cx - ax;
  const sse_f e1_y = cy - ay;
  const sse_f e1_z = cz - az;
#endif


  const sse_i id = convert(triangleID);
  const sse_i shader_id = convert(shaderID);

  sse_f offset_x, offset_y, offset_z;

  if (!MULTIPLE_ORIGINS)
    {
      offset_x = packet.originX(0) - ax;
      offset_y = packet.originY(0) - ay;
      offset_z = packet.originZ(0) - az;
    }

  for (int i = first_active; i < last_active; i++) {

    if (MULTIPLE_ORIGINS)
      {
    offset_x = packet.originX(i) - ax;
    offset_y = packet.originY(i) - ay;
    offset_z = packet.originZ(i) - az;
      }

    const sse_f pvec_x = (packet.directionY(i) * e1_z) - (packet.directionZ(i) * e1_y);
    const sse_f pvec_y = (packet.directionZ(i) * e1_x) - (packet.directionX(i) * e1_z);
    const sse_f pvec_z = (packet.directionX(i) * e1_y) - (packet.directionY(i) * e1_x);

    const sse_f det = (e0_x*pvec_x) + (e0_y*pvec_y) + (e0_z*pvec_z);
    const sse_f inv_det = rcp(det);

    const sse_f u = ((offset_x*pvec_x) + (offset_y*pvec_y) + (offset_z*pvec_z)) * inv_det;

    const sse_f zero = _mm_setzero_ps();
    const sse_f one  = convert(1.0f);

    const sse_f u_mask = _mm_and_ps(_mm_cmple_ps(zero,u),_mm_cmplt_ps(u,one));
    if (_mm_movemask_ps(u_mask) == 0x0) continue;

    const sse_f qvec_x = (offset_y*e0_z) - (offset_z*e0_y);
    const sse_f qvec_y = (offset_z*e0_x) - (offset_x*e0_z);
    const sse_f qvec_z = (offset_x*e0_y) - (offset_y*e0_x);

    const sse_f v = ((packet.directionX(i)*qvec_x) + (packet.directionY(i)*qvec_y) + (packet.directionZ(i)*qvec_z)) * inv_det;

    const sse_f v_mask = _mm_and_ps(_mm_cmple_ps(zero,v),_mm_cmple_ps(v,one));

    if (_mm_movemask_ps(v_mask) == 0x0) continue;

    const sse_f uv = u + v;

    const sse_f uv_mask = _mm_and_ps(_mm_cmplt_ps(zero,uv),_mm_cmple_ps(uv,one));

    const sse_f t = ((e1_x*qvec_x)+(e1_y*qvec_y)+(e1_z*qvec_z))*inv_det;
    const sse_f t_mask = _mm_and_ps(_mm_cmplt_ps(packet.minDistance(i),t),
                    _mm_cmplt_ps(t,packet.maxDistance(i)));
    const sse_f mask = u_mask & v_mask & uv_mask & t_mask;

    if (_mm_movemask_ps(mask) == 0x0) continue;

    packet.u(i)  = _mm_blendv_ps(u,packet.u(i),mask);
    packet.v(i)  = _mm_blendv_ps(v,packet.v(i),mask);
    packet.id(i) = _mm_blendv_epi8(id,packet.id(i),_mm_castps_si128(mask));
    packet.shaderID(i) = _mm_blendv_epi8(shader_id,packet.shaderID(i),_mm_castps_si128(mask));
    packet.maxDistance(i) = _mm_blendv_ps(t,packet.maxDistance(i),mask);
  }
}



/*! intersect given triangle using moeller-trumbore algo. intersect
  only the ray packlets (i.e., four-blocks of rays) as given by the
  packID[packIDs] array */
template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS>
_INLINE void IntersectTriangleMoellerTrumbore(const RTVec3f &v0,
                          const RTVec3f &v1,
                          const RTVec3f &v2,
                          const int triangleID,
                          const int shaderID,
                          RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> &packet,
                          const int packID[],
                          const int packIDs)
{
  const sse_f ax = convert(v0[0]);
  const sse_f ay = convert(v0[1]);
  const sse_f az = convert(v0[2]);

  const sse_f bx = convert(v1[0]);
  const sse_f by = convert(v1[1]);
  const sse_f bz = convert(v1[2]);

  const sse_f cx = convert(v2[0]);
  const sse_f cy = convert(v2[1]);
  const sse_f cz = convert(v2[2]);

  const sse_f e0_x = bx - ax;
  const sse_f e0_y = by - ay;
  const sse_f e0_z = bz - az;

  const sse_f e1_x = cx - ax;
  const sse_f e1_y = cy - ay;
  const sse_f e1_z = cz - az;

  const sse_i id = convert(triangleID);
  const sse_i shader_id = convert(shaderID);

  sse_f offset_x, offset_y, offset_z;

  if (!MULTIPLE_ORIGINS)
    {
      offset_x = packet.originX(0) - ax;
      offset_y = packet.originY(0) - ay;
      offset_z = packet.originZ(0) - az;
    }

  for (int j=0; j<packIDs; j++)
    {
      const int i = packID[j];

      if (MULTIPLE_ORIGINS)
    {
      offset_x = packet.originX(i) - ax;
      offset_y = packet.originY(i) - ay;
      offset_z = packet.originZ(i) - az;
    }

      const sse_f pvec_x = (packet.directionY(i) * e1_z) - (packet.directionZ(i) * e1_y);
      const sse_f pvec_y = (packet.directionZ(i) * e1_x) - (packet.directionX(i) * e1_z);
      const sse_f pvec_z = (packet.directionX(i) * e1_y) - (packet.directionY(i) * e1_x);

      const sse_f det = (e0_x*pvec_x) + (e0_y*pvec_y) + (e0_z*pvec_z);
      const sse_f inv_det = rcp(det);

      const sse_f u = ((offset_x*pvec_x) + (offset_y*pvec_y) + (offset_z*pvec_z)) * inv_det;

      const sse_f zero = _mm_setzero_ps();
      const sse_f one  = convert(1.0f);

      const sse_f u_mask = _mm_and_ps(_mm_cmple_ps(zero,u),_mm_cmplt_ps(u,one));
      if (_mm_movemask_ps(u_mask) == 0x0) continue;

      const sse_f qvec_x = (offset_y*e0_z) - (offset_z*e0_y);
      const sse_f qvec_y = (offset_z*e0_x) - (offset_x*e0_z);
      const sse_f qvec_z = (offset_x*e0_y) - (offset_y*e0_x);

      const sse_f v = ((packet.directionX(i)*qvec_x) + (packet.directionY(i)*qvec_y) + (packet.directionZ(i)*qvec_z)) * inv_det;

      const sse_f v_mask = _mm_and_ps(_mm_cmple_ps(zero,v),_mm_cmple_ps(v,one));

      if (_mm_movemask_ps(v_mask) == 0x0) continue;

      const sse_f uv = u + v;

      const sse_f uv_mask = _mm_and_ps(_mm_cmplt_ps(zero,uv),_mm_cmple_ps(uv,one));

      const sse_f t = ((e1_x*qvec_x)+(e1_y*qvec_y)+(e1_z*qvec_z))*inv_det;
      const sse_f t_mask = _mm_and_ps(_mm_cmplt_ps(packet.minDistance(i),t),
                      _mm_cmplt_ps(t,packet.maxDistance(i)));
      const sse_f mask = u_mask & v_mask & uv_mask & t_mask;

      if (_mm_movemask_ps(mask) == 0x0) continue;

      packet.u(i)  = _mm_blendv_ps(u,packet.u(i),mask);
      packet.v(i)  = _mm_blendv_ps(v,packet.v(i),mask);
      packet.id(i) = _mm_blendv_epi8(id,packet.id(i),_mm_castps_si128(mask));
      packet.shaderID(i) = _mm_blendv_epi8(shader_id,packet.shaderID(i),_mm_castps_si128(mask));
      packet.maxDistance(i) = _mm_blendv_ps(t,packet.maxDistance(i),mask);
    }
}


#endif
