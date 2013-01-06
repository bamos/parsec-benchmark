/*! \file Mesh.hxx defines basic mesh containers for storing geometry ... */

#ifndef RTTL_MESH_HXX
#define RTTL_MESH_HXX

#include "../common/RTInclude.hxx"
#include "../common/RTVec.hxx"
#include "../common/RTBox.hxx"
#include "../common/Timer.hxx"
#include "../Triangle/Triangle.hxx"
//#include "../BVH/BVH.hxx"

#include <vector>
#include <map>


namespace RTTL {

  /* something like a base class for polygonal meshes */

  enum VertexType {
    RT_VERTEX_3F,
    RT_VERTEX_4F
  };
  enum PrimtiveType {
    RT_TRIANGLE,
    RT_QUAD
  };

  class PolygonalBaseMesh {
  public:
    PolygonalBaseMesh() {
      m_meshAABB.setEmpty();
      m_meshCentroidAABB.setEmpty();
    }

    _INLINE const RTBoxSSE &getAABB() const { return m_meshAABB; } 
    _INLINE const RTBoxSSE &getCentroidAABB() const { return m_meshCentroidAABB; } 

    virtual void clear() = 0;
    virtual RTBoxSSE getAABB(const int i) const = 0;

    virtual void storePrimitiveAABBs(RTBoxSSE *const ptr, const int numAABBs) = 0;
    virtual int numPrimitives() const = 0;
    virtual int numVertices() const = 0;
    virtual void addVertices(const float *const v,
                             const float *const txt,
                             const int vertices, 
                             const int type) = 0;
    virtual void addPrimitives(const int *const t,const int primitives, const int type, const int *const shaderID = NULL) = 0;

    virtual void finalize() = 0;

    template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS,int NORMALIZE>    
    _INLINE void getGeometryNormal(RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> &packet,
                                   const int id4,
                                   RTVec_t<3, sse_f> &normal) const
    {
    }

  protected:
    RTBoxSSE m_meshAABB;
    RTBoxSSE m_meshCentroidAABB;

  };

  class StandardTriangleMesh : public PolygonalBaseMesh {
  protected:

    struct Triangle {
      RTVec3i v;
      int shaderID;
    };

    vector< RTVec2f, Align<RTVec2f> > textureCoord;
    vector< RTVec3f, Align<RTVec3f> > vertex;
    vector< Triangle, Align<Triangle> > triangle;

  public:

    StandardTriangleMesh()
    {
    }


    virtual void clear() {
      m_meshAABB.setEmpty();
      m_meshCentroidAABB.setEmpty();
      vertex.clear();
      triangle.clear();
    }

    virtual void addVertices(const float *const v,const float *const txt,const int vertices, const int type) {
      assert(v);
      assert(type == RT_VERTEX_3F);

      vertex.reserve(vertex.size()+vertices);
      for (int i=0;i<vertices;i++)
        vertex.push_back(((RTVec3f*)v)[i]);

      /* -- if != NULL transfer texture coordinates -- */
      if (txt != NULL)
        for (int i=0;i<vertices;i++)
          {
            textureCoord.push_back(((RTVec2f*)txt)[i]);
          }
    }

    virtual void addPrimitives(const int *const t,const int primitives, const int type, const int *const shaderID = NULL) {
      assert(t);
      switch(type)
        {
        case RT_TRIANGLE:
          triangle.reserve(triangle.size()+primitives);
          for (int i=0;i<primitives;i++)
            {
              Triangle tri;
              tri.v = ((RTVec3i*)t)[i];
              tri.shaderID = (shaderID != NULL) ? shaderID[i] : 0;
              triangle.push_back(tri);
              RTBoxSSE box = getTriangleAABB(tri.v[0],tri.v[1],tri.v[2]);
              m_meshAABB.extend(box);
              m_meshCentroidAABB.extend(box.center());
            }
          break;
        case RT_QUAD:
          triangle.reserve(triangle.size()+2*primitives);
          for (int i=0;i<primitives;i++)
            {
              Triangle tri;
              RTVec4i c = ((RTVec4i*)t)[i];
              tri.shaderID = (shaderID != NULL) ? shaderID[i] : 0;
              tri.v = RTVec3i(c[0],c[1],c[2]);
              triangle.push_back(tri);
              tri.v = RTVec3i(c[2],c[3],c[0]);
              triangle.push_back(tri);
              RTBoxSSE box0 = getTriangleAABB(c[0],c[1],c[2]);
              RTBoxSSE box1 = getTriangleAABB(c[2],c[3],c[0]);
              m_meshAABB.extend(box0);
              m_meshAABB.extend(box1);
              m_meshCentroidAABB.extend(box0.center());
              m_meshCentroidAABB.extend(box1.center());
            }
          break;
        default:
          FATAL("addPrimitives: unknown primitive type");
        }
    }

