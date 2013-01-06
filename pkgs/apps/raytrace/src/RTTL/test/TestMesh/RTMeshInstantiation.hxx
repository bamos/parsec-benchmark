// This file allows multiple includes.

#if !defined(RT_VERTEX_POSITION) || !defined(RT_VERTEX_NORMAL) || !defined(RT_VERTEX_TEXTURE1) || !defined(RT_VERTEX_TEXTURE2) || !defined(RT_VERTEX_TEXTURE3) || !defined(RT_VERTEX_TEXTURE4) || !defined(RT_FACE_NORMAL) || !defined(RT_MATERIAL) || !defined(RT_MATERIAL)
#error not all required macros are defined in RTMesh.hxx invocation.
#endif

// Just to allow warning-less compilation of some pathological cases
// (which shoudn't be used anyway).
#pragma warning(push)
#ifdef __INTEL_COMPILER
#pragma warning(disable: 504)  // initial value of reference to non-const must be an lvalue
#pragma warning(disable: 473)  // returning reference to local temporary
#endif
#ifdef _MSC_VER
#pragma warning(disable: 4172) // returning address of local variable or temporary
#endif

// Just to make the coding simpler...
#if RTMESH_COMPONENT(RT_VERTEX_POSITION)
#define STORE_VERTEX_POSITION(s) s
#else
#define STORE_VERTEX_POSITION(s)
#endif
#if RTMESH_COMPONENT(RT_VERTEX_NORMAL)
#define STORE_VERTEX_NORMAL(s) s
#else
#define STORE_VERTEX_NORMAL(s)
#endif
#if RTMESH_COMPONENT(RT_VERTEX_TEXTURE1)
#define STORE_VERTEX_TEXTURE1(s) s
#else
#define STORE_VERTEX_TEXTURE1(s)
#endif
#if RTMESH_COMPONENT(RT_VERTEX_TEXTURE2)
#define STORE_VERTEX_TEXTURE2(s) s
#else
#define STORE_VERTEX_TEXTURE2(s)
#endif
#if RTMESH_COMPONENT(RT_VERTEX_TEXTURE2)
#define STORE_VERTEX_TEXTURE2(s) s
#else
#define STORE_VERTEX_TEXTURE2(s)
#endif
#if RTMESH_COMPONENT(RT_VERTEX_TEXTURE4)
#define STORE_VERTEX_TEXTURE4(s) s
#else
#define STORE_VERTEX_TEXTURE4(s)
#endif
#if RTMESH_COMPONENT(RT_FACE_NORMAL)
#define STORE_FACE_NORMAL(s) s
#else
#define STORE_FACE_NORMAL(s)
#endif
#if RTMESH_COMPONENT(RT_MATERIAL)
#define STORE_MATERIAL(s) s
#else
#define STORE_MATERIAL(s)
#endif

// Size for the additional vertex/traingle members 
// (beyond RT_* semanthics), defaulted to 0.
#ifndef RT_VERTEX_DATA
#define RT_VERTEX_DATA 0
#endif
#ifndef RT_FACE_DATA
#define RT_FACE_DATA 0
#endif

// Data types for indices and data 
// (defaulted to int and float if not defined in the includer).
#ifndef IndexType
#define IndexType int
#endif
#ifndef FloatType
#define FloatType float
#endif

namespace RTTL {

    /// \class RTTriangleMesh
    /// Vertices (for indexed meshes), primitives (triangles),
    /// texture coordinates, materials, and geometric normals.
    /// Some of the entries might never be used.
    ///
    /// We will define here a partially specialized instantiation of RTTriangleMesh
    /// (with template parameter equal to RT_MESH_DESCRIPTOR).
    /// Non-specialized instantiations of RTTriangleMesh are prohibited by law.
    template<>
    class RTTriangleMesh<RT_MESH_DESCRIPTOR> {
    public:

        // 2 possibilities:
        typedef RTVec_t<3, FloatType> vec3f;
        // typedef RTVec_t<3, FloatType, RTAlignedData_t<3, FloatType> > vec3f;

        typedef RTVec_t<2, FloatType> vec2f;
        typedef RTVec_t<3, IndexType> vec3i;

        // Vertices do not exist in 'fat triangle' layout.
        #if RTMESH_COMPONENT(RT_VERTEX_POSITION)
        #if defined(STL_MESH_VECTORS)
        typedef RTVec_t<VERTEX_SIZE, FloatType,  0> VertexAsVector;
        #else
        typedef RTVec_t<VERTEX_SIZE, FloatType, 16> VertexAsVector;
        _ALIGN(16) // will not work with stl::vector (at least for current stl implementations).
        #endif
        class Vertex {
        public:
        
