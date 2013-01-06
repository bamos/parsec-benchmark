#ifndef RTTL_RAY_HXX
#define RTTL_RAY_HXX

#include "RTBox.hxx"
#include <vector>

namespace RTTL {
  /* actually, we have to spend quite some brain power on what exactly
     a ray should look like:

     - will we split ray and hitpoint, or have both together ? if we
     split, we conserve memory if we want to buffer a lot of rays
     before tracig some of them (which would be veeery good; but it's
     a pain otherwise, and probably requires storing both ray::max_t
     (and ray::min_t ?) and hit::t ... Could also, in theory, template
     the ray over its base type (float vs half), and then generate a
     IntervalRay as a Ray<FloatInterval> etc. Very nice, but do we
     need that ?

     - will we have a two-level system with instanceID and primitiveID
     ? or only one pointer ? what about instances of instances ?

     - what will we pack into the hit structure ? texcoords ? 3D
     hitpoint ? normal ? or only minimum intersection set (distance,
     hit/prim ID, and local surf coords) ? even _if_ we do lazy
     evaluation of hitpoint,normal, etc - how much of that do we have
     to _reserve_ mem for in the hit record ?

     - how much of the actual hitpoint calculations (hitpoint, normals,
     texcoords, etc) can we push down into the shader ? ideally, via
     a template mechanism, not via virtual functions .... (note that
     in that case, we need physically different shaders for the
     different primitive types ! i.e., a TriMeshDiffuseShade, a
     SubdivDiffuseShader, etc, plus something that generates the
     right shaders depending on object type ! what a pain ...)
  */

  class Ray {
  public:
    RTVec3f m_origin;
    RTVec3f m_direction;
    RTVec3f m_reciprocal;
    float   m_distance;
    float   m_ut;
    float   m_vt;
    int     m_id;
  }; // one ray ...

  class Hit {
  public:
  };

  /*! vector of rays, in (dumb) AoS form */
  class VectorOfRays {
  public:
    std::vector<Ray> m_ray;
  };

  template <int N>
  class PacketOfRays {
    Ray m_ray[N];
  };

  // Set particular bits to 1 to enable the specified functionality.
  enum LayoutTags {
    USE_CORNER_RAYS         = 1<<0, // first 4 rays define frustum
    STORE_NEAR_FAR_DISTANCE = 1<<1, // near values will be used in addition to far ones
    MIN_MAX_RECIPROCAL      = 1<<2, // store min/max of 1/distance for each coordinate
    VALID_RAYS_MASK         = 1<<3, // sse_f vector shown validity of each ray (in sign bit)
    VALID_GROUPS_MASK       = 1<<4, // integer array shown validity of each sse_f groups of rays
    STORE_VERTEX_NORMALS    = 1<<5  // assumes triangles as base primitives and stores vertex normals in hit structure
  };

  // Generic packet (with origins, directions and near/far values).
  // N                - packet size in sse_f units (4*N total rays).
  // LAYOUT           - how rays are packed: 1 for corner rays.
  // MULTIPLE_ORIGINS - 0 for common origins (primary rays); 1 otherwise.
  template <int N, int LAYOUT, int MULTIPLE_ORIGINS>
  class BaseRayPacket {
  public:
    // Accessing SIMD values.
    _INLINE sse_f   origin(int a, int i = 0) const { return m_origin[a][MULTIPLE_ORIGINS? i:0]; }
    _INLINE sse_f&  origin(int a, int i = 0)       { return m_origin[a][MULTIPLE_ORIGINS? i:0]; }
    _INLINE sse_f   originX(int i = 0) const { return m_origin.x[MULTIPLE_ORIGINS? i:0]; }
    _INLINE sse_f&  originX(int i = 0)       { return m_origin.x[MULTIPLE_ORIGINS? i:0]; }
    _INLINE sse_f   originY(int i = 0) const { return m_origin.y[MULTIPLE_ORIGINS? i:0]; }
    _INLINE sse_f&  originY(int i = 0)       { return m_origin.y[MULTIPLE_ORIGINS? i:0]; }
    _INLINE sse_f   originZ(int i = 0) const { return m_origin.z[MULTIPLE_ORIGINS? i:0]; }
    _INLINE sse_f&  originZ(int i = 0)       { return m_origin.z[MULTIPLE_ORIGINS? i:0]; }