    virtual int numPrimitives() const
    {
      return triangle.size();
    }

    virtual int numVertices() const
    {
      return vertex.size();
    }

    virtual RTBoxSSE getAABB(const int i) const {      
      return getTriangleAABB(i);
    }

    virtual void storePrimitiveAABBs(RTBoxSSE *const ptr, const int numAABBs)
    {
      for (int i=0;i<numAABBs;i++)
        ptr[i] = getTriangleAABB(i);
    }

    virtual void finalize()
    {
      print();
    }

    _INLINE void print() const {
      cout << "vertices            " << vertex.size() << " (" << KBytes(sizeof(RTVec3f)*vertex.size()) << " KB)" << endl;
      cout << "triangles           " << triangle.size() << " (" << KBytes(sizeof(RTVec3i)*triangle.size()) << " KB)" << endl;
      cout << "texture coordinates " << textureCoord.size() << " (" << KBytes(sizeof(RTVec2f)*textureCoord.size()) << " KB)" << endl;
      //for (int i=0;i<textureCoord.size();i++)
      //cout << i << " -> " << textureCoord[i] << endl;
#if 0
      for (unsigned int i=0; i < triangle.size(); i++)
        cout << i << " : " << triangle[i][0] << " " << triangle[i][1] << " " << triangle[i][2]
             << " -> " << vertex[triangle[i][0]] << " " << vertex[triangle[i][1]] << " " << vertex[triangle[i][2]] << endl;
#endif
    }

    _INLINE const RTVec3f &getTriangleVertex(const int t, const int i) const
    {
      assert(0 <= i && i <=2);
      assert(t < (int)triangle.size());
      return vertex[triangle[t].v[i]];
    }

    _INLINE int getShaderID(const int t) const
    {
      return triangle[t].shaderID;
    }

    _INLINE RTVec3f getTriangleNormal(const int t) const
    {
      const RTVec3f &a = getTriangleVertex(t,0);
      const RTVec3f &b = getTriangleVertex(t,1);
      const RTVec3f &c = getTriangleVertex(t,2);
      return (a-c) ^ (b-c);
    }

    _INLINE RTBoxSSE getTriangleAABB(const int i) const {
      return getTriangleAABB(triangle[i].v[0],triangle[i].v[1],triangle[i].v[2]);
    }

    _INLINE RTBoxSSE getTriangleAABB(const int id0, const int id1, const int id2) const 
    {
      RTBoxSSE box;
      box.setEmpty();
      const sse_f v0 = _mm_setr_ps(vertex[id0][0],vertex[id0][1],vertex[id0][2],0.0f);
      const sse_f v1 = _mm_setr_ps(vertex[id1][0],vertex[id1][1],vertex[id1][2],0.0f);
      const sse_f v2 = _mm_setr_ps(vertex[id2][0],vertex[id2][1],vertex[id2][2],0.0f);
      box.extend(v0);
      box.extend(v1);
      box.extend(v2);
      return box;
    }
    
    template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS>    
    _INLINE void intersectPrimitive(RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> &packet, 
                                    const int ID,
                                    const int first,
                                    const int last) const
    {
      const RTVec3f &v0 = getTriangleVertex(ID,0);
      const RTVec3f &v1 = getTriangleVertex(ID,1);
      const RTVec3f &v2 = getTriangleVertex(ID,2);
      const int shaderID = getShaderID(ID);
      IntersectTriangleMoellerTrumbore<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>(v0,v1,v2,ID,shaderID,packet,first,last);
    }