            /// Empty default ctor (still need it). There will be other ctors down below.
            Vertex() {}
        
            /// static typing -- it might be necessary to change it to dynamic at certain stage
            /// though it will be unfortunate...
            static const int m_type = RT_VERTEX_TYPE;
            static _INLINE int type() { return m_type; }

            /// Return # of floats in Vertex.
            static _INLINE int size() { return VERTEX_SIZE; }
        
            /// Linearization: overloaded operator* facilitates arithmetic ops over vertices.
            const VertexAsVector& operator*() const { return *(VertexAsVector*)this; }
            VertexAsVector&       operator*()       { return *(VertexAsVector*)this; }
            Vertex(const VertexAsVector& vd) {
                *(VertexAsVector*)this = vd;
            }
            explicit Vertex(const FloatType* v, int n = VERTEX_SIZE) {
                // Only if it is really really needed...
                memcpy((FloatType*)this, v, sizeof(FloatType) * n);
            }
            const VertexAsVector& operator=(const VertexAsVector& vd) {
                return *(VertexAsVector*)this = vd;
            }

            // These are public members though should rarely be accessed directly
            // (better use RTTriangleMesh access member functions).

            #if   RT_VERTEX_POSITION == RT_INDEXED
            vec3f m_vertex_position;
            #endif
            #if RT_VERTEX_NORMAL   == RT_VERTEXDATA
            vec3f m_vertex_normal;
            #endif
            #if RT_VERTEX_TEXTURE1 == RT_VERTEXDATA
            vec2f m_vertex_texture1;
            #endif
            #if RT_VERTEX_TEXTURE2 == RT_VERTEXDATA
            vec2f m_vertex_texture2;
            #endif
            #if RT_VERTEX_TEXTURE3 == RT_VERTEXDATA
            vec2f m_vertex_texture3;
            #endif
            #if RT_VERTEX_TEXTURE4 == RT_VERTEXDATA
            vec2f m_vertex_texture4;
            #endif
            // Application-specific data
            #if RT_VERTEX_DATA > 0
            FloatType m_vertex_data[RT_VERTEX_DATA];
            #endif
        };
        #endif
        
        class Triangle {
        public:

            // These are public members though should rarely be accessed directly
            // (better use RTTriangleMesh access member functions).

            // Per vertex data
            #if   RT_VERTEX_POSITION == RT_INDEXED
            vec3i m_vertex_position_index;
            #elif RT_VERTEX_POSITION == RT_EMBEDDED
            vec3f m_vertex_position[3];
            #endif
            #if   RT_VERTEX_NORMAL   == RT_INDEXED
            vec3i m_vertex_normal_index;
            #elif RT_VERTEX_NORMAL   == RT_EMBEDDED
            vec3f m_vertex_normal[3];
            #endif
            #if   RT_VERTEX_TEXTURE1 == RT_INDEXED
            vec3i m_vertex_texture1_index;
            #elif RT_VERTEX_TEXTURE1 == RT_EMBEDDED
            vec2f m_vertex_texture1[3];
            #endif
            #if   RT_VERTEX_TEXTURE2 == RT_INDEXED
            vec3i m_vertex_texture2_index;
            #elif RT_VERTEX_TEXTURE2 == RT_EMBEDDED
            vec2f m_vertex_texture2[3];
            #endif
            #if   RT_VERTEX_TEXTURE2 == RT_INDEXED
            vec3i m_vertex_texture3_index;
            #elif RT_VERTEX_TEXTURE3 == RT_EMBEDDED
            vec2f m_vertex_texture3[3];
            #endif
            #if   RT_VERTEX_TEXTURE4 == RT_INDEXED
            vec3i m_vertex_texture4_index;
            #elif RT_VERTEX_TEXTURE4 == RT_EMBEDDED
            vec2f m_vertex_texture4[3];
            #endif
        
            // Per triangle data
            #if   RT_FACE_NORMAL     == RT_INDEXED
            IndexType m_face_normal_index;
            #elif RT_FACE_NORMAL     == RT_EMBEDDED
            vec3f m_face_normal;
            #endif
            #if   RT_MATERIAL        == RT_INDEXED
            IndexType m_material_index;
            #elif RT_MATERIAL        == RT_EMBEDDED
            RTMaterial m_material;
            #endif
            // Application-specific data
            #if RT_FACE_DATA > 0
            FloatType m_data[RT_FACE_DATA];
            #endif
        };

        // RTTriangleMesh() {}
        // ~RTTriangleMesh() {}

        long long descriptor() const { return RT_MESH_DESCRIPTOR; }