    _INLINE sse_f   direction(int a, int i) const { return m_direction[a][i]; }
    _INLINE sse_f&  direction(int a, int i)       { return m_direction[a][i]; }
    _INLINE sse_f   directionX(int i) const  { return m_direction.x[i]; }
    _INLINE sse_f&  directionX(int i)        { return m_direction.x[i]; }
    _INLINE sse_f   directionY(int i) const  { return m_direction.y[i]; }
    _INLINE sse_f&  directionY(int i)        { return m_direction.y[i]; }
    _INLINE sse_f   directionZ(int i) const  { return m_direction.z[i]; }
    _INLINE sse_f&  directionZ(int i)        { return m_direction.z[i]; }

    _INLINE sse_f   reciprocal(int a, int i) const { return m_reciprocal[a][i]; }
    _INLINE sse_f&  reciprocal(int a, int i)       { return m_reciprocal[a][i]; }
    _INLINE sse_f   reciprocalX(int i) const  { return m_reciprocal.x[i]; }
    _INLINE sse_f&  reciprocalX(int i)        { return m_reciprocal.x[i]; }
    _INLINE sse_f   reciprocalY(int i) const  { return m_reciprocal.y[i]; }
    _INLINE sse_f&  reciprocalY(int i)        { return m_reciprocal.y[i]; }
    _INLINE sse_f   reciprocalZ(int i) const  { return m_reciprocal.z[i]; }
    _INLINE sse_f&  reciprocalZ(int i)        { return m_reciprocal.z[i]; }

    // These functions are meaninful only if MIN_MAX_RECIPROCAL bit is set.
    _INLINE sse_f   reciprocalMin(int i) const  { return m_reciprocal[i][N]; }
    _INLINE sse_f&  reciprocalMin(int i)        { return m_reciprocal[i][N]; }
    _INLINE sse_f   reciprocalMax(int i) const  { return m_reciprocal[i][N+1]; }
    _INLINE sse_f&  reciprocalMax(int i)        { return m_reciprocal[i][N+1]; }

    // All functions with distance keyword define max distance (far value).
    // All functions with minDistance keyword define min distance (near value).
    // minDistances are defined only if STORE_NEAR_FAR_DISTANCE layout bit is set.
    _INLINE sse_f   distance(int i) const     { return m_distance[i]; }
    _INLINE sse_f&  distance(int i)           { return m_distance[i]; }
    _INLINE sse_f   maxDistance(int i) const  { return m_distance[i]; }
    _INLINE sse_f&  maxDistance(int i)        { return m_distance[i]; }
    _INLINE sse_f   minDistance(int i) const  { return m_distance[i+N]; }
    _INLINE sse_f&  minDistance(int i)        { return m_distance[i+N]; }

    // Accessing float components.
    _INLINE float   floatOrigin(int a, int i = 0) const { return ((float*)&m_origin[a])[MULTIPLE_ORIGINS? i:0]; }
    _INLINE float&  floatOrigin(int a, int i = 0)       { return ((float*)&m_origin[a])[MULTIPLE_ORIGINS? i:0]; }
    _INLINE float   floatOriginX(int i = 0) const { return ((float*)&m_origin.x)[MULTIPLE_ORIGINS? i:0]; }
    _INLINE float&  floatOriginX(int i = 0)       { return ((float*)&m_origin.x)[MULTIPLE_ORIGINS? i:0]; }
    _INLINE float   floatOriginY(int i = 0) const { return ((float*)&m_origin.y)[MULTIPLE_ORIGINS? i:0]; }
    _INLINE float&  floatOriginY(int i = 0)       { return ((float*)&m_origin.y)[MULTIPLE_ORIGINS? i:0]; }
    _INLINE float   floatOriginZ(int i = 0) const { return ((float*)&m_origin.z)[MULTIPLE_ORIGINS? i:0]; }
    _INLINE float&  floatOriginZ(int i = 0)       { return ((float*)&m_origin.z)[MULTIPLE_ORIGINS? i:0]; }

    _INLINE float   floatDirection(int a, int i) const { return ((float*)&m_direction[a])[i]; }
    _INLINE float&  floatDirection(int a, int i)       { return ((float*)&m_direction[a])[i]; }
    _INLINE float   floatDirectionX(int i) const  { return ((float*)&m_direction.x)[i]; }
    _INLINE float&  floatDirectionX(int i)        { return ((float*)&m_direction.x)[i]; }
    _INLINE float   floatDirectionY(int i) const  { return ((float*)&m_direction.y)[i]; }
    _INLINE float&  floatDirectionY(int i)        { return ((float*)&m_direction.y)[i]; }
    _INLINE float   floatDirectionZ(int i) const  { return ((float*)&m_direction.z)[i]; }
    _INLINE float&  floatDirectionZ(int i)        { return ((float*)&m_direction.z)[i]; }