    template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS,int NORMALIZE>    
    _INLINE void getGeometryNormal(RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> &packet,
                                   const int id4,
                                   RTVec_t<3, sse_f> &normal) const
    {
      const sse_i &fourID = packet.id(id4);
      if (__builtin_expect(fourID == splat4i<0>(fourID),1))
        {
          const int ID = M128_INT(fourID,0);
          if (__builtin_expect(ID == -1,0))
            {
              normal[0] = _mm_setzero_ps();
              normal[1] = _mm_setzero_ps();
              normal[2] = _mm_setzero_ps();
            }
          else
            {
              const RTVec3f n = getTriangleNormal(ID);
              normal[0] = convert<sse_f>(n.x);
              normal[1] = convert<sse_f>(n.y);
              normal[2] = convert<sse_f>(n.z);
            }
        }
      else
        {
#pragma unroll(4)
          for (int i=0;i<4;i++)
            {
              const int ID = M128_INT(packet.id(id4),i);
              if (__builtin_expect(ID == -1,0)) 
                {
                  M128_FLOAT(normal[0],i) = 0.0f;
                  M128_FLOAT(normal[1],i) = 0.0f;
                  M128_FLOAT(normal[2],i) = 0.0f;          
                }
              else
                {
                  const RTVec3f n = getTriangleNormal(ID);
                  M128_FLOAT(normal[0],i) = n.x;
                  M128_FLOAT(normal[1],i) = n.y;
                  M128_FLOAT(normal[2],i) = n.z;
                }
            }
        }
      if (NORMALIZE)
        {
          const sse_f f = rsqrt(normal[0]*normal[0]+normal[1]*normal[1]+normal[2]*normal[2]);
          normal[0] *= f;
          normal[1] *= f;
          normal[2] *= f;
        }    
    }

    template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS,int NORMALIZE>    
    _INLINE void getTextureCoordinate(RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> &packet,
                                      const int id4,
                                      RTVec_t<2, sse_f> &txt) const
    {
      const sse_i &fourID = packet.id(id4);

      const sse_f u = packet.u(id4);
      const sse_f v = packet.v(id4);
      const sse_f uv = _mm_sub_ps(convert<sse_f>(1.0f),_mm_add_ps(u,v));

      if (__builtin_expect(fourID == splat4i<0>(fourID),1))
        {
          const int ID = M128_INT(fourID,0);
          if (__builtin_expect(ID == -1,0)) 
            {
              txt[0] = _mm_setzero_ps();
              txt[1] = _mm_setzero_ps();
            }
          else
            {
              const sse_f u0 = convert<sse_f>(textureCoord[triangle[ID].v[0]][0]);
              const sse_f v0 = convert<sse_f>(textureCoord[triangle[ID].v[0]][1]);
              const sse_f u1 = convert<sse_f>(textureCoord[triangle[ID].v[1]][0]);
              const sse_f v1 = convert<sse_f>(textureCoord[triangle[ID].v[1]][1]);
              const sse_f u2 = convert<sse_f>(textureCoord[triangle[ID].v[2]][0]);
              const sse_f v2 = convert<sse_f>(textureCoord[triangle[ID].v[2]][1]);
              txt[0] = uv * u0 + u * u1 + v * u2;
              txt[1] = uv * v0 + u * v1 + v * v2;
            }
        }
      else
        {
          sse_f u0,v0,u1,v1,u2,v2;
#pragma unroll(4)
          for (int i=0;i<4;i++)
            {
              const int ID = M128_INT(packet.id(id4),i);
              if (__builtin_expect(ID == -1,0)) 
                {
                  M128_FLOAT(u0,i) = 0.0f;
                  M128_FLOAT(u1,i) = 0.0f;
                  M128_FLOAT(u2,i) = 0.0f;
                  M128_FLOAT(v0,i) = 0.0f;
                  M128_FLOAT(v1,i) = 0.0f;
                  M128_FLOAT(v2,i) = 0.0f;
                }
              else
                {
                  M128_FLOAT(u0,i) = textureCoord[triangle[ID].v[0]][0];
                  M128_FLOAT(v0,i) = textureCoord[triangle[ID].v[0]][1];
                  M128_FLOAT(u1,i) = textureCoord[triangle[ID].v[1]][0];
                  M128_FLOAT(v1,i) = textureCoord[triangle[ID].v[1]][1];
                  M128_FLOAT(u2,i) = textureCoord[triangle[ID].v[2]][0];
                  M128_FLOAT(v2,i) = textureCoord[triangle[ID].v[2]][1];
                }
            }
          txt[0] = uv * u0 + u * u1 + v * u2;
          txt[1] = uv * v0 + u * v1 + v * v2;
        }
    }

  };