        _INLINE void clear() {
            // reserve(0) forces delete.
            m_triangle.reserve(0);
            STORE_VERTEX_POSITION(m_vertex.reserve(0));
            STORE_VERTEX_NORMAL  (m_vertex_normal.reserve(0));
            STORE_VERTEX_TEXTURE1(m_vertex_texture1.reserve(0));
            STORE_VERTEX_TEXTURE2(m_vertex_texture2.reserve(0));
            STORE_VERTEX_TEXTURE2(m_vertex_texture3.reserve(0));
            STORE_VERTEX_TEXTURE4(m_vertex_texture4.reserve(0));
            STORE_FACE_NORMAL    (m_face_normal.reserve(0));
            STORE_MATERIAL       (m_material.reserve(0));
        }

        /// Better not to use these 2 functions directly.
        Triangle&       triangle(int ti)       { return m_triangle[ti]; }
        const Triangle& triangle(int ti) const { return m_triangle[ti]; }

        template<typename T>
        _INLINE T* vertexData(int ti, int vi) {
            #if   RT_VERTEX_DATA > 0
            #if   RT_VERTEX_POSITION == RT_INDEXED
            int i = m_triangle[ti].m_vertex_position_index[vi];
            return (T*)m_vertex[i].m_vertex_data;
            #elif RT_VERTEX_POSITION == RT_COMMON
            int i = 3*ti+vi;
            return (T*)m_vertex[i].m_vertex_data;
            #else
            FATAL("vertexData");
            #endif
            #else
            return 0;
            #endif
        }
        template<typename T>
        _INLINE T* vertexData(int vi) {
            #if RT_FACE_DATA > 0
            return (T*)m_vertex[vi].m_vertex_data;
            #else
            return 0;
            #endif
        }
        template<typename T>
        _INLINE T* triangleData(int ti) {
            #if RT_FACE_DATA > 0
            return (T*)m_triangle[ti].m_data;
            #else
            return 0;
            #endif
        }

        // const access functions are implemented through casting to non-const ones,
        // which may not be up to the highest standards, but saves space.

        _INLINE const vec3f& vertexPosition(int ti, int vi) const { return (const vec3f&)((RTTriangleMesh*)this)->vertexPosition(ti, vi); }
        _INLINE vec3f& vertexPosition(int ti, int vi) {
            #if   RT_VERTEX_POSITION == RT_EMBEDDED
            return m_triangle[ti].m_vertex_position[vi];
            #elif RT_VERTEX_POSITION == RT_INDEXED
            int i = m_triangle[ti].m_vertex_position_index[vi];
            return m_vertex[i].m_vertex_position;
            #elif RT_VERTEX_POSITION == RT_COMMON
            return m_vertex[3*ti+vi].m_vertex_position;
            #endif
        }

        _INLINE const vec3f& vertexNormal(int ti, int vi) const { return (const vec3f&)((RTTriangleMesh*)this)->vertexNormal(ti, vi); }
        _INLINE vec3f& vertexNormal(int ti, int vi) {
            #if   RT_VERTEX_NORMAL   == RT_EMBEDDED
            return m_triangle[ti].m_vertex_normal[vi];
            #elif RT_VERTEX_NORMAL   == RT_INDEXED
            int i = m_triangle[ti].m_vertex_normal_index[vi];
            return m_vertex_normal[i];
            #elif RT_VERTEX_NORMAL   == RT_COMMON
            return m_vertex_normal[3*ti+vi];
            #elif RT_VERTEX_NORMAL   == RT_VERTEXDATA
            int i = m_triangle[ti].m_vertex_position_index[vi];
            return m_vertex[i].m_vertex_normal;
            #endif
        }

        _INLINE const vec3f& faceNormal(int ti) const { return (const vec3f&)((RTTriangleMesh*)this)->faceNormal(ti); }
        _INLINE vec3f& faceNormal(int ti) {
            #if   RT_FACE_NORMAL   == RT_EMBEDDED
            return m_triangle[ti].m_face_normal;
            #elif RT_FACE_NORMAL   == RT_INDEXED
            int i = m_triangle[ti].m_face_normal_index;
            return m_face_normal[i];
            #elif RT_FACE_NORMAL   == RT_COMMON
            return m_face_normal[ti];
            #elif RT_FACE_NORMAL   == RT_NOTSET
            const vec3f& v0 = vertexPosition(ti, 0);
            const vec3f& v1 = vertexPosition(ti, 1);
            const vec3f& v2 = vertexPosition(ti, 2);
            return (v0 - v1) ^ (v0 - v2);
            #endif
        }

        /// Per-triangle material.
        _INLINE const RTMaterial& material(int ti) const { return (const RTMaterial&)((RTTriangleMesh*)this)->material(ti); }
        _INLINE RTMaterial& material(int ti) {
            #if   RT_MATERIAL == RT_EMBEDDED
            return m_triangle[ti].m_material;
            #elif RT_MATERIAL == RT_INDEXED
            int i = m_triangle[ti].m_material_index;
            return m_material[i];
            #elif RT_MATERIAL == RT_COMMON
            return m_material[ti];
            #endif
        }