    _INLINE float   floatReciprocal(int a, int i) const { return ((float*)&m_reciprocal[a])[i]; }
    _INLINE float&  floatReciprocal(int a, int i)       { return ((float*)&m_reciprocal[a])[i]; }
    _INLINE float   floatReciprocalX(int i) const { return ((float*)&m_reciprocal.x)[i]; }
    _INLINE float&  floatReciprocalX(int i)       { return ((float*)&m_reciprocal.x)[i]; }
    _INLINE float   floatReciprocalY(int i) const { return ((float*)&m_reciprocal.y)[i]; }
    _INLINE float&  floatReciprocalY(int i)       { return ((float*)&m_reciprocal.y)[i]; }
    _INLINE float   floatReciprocalZ(int i) const { return ((float*)&m_reciprocal.z)[i]; }
    _INLINE float&  floatReciprocalZ(int i)       { return ((float*)&m_reciprocal.z)[i]; }

    _INLINE float   floatDistance(int i) const    { return ((float*)&m_distance)[i]; }
    _INLINE float&  floatDistance(int i)          { return ((float*)&m_distance)[i]; }
    _INLINE float   floatMaxDistance(int i) const { return ((float*)&m_distance)[i]; }
    _INLINE float&  floatMaxDistance(int i)       { return ((float*)&m_distance)[i]; }
    _INLINE float   floatMinDistance(int i) const { return ((float*)&m_distance)[i+N*sizeof(sse_f)/sizeof(float)]; }
    _INLINE float&  floatMinDistance(int i)       { return ((float*)&m_distance)[i+N*sizeof(sse_f)/sizeof(float)]; }


    _INLINE void computeReciprocalDirections()
    {
      int i;
#pragma unroll(4)
      for (i = 0; i < N; i++) {
        m_reciprocal.x[i] = rcp_save(m_direction.x[i]);
      }
#pragma unroll(4)
      for (i = 0; i < N; i++) {
        m_reciprocal.y[i] = rcp_save(m_direction.y[i]);
      }
#pragma unroll(4)
      for (i = 0; i < N; i++) {
        m_reciprocal.z[i] = rcp_save(m_direction.z[i]);
      }
    }

    _INLINE void computeReciprocalDirectionsAndInitMinMax()
    {
      int i;
      m_reciprocal.x[0] = rcp_save(m_direction.x[0]);
      m_reciprocal.y[0] = rcp_save(m_direction.y[0]);
      m_reciprocal.z[0] = rcp_save(m_direction.z[0]);
      sse_f minX,maxX,minY,maxY,minZ,maxZ;
      minX = maxX = m_reciprocal.x[0];
      minY = maxY = m_reciprocal.y[0];
      minZ = maxZ = m_reciprocal.z[0];

      for (i = 1; i < N; i++)
        {
          m_reciprocal.x[i] = rcp_save(m_direction.x[i]);
          m_reciprocal.y[i] = rcp_save(m_direction.y[i]);
          m_reciprocal.z[i] = rcp_save(m_direction.z[i]);
          minX = min(minX,m_reciprocal.x[i]);
          maxX = max(maxX,m_reciprocal.x[i]);
          minY = min(minY,m_reciprocal.y[i]);
          maxY = max(maxY,m_reciprocal.y[i]);
          minZ = min(minZ,m_reciprocal.z[i]);
          maxZ = max(maxZ,m_reciprocal.z[i]);
        }
      if (LAYOUT & MIN_MAX_RECIPROCAL) {
        m_reciprocal.x[N]   = minX;
        m_reciprocal.y[N]   = minY;
        m_reciprocal.z[N]   = minZ;
        m_reciprocal.x[N+1] = maxX;
        m_reciprocal.y[N+1] = maxY;
        m_reciprocal.z[N+1] = maxZ;
      }
    }

  protected:

    // m_origin has different size for packets with common/multiple origins.
    RTVec_t<3, RTVec_t<1 + (N-1)*MULTIPLE_ORIGINS, sse_f> > m_origin;

    typedef RTVec_t<N, sse_f> ArrayN;
    RTVec_t<3, ArrayN> m_direction;  // may be not normalized
    // If MIN_MAX_RECIPROCAL is set,
    // the last 2 entries in m_reciprocal will store global min/max values.
    RTVec_t<3, RTVec_t<N + ((LAYOUT & MIN_MAX_RECIPROCAL)?2:0), sse_f> > m_reciprocal; // 1/direction

    // m_distance has different size for packets with near/far and only far values
    // (defined by STORE_NEAR_FAR_DISTANCE layout bit)
    RTVec_t<N*((LAYOUT & STORE_NEAR_FAR_DISTANCE)?2:1), sse_f> m_distance;

  };