  class DirectedEdgeMesh : public PolygonalBaseMesh {
  protected:

    struct DirectedEdge {
      unsigned int start_vertex    : 30;
      unsigned int boundary_vertex :  1; // vertex incident to a boundary edge
      unsigned int regular_vertex  :  1; // four quads incident to given vertex
      int neighbor_edge;
      DirectedEdge() {}

      DirectedEdge(const int _vertex,const int _neighbor = -1) : start_vertex(_vertex), neighbor_edge(_neighbor) {}     
    };

    vector< sse_f, Align<sse_f> > vertex;
    vector< DirectedEdge, Align<DirectedEdge> > edge;

  public:

    DirectedEdgeMesh()
    {
    }

    virtual RTBoxSSE getAABB(const int quadID) const 
    {
      return quadGetOneNeighborhoodAABB(quadID);
    }

    virtual void storePrimitiveAABBs(RTBoxSSE *const ptr, const int numAABBs)
    {
      m_meshAABB.setEmpty();
      m_meshCentroidAABB.setEmpty();
      for (int i=0;i<numAABBs;i++)
        {
          RTBoxSSE box = quadGetOneNeighborhoodAABB(i);
          m_meshAABB.extend(box);
          m_meshCentroidAABB.extend(box.center());
          ptr[i] = box;
        }
    }

    virtual int numPrimitives() const 
    {
      return quads();
    }

    virtual int numVertices() const
    {
      return vertices();
    }

    virtual void clear() 
    {
      vertex.clear();
      edge.clear();
    }

    _INLINE void addVertices(const float *const v,const float *const txt,const int primitives, const int type) {
      assert(v);
      assert(type == RT_VERTEX_3F);
      vertex.reserve(vertex.size()+primitives);
      for (int i=0;i<primitives;i++)
        {
          const sse_f c = vec3fToSSE(((RTVec3f*)v)[i]);
          m_meshAABB.extend(c);
          vertex.push_back(c);
        }
      if (txt != NULL)
        cout << "texture coordinates currently not supported" << endl;
    }

    _INLINE void addPrimitives(const int *const t,const int primitives, const int type, const int *const shaderIds = NULL) {
      assert(t);
      assert(type == RT_QUAD);

      edge.reserve(edge.size()+4*primitives);
      for (int i=0;i<primitives;i++)
        {
          const RTVec4i &c = ((RTVec4i*)t)[i];
          edge.push_back(DirectedEdge(c[0]));
          edge.push_back(DirectedEdge(c[1]));
          edge.push_back(DirectedEdge(c[2]));
          edge.push_back(DirectedEdge(c[3]));              
        }
    }


    _INLINE void print() const {
      cout << "vertices  " << vertices() << " (" << KBytes(sizeof(sse_f)*vertices()) << " KB)" << endl;
      cout << "edges     " << edges() << " (" << KBytes(sizeof(DirectedEdge)*edges()) << " KB)" << endl;
      cout << "quads     " << quads() << endl;
      //for (unsigned int i=0; i < edge.size(); i++) printEdge(i);
    }

    virtual void finalize()
    {
      DBG_PRINT(sizeof(DirectedEdge));
      cout << "Computing topology..." << endl;
      Timer timer;
      timer.start();
      computeTopology();
      float t = timer.stop();
      cout << "finished in (" << t << " sec)" << endl;
      print();
      printBoundaryEdges();
    }

    template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS,int NORMALIZE>    
    _INLINE void getGeometryNormal(RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> &packet,
                                   const int id4,
                                   RTVec_t<3, sse_f> &normal) const
    {
    }

    template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS,int NORMALIZE>    
    _INLINE void getTextureCoordinate(RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> &packet,
                                      const int id4,
                                      RTVec_t<2, sse_f> &txt) const
    {
    }