        /// There isn't anything better than replication...
        _INLINE const vec2f& texture1(int ti, int vi) const { return (const vec2f&)((RTTriangleMesh*)this)->texture1(ti, vi); }
        _INLINE vec2f& texture1(int ti, int vi) {
            #if   RT_VERTEX_TEXTURE1   == RT_EMBEDDED
            return m_triangle[ti].m_vertex_texture1[vi];
            #elif RT_VERTEX_TEXTURE1   == RT_INDEXED
            int i = m_triangle[ti].m_vertex_texture1_index[vi];
            return m_vertex_texture1[i];
            #elif RT_VERTEX_TEXTURE1   == RT_COMMON
            return m_vertex_texture1[3*ti+vi];
            #elif RT_VERTEX_TEXTURE1   == RT_VERTEXDATA
            int i = m_triangle[ti].m_vertex_position_index[vi];
            return m_vertex[i].m_vertex_texture1;
            #elif RT_VERTEX_TEXTURE1   == RT_NOTSET
            return vec2f(-1, -1);
            #endif
        }
        _INLINE const vec2f& texture2(int ti, int vi) const { return (const vec2f&)((RTTriangleMesh*)this)->texture2(ti, vi); }
        _INLINE vec2f& texture2(int ti, int vi) {
            #if   RT_VERTEX_TEXTURE2   == RT_EMBEDDED
            return m_triangle[ti].m_vertex_texture2[vi];
            #elif RT_VERTEX_TEXTURE2   == RT_INDEXED
            int i = m_triangle[ti].m_vertex_texture2_index[vi];
            return m_vertex_texture2[i];
            #elif RT_VERTEX_TEXTURE2   == RT_COMMON
            return m_vertex_texture2[3*ti+vi];
            #elif RT_VERTEX_TEXTURE2   == RT_VERTEXDATA
            int i = m_triangle[ti].m_vertex_position_index[vi];
            return m_vertex[i].m_vertex_texture2;
            #else
            return vec2f(-1, -1);
            #endif
        }
        _INLINE const vec2f& texture3(int ti, int vi) const { return (const vec2f&)((RTTriangleMesh*)this)->texture3(ti, vi); }
        _INLINE vec2f& texture3(int ti, int vi) {
            #if   RT_VERTEX_TEXTURE3   == RT_EMBEDDED
            return m_triangle[ti].m_vertex_texture3[vi];
            #elif RT_VERTEX_TEXTURE3   == RT_INDEXED
            int i = m_triangle[ti].m_vertex_texture3_index[vi];
            return m_vertex_texture3[i];
            #elif RT_VERTEX_TEXTURE3   == RT_COMMON
            return m_vertex_texture3[3*ti+vi];
            #elif RT_VERTEX_TEXTURE3   == RT_VERTEXDATA
            int i = m_triangle[ti].m_vertex_position_index[vi];
            return m_vertex[i].m_vertex_texture3;
            #else
            return vec2f(-1, -1);
            #endif
        }
        _INLINE const vec2f& texture4(int ti, int vi) const { return (const vec2f&)((RTTriangleMesh*)this)->texture4(ti, vi); }
        _INLINE vec2f& texture4(int ti, int vi) {
            #if   RT_VERTEX_TEXTURE4   == RT_EMBEDDED
            return m_triangle[ti].m_vertex_texture4[vi];
            #elif RT_VERTEX_TEXTURE4   == RT_INDEXED
            int i = m_triangle[ti].m_vertex_texture4_index[vi];
            return m_vertex_texture4[i];
            #elif RT_VERTEX_TEXTURE4   == RT_COMMON
            return m_vertex_texture4[3*ti+vi];
            #elif RT_VERTEX_TEXTURE4   == RT_VERTEXDATA
            int i = m_triangle[ti].m_vertex_position_index[vi];
            return m_vertex[i].m_vertex_texture4;
            #else
            return vec2f(-1, -1);
            #endif
        }

        _INLINE int numberOfTriangles() { return m_triangle.size(); }

        STORE_VERTEX_POSITION(_INLINE int numberOfVertices()            { return m_vertex.size(); })
        STORE_VERTEX_NORMAL  (_INLINE int numberOfVertexNormals()       { return m_vertex_normal.size(); })
        STORE_VERTEX_TEXTURE1(_INLINE int numberOfTextureCoordinates1() { return m_vertex_texture1.size(); })
        STORE_VERTEX_TEXTURE2(_INLINE int numberOfTextureCoordinates2() { return m_vertex_texture2.size(); })
        STORE_VERTEX_TEXTURE2(_INLINE int numberOfTextureCoordinates3() { return m_vertex_texture3.size(); })
        STORE_VERTEX_TEXTURE4(_INLINE int numberOfTextureCoordinates4() { return m_vertex_texture4.size(); })