  template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS>
  class RayPacket: public BaseRayPacket<N, LAYOUT, MULTIPLE_ORIGINS> {
    // Private ctor to prohibit explicit instantiation of this class.
    // Only specialized classes (with specific values of SHADOW_RAYS) are allowed.
    RayPacket() {}
  };

  // Primary/secondary (with texture coordinates and hit id).
  template <int N, int LAYOUT, int MULTIPLE_ORIGINS>
  class RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, 0>: public BaseRayPacket<N, LAYOUT, MULTIPLE_ORIGINS> {
  public:
    typedef BaseRayPacket<N, LAYOUT, MULTIPLE_ORIGINS> Base;
    // Accessing SIMD values.
    _INLINE sse_f   u(int i)  const { return m_ut[i]; }
    _INLINE sse_f&  u(int i)        { return m_ut[i]; }
    _INLINE sse_f   v(int i)  const { return m_vt[i]; }
    _INLINE sse_f&  v(int i)        { return m_vt[i]; }
    _INLINE sse_i  id(int i)  const { return m_id[i]; }
    _INLINE sse_i& id(int i)        { return m_id[i]; }
    _INLINE sse_i  shaderID(int i)  const { return m_shaderID[i]; }
    _INLINE sse_i& shaderID(int i)        { return m_shaderID[i]; }

    // Accessing float/integer components.
    _INLINE float  floatU(int i) const { return ((float*)&m_ut)[i]; }
    _INLINE float& floatU(int i)       { return ((float*)&m_ut)[i]; }
    _INLINE float  floatV(int i) const { return ((float*)&m_vt)[i]; }
    _INLINE float& floatV(int i)       { return ((float*)&m_vt)[i]; }
    _INLINE int    intId(int i)  const { return ((int*)  &m_id)[i]; }
    _INLINE int&   intId(int i)        { return ((int*)  &m_id)[i]; }
    _INLINE int    intShaderID(int i)  const { return ((int*)  &m_shaderID)[i]; }
    _INLINE int&   intShaderID(int i)        { return ((int*)  &m_shaderID)[i]; }

    // extended components

    _INLINE sse_f   vertexNormalX(int v, int i) const { return m_vertexNormal[v][0][i]; }
    _INLINE sse_f&  vertexNormalX(int v, int i)       { return m_vertexNormal[v][0][i]; }
    _INLINE sse_f   vertexNormalY(int v, int i) const { return m_vertexNormal[v][1][i]; }
    _INLINE sse_f&  vertexNormalY(int v, int i)       { return m_vertexNormal[v][1][i]; }
    _INLINE sse_f   vertexNormalZ(int v, int i) const { return m_vertexNormal[v][2][i]; }
    _INLINE sse_f&  vertexNormalZ(int v, int i)       { return m_vertexNormal[v][2][i]; }

    _INLINE void reset() {
      int i;
      for (i = 0; i < N; i++)
        Base::maxDistance(i) = infinity<sse_f>();

      if (LAYOUT & STORE_NEAR_FAR_DISTANCE)
        for (i = 0; i < N; i++)
          Base::minDistance(i) = convert<sse_f>(0);

      // not sure if we should initialize shaderIDs here
      m_shaderID = convert<sse_i>(0);
      m_id = convert<sse_i>(-1);
    }

  protected:
    RTVec_t<N, sse_f> m_ut; // texture coordinates
    RTVec_t<N, sse_f> m_vt; // of hit points
    RTVec_t<N, sse_i> m_id; // id of hit object(s) or -1
    RTVec_t<N, sse_i> m_shaderID; // shader id of hit object(s) or -1
    RTVec_t<3, RTVec_t<1+(N-1)*((LAYOUT & STORE_VERTEX_NORMALS)?1:0), sse_f> > m_vertexNormal[3];
  };

  // Shadow packet (no u,v,id).
  template <int N, int LAYOUT, int MULTIPLE_ORIGINS>
  class RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, 1>: public BaseRayPacket<N, LAYOUT, MULTIPLE_ORIGINS> {
  public:
    typedef BaseRayPacket<N, LAYOUT, MULTIPLE_ORIGINS> Base;
    _INLINE void reset() {
      int i;
      for (i = 0; i < N; i++)
        Base::maxDistance(i) = convert<sse_f>(1);

      if (LAYOUT & STORE_NEAR_FAR_DISTANCE)
        for (i = 0; i < N; i++)
          Base::minDistance(i) = convert<sse_f>(0);
    }
  };

  // extended primary/secondary (with texture coordinates, hit id, and normals).

};

#endif