    template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS>    
    _INLINE void intersectPrimitive(RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> &packet, 
                                    const int ID,
                                    const int first,
                                    const int last) const
    {
      const int shaderID = getShaderID(0);
      const RTVec3f &v0 = *(RTVec3f*)&vertex[quadGetVertex(ID,0)];
      const RTVec3f &v1 = *(RTVec3f*)&vertex[quadGetVertex(ID,1)];
      const RTVec3f &v2 = *(RTVec3f*)&vertex[quadGetVertex(ID,2)];
      const RTVec3f &v3 = *(RTVec3f*)&vertex[quadGetVertex(ID,3)];
      IntersectTriangleMoellerTrumbore<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>(v0,v1,v2,ID,shaderID,packet,first,last);
      IntersectTriangleMoellerTrumbore<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>(v2,v3,v0,ID,shaderID,packet,first,last);
    }

    _INLINE bool InitCatmullClarkVertexMatrix(const int quadID,sse_f (&matrix)[4][4], sse_f *ev) const
    {
      const int regularVertexPosition[4][2] = {
        { 1, 1},
        { 1, 2},
        { 2, 2},
        { 2, 1}
      };
      if (quadHasBoundary(quadID)) return false;

      if (quadIsRegular(quadID))
        {
          for (int i=0;i<4;i++)
            vertexCopyRegularOneNeigborhood(quadFirstEdge(quadID)+i,i,regularVertexPosition[i],matrix);
        }
      else
        for (int i=0;i<4;i++)	
          if (quadIsVertexRegular(quadID,i))
            vertexCopyRegularOneNeigborhood(quadFirstEdge(quadID)+i,i,regularVertexPosition[i],matrix);
          else
            vertexCopyIrregularOneNeigborhood(quadFirstEdge(quadID),ev);	      
      return true;
    }

    _INLINE int getShaderID(const int t) const
    {
      return 0; // triangle[t].shaderID;
    }

  protected:

    // initialization in ccw order, quad edges in cw order
    _INLINE void vertexCopyRegularOneNeigborhood(const int startEdge,const int offset,const int pos[2],sse_f (&matrix)[4][4]) const
    {
      const int adj[8][2] = { { 1, 1},{ 0, 1},{-1, 1},{-1, 0},{-1,-1},{ 0,-1},{ 1,-1},{ 1, 0} };

      matrix[pos[0]][pos[1]] = vertex[start_vertex(startEdge)];    

      int edge = startEdge;
      int index = (8 - offset * 2) % 8;
      do {
        //cout << "y0 " << pos[0] + adj[index+0][0] << " x0 " << pos[1] + adj[index+0][1] << endl;
        //cout << "y1 " << pos[0] + adj[index+1][0] << " x1 " << pos[1] + adj[index+1][1] << endl;
        matrix[ pos[0] + adj[index+0][0] ][ pos[1] + adj[index+0][1] ] = vertex[end_vertex(next(edge))];
        matrix[ pos[0] + adj[index+1][0] ][ pos[1] + adj[index+1][1] ] = vertex[end_vertex(edge)];
        index = (index + 2) % 8;
        const int neighbor = opposite(edge);    
        if (__builtin_expect(neighbor == -1,0)) { WARN("boundary for regular patches not yet supported"); return; }
        edge = next(neighbor);
      } while(edge != startEdge);
    }

    // initialization in cw order due to compatibility
    _INLINE int vertexCopyIrregularOneNeigborhood(const int startEdge,sse_f *dest) const
    {
      dest[0]                = vertex[start_vertex(startEdge)];
      int edge = startEdge;
      int index = 1;
      do {
        dest[index+0] = vertex[end_vertex(edge)];
        dest[index+1] = vertex[end_vertex(next(edge))];
        index += 2;
        const int neighbor = opposite(edge);    
        if (__builtin_expect(neighbor == -1,0)) FATAL("not supported");
        edge = next(neighbor);
      } while(edge != startEdge);
      return index;
    }

    /* --------------------------------------------------------------------------- */

    _INLINE int next(const int i) const { return ((i+1) % 4) + (i & ~3); } 
    _INLINE int prev(const int i) const { return ((i+3) % 4) + (i & ~3); }
    