        STORE_FACE_NORMAL    (_INLINE int numberOfFaceNormals()         { return m_face_normal.size(); })
        STORE_MATERIAL       (_INLINE int numberOfMaterials()           { return m_material.size(); })

        /// Reservations are not necessary, but advised :)
        _INLINE void setNumberOfTriangles(int n) { m_triangle.reserve(n); }

        STORE_VERTEX_POSITION(_INLINE void setNumberOfVertices(int n)            { m_vertex.reserve(n); })
        STORE_VERTEX_NORMAL  (_INLINE void setNumberOfVertexNormals(int n)       { m_vertex_normal.reserve(n); })
        STORE_VERTEX_TEXTURE1(_INLINE void setNumberOfTextureCoordinates1(int n) { m_vertex_texture1.reserve(n); })
        STORE_VERTEX_TEXTURE2(_INLINE void setNumberOfTextureCoordinates2(int n) { m_vertex_texture2.reserve(n); })
        STORE_VERTEX_TEXTURE2(_INLINE void setNumberOfTextureCoordinates3(int n) { m_vertex_texture3.reserve(n); })
        STORE_VERTEX_TEXTURE4(_INLINE void setNumberOfTextureCoordinates4(int n) { m_vertex_texture4.reserve(n); })

        STORE_FACE_NORMAL    (_INLINE void setNumberOfFaceNormals(int n)         { m_face_normal.reserve(n); })
        STORE_MATERIAL       (_INLINE void setNumberOfMaterials(int n)           { m_material.reserve(n); })

        // Multiple flavors of addSomething functions.

        int addVertexPosition(const vec3f& v0, int vi = -1) {
            #if RT_VERTEX_POSITION == RT_EMBEDDED 
            FATAL("addVertexPosition");
            #else
            if (vi == -1) vi = m_vertex.add();
            m_vertex[vi].m_vertex_position = v0;
            #endif
            return vi;
        }

        int addVertexNormal(const vec3f& v0, int vi = -1) {
            #if RT_VERTEX_NORMAL == RT_EMBEDDED 
            FATAL("addVertexNormal");
            #elif RT_VERTEX_NORMAL == RT_COMMON || RT_VERTEX_NORMAL == RT_INDEXED
            if (vi == -1) vi = m_vertex_normal.add();
            m_vertex_normal[vi] = v0;
            #elif RT_VERTEX_NORMAL == RT_VERTEXDATA
            if (vi == -1) vi = m_vertex.add();
            m_vertex[vi].m_vertex_normal = v0;
            #endif
            return vi;
        }

        int addVertexTexture1(const vec2f& v0, int vi = -1) {
            #if   RT_VERTEX_TEXTURE1 == RT_EMBEDDED 
            FATAL("addVertexTexture1");
            #elif RT_VERTEX_TEXTURE1 == RT_COMMON || RT_VERTEX_TEXTURE1 == RT_INDEXED
            if (vi == -1) vi = m_vertex_texture1.add();
            m_vertex_texture1[vi] = v0;
            #elif RT_VERTEX_TEXTURE1 == RT_VERTEXDATA
            if (vi == -1) vi = m_vertex.add();
            m_vertex[vi].m_vertex_texture1 = v0;
            #endif
            return vi;
        }
        int addVertexTexture2(const vec2f& v0, int vi = -1) {
            #if   RT_VERTEX_TEXTURE2 == RT_EMBEDDED 
            FATAL("addVertexTexture2");
            #elif RT_VERTEX_TEXTURE2 == RT_COMMON || RT_VERTEX_TEXTURE2 == RT_INDEXED
            if (vi == -1) vi = m_vertex_texture2.add();
            m_vertex_texture2[vi] = v0;
            #elif RT_VERTEX_TEXTURE2 == RT_VERTEXDATA
            if (vi == -1) vi = m_vertex.add();
            m_vertex[vi].m_vertex_texture2 = v0;
            #endif
            return vi;
        }
        int addVertexTexture3(const vec2f& v0, int vi = -1) {
            #if   RT_VERTEX_TEXTURE3 == RT_EMBEDDED 
            FATAL("addVertexTexture3");
            #elif RT_VERTEX_TEXTURE3 == RT_COMMON || RT_VERTEX_TEXTURE3 == RT_INDEXED
            if (vi == -1) vi = m_vertex_texture3.add();
            m_vertex_texture3[vi] = v0;
            #elif RT_VERTEX_TEXTURE3 == RT_VERTEXDATA
            if (vi == -1) vi = m_vertex.add();
            m_vertex[vi].m_vertex_texture3 = v0;
            #endif
            return vi;
        }
        int addVertexTexture4(const vec2f& v0, int vi = -1) {
            #if   RT_VERTEX_TEXTURE4 == RT_EMBEDDED 
            FATAL("addVertexTexture4");
            #elif RT_VERTEX_TEXTURE4 == RT_COMMON || RT_VERTEX_TEXTURE4 == RT_INDEXED
            if (vi == -1) vi = m_vertex_texture4.add();
            m_vertex_texture4[vi] = v0;
            #elif RT_VERTEX_TEXTURE4 == RT_VERTEXDATA
            if (vi == -1) vi = m_vertex.add();
            m_vertex[vi].m_vertex_texture4 = v0;
            #endif
            return vi;
        }