    _INLINE int opposite(const int i) const { return edge[i].neighbor_edge; }
    _INLINE bool boundary(const int i) const { return edge[i].neighbor_edge == -1; }
    _INLINE int start_vertex(const int i) const { return edge[i].start_vertex; }
    _INLINE int end_vertex(const int i) const { return edge[next(i)].start_vertex; }

    _INLINE int quadFirstEdge(const int quad) const {
      return quad << 2;
    }

    _INLINE void printEdge(int i) const {
      cout << i << " (" << i%4 << ") : " 
           << "vertex = " << edge[i].start_vertex 
           << " neighbor = " << edge[i].neighbor_edge << " -> " << vertex[edge[i].start_vertex] << endl;      
    }

    _INLINE void printBoundaryEdges() const {
      int boundaryEdges = 0;
      for (int i=0;i<edges();i++)
        if (boundary(i))
          { printEdge(i); boundaryEdges++; }
      cout << "#boundary edges " << boundaryEdges << endl;
    }

    _INLINE void computeTopology()
    {
      PING;
      DBG_PRINT(quads());
      std::map< std::pair<int, int>, int > edgeMap;
      for (int i=0;i<edges();i++)
        {
          vertexBindEdgeID(edge[i].start_vertex,i);
          const int a = start_vertex(i);
          const int b = end_vertex(i);
          const int index = findOrInsertEdge(a,b,i,edgeMap);
          if (index != -1)
            {
              assert(edge[i].neighbor_edge == -1);
              assert(edge[index].neighbor_edge == -1);
              edge[i].neighbor_edge = index;
              edge[index].neighbor_edge = i;
            }
        }

      int boundaryVertices = 0;
      for (int i=0;i<edges();i++)
        {
          edge[i].boundary_vertex = vertexHasBoundaryEdge(edge[i].start_vertex) ? 1 : 0;
          boundaryVertices += edge[i].boundary_vertex;
          edge[i].regular_vertex  = vertexGetEdgeValence(edge[i].start_vertex) == 4 ? 1 : 0;
        }
      
#if 1
      int regularPatches = 0;
      for (int i=0;i<quads();i++)
        {
          if (quadIsRegularWithoutBoundary(i)) regularPatches++;
        }
      DBG_PRINT(regularPatches);
      DBG_PRINT(boundaryVertices);
#endif      
    }

    _INLINE int findOrInsertEdge(int v0,int v1,const int edgeIndex, std::map< std::pair<int, int>, int > &edgeMap)
    {
      std::map< std::pair<int, int>, int >::iterator it;
      if (v0 > v1) std::swap(v0,v1);

      it = edgeMap.find(std::pair<int,int>(v0,v1));
      if ( it == edgeMap.end()) 
        {
          edgeMap.insert(make_pair(std::pair<int,int>(v0,v1),edgeIndex));
          return -1;
        }
      else
        return it->second;
    }

    /* -------------------------------------------------------------------------------------- */
    /* -------------------------------------------------------------------------------------- */
    /* -------------------------------------------------------------------------------------- */

    _INLINE int quads() const {
      return edge.size() >> 2;
    }

    _INLINE int edges() const {
      return edge.size();
    }

    _INLINE int vertices() const {
      return vertex.size();
    }

    // todo: get rid of many redundent operations
    _INLINE RTBoxSSE quadGetOneNeighborhoodAABB(const int quad) const {
      RTBoxSSE box;
      box.setEmpty();
      box.extend(vertexGetOneNeighborhood(quadGetVertex(quad,0)));
      box.extend(vertexGetOneNeighborhood(quadGetVertex(quad,1)));
      box.extend(vertexGetOneNeighborhood(quadGetVertex(quad,2)));
      box.extend(vertexGetOneNeighborhood(quadGetVertex(quad,3)));
      return box;
    }

    // NOTE: 1-neighborhood consists of quads, need to include opposite vertex
    _INLINE RTBoxSSE vertexGetOneNeighborhood(const int vertexID) const {
      RTBoxSSE box;
      box.min_f() = vertex[vertexID];
      box.max_f() = vertex[vertexID];
      const int startEdge = vertexGetEdgeID(vertexID);
      int edge = startEdge;
      do {
        box.extend(vertex[end_vertex(edge)]);
        box.extend(vertex[end_vertex(next(edge))]); // opposite vertex
        const int neighbor = opposite(edge);    
        if (__builtin_expect(neighbor == -1,0)) // found boundary vertex, cycle in reverse direction
          { 
            edge = prev(startEdge);
            while(1) {
              box.extend(vertex[start_vertex(edge)]);
              box.extend(vertex[start_vertex(prev(edge))]); // opposite vertex
              const int neighbor = opposite(edge);
              if (__builtin_expect(neighbor == -1,0)) break; // found second boundary edge
              edge = prev(neighbor);
            }
            break;
            DBG_PRINT(box);
          }
        edge = next(neighbor);
      } while(edge != startEdge);
      return box;
    }