        /// Some possible combinations are implemented here;
        /// all other data could be added by obtianing index with addTriangle()
        /// and then using data access functions for specific layouts.
        /// Indices:
        /// ti -- triangle index
        /// vi -- vertex position index
        /// ni -- vertex normal index
        /// mi -- material index
        /// fi -- face normal index

        _INLINE int addTriangle() {
            // Return index of the next triangle.
            return m_triangle.add();
        }
        _INLINE int addTriangle(int i0, int i1, int i2) {
            return addTriangle(RTVec3i(i0, i1, i2));
        }
        _INLINE int addTriangle(const RTVec3i& vi) {
            int ti = addTriangle();
            #if RT_VERTEX_POSITION == RT_INDEXED
            m_triangle[ti].m_vertex_position_index = vi;
            #endif
            return ti;
        }
        _INLINE int addTriangle(const RTVec3i& vi, int mi) {
            int ti = addTriangle(vi);
            #if RT_MATERIAL == RT_INDEXED
            m_triangle[ti].m_material_index = mi;
            #endif
            return ti;
        }
        _INLINE int addTriangle(const RTVec3i& vi, const RTVec3i& ni) {
            int ti = addTriangle(vi);
            #if RT_VERTEX_NORMAL == RT_INDEXED
            m_triangle[ti].m_vertex_normal_index = vi;
            #endif
            return ti;
        }
        _INLINE int addTriangle(const RTVec3i& vi, const RTVec3i& ni, int mi) {
            int ti = addTriangle(vi, ni);
            #if RT_VERTEX_NORMAL == RT_INDEXED
            m_triangle[ti].m_vertex_normal_index = vi;
            #endif
            return ti;
        }
        _INLINE int addTriangleNormal(int ti = -1) {
            if (ti == -1) {
                // Use the last triangle if ti is not defined.
                ti = m_triangle.size() - 1;
            }
            // Compute it on the fly and store in apropriate place.
            const vec3f& v0 = vertexPosition(ti, 0);
            const vec3f& v1 = vertexPosition(ti, 1);
            const vec3f& v2 = vertexPosition(ti, 2);
            vec3f n = (v0 - v1) ^ (v0 - v2);
            // n.normalize(); // do we care?
            return addTriangleNormal(ti, n);
        }
        _INLINE int addTriangleNormal(int ti, const vec3f& n) {
            // Store it in apropriate place.
            #if   RT_FACE_NORMAL     == RT_INDEXED
            m_triangle[ti].m_face_normal_index = m_face_normal.add(n);
            #elif RT_FACE_NORMAL     == RT_EMBEDDED
            m_triangle[ti].m_face_normal = n;
            #endif
            return ti;
        }
        _INLINE int addTriangleNormal(int ti, int fi) {
            // Store it in apropriate place.
            #if   RT_FACE_NORMAL     == RT_INDEXED
            m_triangle[ti].m_face_normal_index = fi;
            #elif RT_FACE_NORMAL     == RT_EMBEDDED
            FATAL("addTriangleNormal");
            #endif
            return ti;
        }


    protected:
        // Mesh components (defined by RT_* macro definitions in the caller, like RTmesh.hxx).
        // Not using plural noons since all these entries are arrays.

        /// Triangles
        RTArray_t<Triangle>   m_triangle;

        /// Vertices (indexed in m_triangle).
        #if   RTMESH_COMPONENT(RT_VERTEX_POSITION)
        RTArray_t<Vertex>     m_vertex;
        #endif

        /// Extra vertex data (indexed in m_triangle or implicitly).
        /// If it is not here, than it is inside m_vertex or RT_NOTSET).
        #if   RTMESH_COMPONENT(RT_VERTEX_NORMAL)
        RTArray_t<vec3f>      m_vertex_normal;
        #endif
        #if   RTMESH_COMPONENT(RT_VERTEX_TEXTURE1)
        RTArray_t<vec2f, -1>  m_vertex_texture1;
        #endif
        #if   RTMESH_COMPONENT(RT_VERTEX_TEXTURE2)
        RTArray_t<vec2f, -1>  m_vertex_texture2;
        #endif
        #if   RTMESH_COMPONENT(RT_VERTEX_TEXTURE2)
        RTArray_t<vec2f, -1>  m_vertex_texture3;
        #endif
        #if   RTMESH_COMPONENT(RT_VERTEX_TEXTURE4)
        RTArray_t<vec2f, -1>  m_vertex_texture4;
        #endif

        /// Extra vertex data (indexed in m_triangle or implicitly).
        /// If it is not here, than it is inside m_vertex or RT_NOTSET).
        #if   RTMESH_COMPONENT(RT_FACE_NORMAL)
        RTArray_t<vec3f>      m_face_normal;
        #endif
        #if   RTMESH_COMPONENT(RT_MATERIAL)
        RTArray_t<RTMaterial, -1> m_material;
        #endif
    };

    /// Streaming.
    #if   RTMESH_COMPONENT(RT_VERTEX_POSITION)
    _INLINE ostream& operator<<(ostream& out, const RTTriangleMesh<RT_MESH_DESCRIPTOR>::Vertex& v) {
        FloatType* data = (FloatType*)&v;

        bool not_defined = false;
        int i;
        for (i = 0; i < v.size(); i++) not_defined |= NOTINITIALIZED(data[i]);
        if (not_defined) {
            out << "[not defined]";
            goto done;
        }

        out << "[" << data[0];
        for (i = 1; i < v.size(); i++) {
            not_defined |= NOTINITIALIZED(data[i]);
            out << "," << (i!=3 && i!=6? " ":"\t");
            out << data[i];
        }
        out << "]";

    done:
        return out;
    }
    #endif