    _INLINE int vertexGetEdgeValence(const int vertexID) const {
      int valence = 0;
      const int startEdge = vertexGetEdgeID(vertexID);
      int edge = startEdge;
      do {
        valence++;
        const int neighbor = opposite(edge);    
        if (__builtin_expect(neighbor == -1,0)) // found boundary vertex, cycle in reverse direction
          { 
            edge = prev(startEdge);
            while(1) {
              valence++;
              const int neighbor = opposite(edge);
              if (__builtin_expect(neighbor == -1,0)) break; // found second boundary edge
              edge = prev(neighbor);
            }
            break;
          }
        edge = next(neighbor);
      } while(edge != startEdge);
      return valence;
    }

    _INLINE bool vertexHasBoundaryEdge(const int vertexID) const {
      int valence = 0;
      const int startEdge = vertexGetEdgeID(vertexID);
      int edge = startEdge;
      do {
        valence++;
        const int neighbor = opposite(edge);    
        if (__builtin_expect(neighbor == -1,0)) return true;
        edge = next(neighbor);
      } while(edge != startEdge);
      return false;
    }

    // does not handle boundaries
    _INLINE int quadGetNumIrregularVertices(const int quadID) const {
      const int edge = quadFirstEdge(quadID);
      int num = 0;
      num += vertexGetEdgeValence(start_vertex(edge+0)) != 4 ? 1 : 0;
      num += vertexGetEdgeValence(start_vertex(edge+1)) != 4 ? 1 : 0;
      num += vertexGetEdgeValence(start_vertex(edge+2)) != 4 ? 1 : 0;
      num += vertexGetEdgeValence(start_vertex(edge+3)) != 4 ? 1 : 0;
      return num;
    }

    
    _INLINE void vertexBindEdgeID(const int vertexID,const int edgeID)
    {
      M128_INT(vertex[vertexID],3) = edgeID;
    }

    _INLINE int vertexGetEdgeID(const int vertexID) const
    {
      return M128_INT(vertex[vertexID],3);
    }

    _INLINE int quadGetVertex(const int quadID,const int index) const
    {
      return start_vertex(quadFirstEdge(quadID)+index);
    }

    _INLINE int quadGetIrregularVertexIndex(const int quadID) const {
      const int edgeID = quadFirstEdge(quadID);
      for (int i=0;i<4;i++)
        if (!edge[edgeID+i].regular_vertex) return i;
      return -1;
    }

    _INLINE bool quadIsVertexRegular(const int quadID, const int i) const {
      const int edgeID = quadFirstEdge(quadID);
      return edge[edgeID+i].regular_vertex;
    }

    _INLINE bool quadIsRegular(const int quadID) const
    {
      const int edgeID = quadFirstEdge(quadID);
      const int regular = \
        edge[edgeID+0].regular_vertex &
        edge[edgeID+1].regular_vertex &
        edge[edgeID+2].regular_vertex &
        edge[edgeID+3].regular_vertex;	
      if (__builtin_expect(regular != 0,1)) return true;
      return false;
    }

    _INLINE bool quadHasBoundary(const int quadID) const
    {
      const int edgeID = quadFirstEdge(quadID);
      const int boundary = \
        edge[edgeID+0].boundary_vertex |
        edge[edgeID+1].boundary_vertex |
        edge[edgeID+2].boundary_vertex |
        edge[edgeID+3].boundary_vertex;
      if (__builtin_expect(boundary != 0,1)) return true;
      return false;
    }

    _INLINE bool quadIsRegularWithoutBoundary(const int quadID) const
    {
      return quadIsRegular(quadID) == true && quadHasBoundary(quadID) == false;
    }
  };

};

#endif