    _INLINE ostream& operator<<(ostream& out, const RTTriangleMesh<RT_MESH_DESCRIPTOR>::Triangle& t) {
        // There is a limited output functionality for a triangle without underlying mesh,
        // mostly for debugging.

        // Per vertex data
        #if   RT_VERTEX_POSITION == RT_INDEXED
        out << "m_vertex_position_index\t= ";
        if (NOTINITIALIZED(t.m_vertex_position_index))
            out << "not defined\n";
        else
            out << t.m_vertex_position_index << endl;
        #elif RT_VERTEX_POSITION == RT_EMBEDDED
        out << "m_vertex_position\t = [";
        if (NOTINITIALIZED(t.m_vertex_position[0]) || NOTINITIALIZED(t.m_vertex_position[1]) || NOTINITIALIZED(t.m_vertex_position[2]))
            out << "not defined\n";
        else
            out << t.m_vertex_position[0] << ", " << t.m_vertex_position[1] << ", " << t.m_vertex_position[2] << "]" << endl;
        #endif
        #if   RT_VERTEX_NORMAL   == RT_INDEXED
        out << "m_vertex_normal_index\t = ";
        if (NOTINITIALIZED(t.m_vertex_normal_index))
            out << "not defined\n";
        else
            out << t.m_vertex_normal_index << endl;
        #elif RT_VERTEX_NORMAL   == RT_EMBEDDED
        out << "m_vertex_normal\t = [";
        if (NOTINITIALIZED(t.m_vertex_normal[0]) || NOTINITIALIZED(t.m_vertex_normal[1]) || NOTINITIALIZED(t.m_vertex_normal[2]))
            out << "not defined]\n";
        else
            out << t.m_vertex_normal[0] << ", " << t.m_vertex_normal[1] << ", " << t.m_vertex_normal[2] << "]" << endl;
        #endif
        #if   RT_VERTEX_TEXTURE1 == RT_INDEXED
        out << "m_vertex_texture1_index\t = ";
        if (NOTINITIALIZED(t.m_vertex_texture1_index))
            out << "not defined\n";
        else
            out << t.m_vertex_texture1_index << endl;
        #elif RT_VERTEX_TEXTURE1 == RT_EMBEDDED
        out << "m_vertex_texture1\t = [";
        if (NOTINITIALIZED(t.m_vertex_texture1[0]) || NOTINITIALIZED(t.m_vertex_texture1[1]))
            out << "not defined]\n";
        else
            out << t.m_vertex_texture1[0] << ", " << t.m_vertex_texture1[1] << "]" << endl;
        #endif
        #if   RT_VERTEX_TEXTURE2 == RT_INDEXED
        out << "m_vertex_texture2_index\t = ";
        if (NOTINITIALIZED(t.m_vertex_texture2_index))
            out << "not defined\n";
        else
            out << t.m_vertex_texture2_index << endl;
        #elif RT_VERTEX_TEXTURE2 == RT_EMBEDDED
        out << "m_vertex_texture2\t = [";
        if (NOTINITIALIZED(t.m_vertex_texture2[0]) || NOTINITIALIZED(t.m_vertex_texture2[1]))
            out << "not defined]\n";
        else
            out << t.m_vertex_texture2[0] << ", " << t.m_vertex_texture2[1] << ", " << t.m_vertex_texture2[2] << "]" << endl;
        #endif
        #if   RT_VERTEX_TEXTURE2 == RT_INDEXED
        out << "m_vertex_texture3_index\t = ";
        if (NOTINITIALIZED(t.m_vertex_texture1[0]) || NOTINITIALIZED(t.m_vertex_texture1[1]))
        if (NOTINITIALIZED(t.m_vertex_texture3_index))
            out << "not defined\n";
        else
            out << t.m_vertex_texture3_index << endl;
        #elif RT_VERTEX_TEXTURE3 == RT_EMBEDDED
        out << "m_vertex_texture3\t = [";
        if (NOTINITIALIZED(t.m_vertex_texture3[0]) || NOTINITIALIZED(t.m_vertex_texture3[1]))
            out << "not defined]\n";
        else
            out << t.m_vertex_texture3[0] << ", " << t.m_vertex_texture3[1] << ", " << t.m_vertex_texture3[2] << "]" << endl;
        #endif
        #if   RT_VERTEX_TEXTURE4 == RT_INDEXED
        out << "m_vertex_texture4_index\t = ";
        if (NOTINITIALIZED(t.m_vertex_texture4_index))
            out << "not defined\n";
        else
            out << t.m_vertex_texture4_index << endl;
        #elif RT_VERTEX_TEXTURE4 == RT_EMBEDDED
        out << "m_vertex_texture4\t = [";
        if (NOTINITIALIZED(t.m_vertex_texture4[0]) || NOTINITIALIZED(t.m_vertex_texture4[1]))
            out << "not defined]\n";
        else
            out << t.m_vertex_texture4[0] << ", " << t.m_vertex_texture4[1] << ", " << t.m_vertex_texture4[2] << "]" << endl;
        #endif
        
        // Per triangle data
        #if   RT_FACE_NORMAL     == RT_INDEXED
        out << "m_face_normal_index\t = ";
        if (NOTINITIALIZED(t.m_face_normal_index))
            out << "not defined\n";
        else
            out << t.m_face_normal_index << endl;
        #elif RT_FACE_NORMAL     == RT_EMBEDDED
        out << "m_face_normal\t = ";
        if (NOTINITIALIZED(t.m_face_normal.x) || NOTINITIALIZED(t.m_face_normal.y) || NOTINITIALIZED(t.m_face_normal.z))
            out << "[not defined]\n";
        else
            out << t.m_face_normal << endl;
        #endif
        #if   RT_MATERIAL        == RT_INDEXED
        out << "m_material_index\t = ";
        if (NOTINITIALIZED(t.m_material_index))
            out << "not defined\n";
        else
            out << t.m_material_index << endl;
        #elif RT_MATERIAL        == RT_EMBEDDED
        out << "m_material\t = ";
        if (NOTINITIALIZED(t.m_material))
            out << "not defined\n";
        else
            out << t.m_material << endl;
        #endif

        #if RT_FACE_DATA > 0
        out << "extra data\t = [";
        if (NOTINITIALIZED(t.m_data[0])) {
            out << "not defined]\n";
        } else {
            out << t.m_data[0];
            for (int i = 1; i < RT_FACE_DATA; i++) {
                out << ", " << t.m_data[i];
            }
            out << "]" << endl;
        }
        #endif

        return out;
    }

// Undefined these definitions to allow a new invocation of this file.
#undef RT_VERTEX_POSITION
#undef RT_VERTEX_NORMAL
#undef RT_VERTEX_TEXTURE1
#undef RT_VERTEX_TEXTURE2
#undef RT_VERTEX_TEXTURE3
#undef RT_VERTEX_TEXTURE4
#undef RT_FACE_NORMAL
#undef RT_MATERIAL

#undef STORE_VERTEX_POSITION
#undef STORE_VERTEX_NORMAL
#undef STORE_VERTEX_TEXTURE1
#undef STORE_VERTEX_TEXTURE2
#undef STORE_VERTEX_TEXTURE2
#undef STORE_VERTEX_TEXTURE4
#undef STORE_FACE_NORMAL
#undef STORE_MATERIAL

#undef RT_VERTEX_DATA
#undef RT_FACE_DATA

#undef IndexType
#undef FloatType

#pragma warning(pop)

};
