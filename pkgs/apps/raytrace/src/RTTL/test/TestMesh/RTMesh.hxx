#include "RTTL/common/RTBox.hxx"
using namespace RTTL;

#include <stdarg.h>
#include <vector>

// If defined, stl::vectors will be used for arrays inside mesh.
//#define STL_MESH_VECTORS
#undef  STL_MESH_VECTORS

namespace RTTL {

// Features are either
#define RT_NOTSET     0x0 // feature is not used or computed on the fly
#define RT_EMBEDDED   0x1 // feature is directly present in the triangle layout as it is
#define RT_INDEXED    0x2 // individual index for the given feature exists, which points out into mesh storage
#define RT_COMMON     0x3 // implicit common index (only one) for the given feature is defined by triangle #
#define RT_VERTEXDATA 0x4 // feature exists inside the appropriate vertex layout ('fat' vertex)

// True if array of c-entities is defined in the mesh.
#define RTMESH_COMPONENT(c) ((c) == RT_INDEXED || (c) == RT_COMMON)

// Unique descriptor containing all RT_* layout keywords at specific places.
// It is used only to differentiate between different mesh layouts, nothing else.

    enum {
        RT_VERTEX_TYPE,
        RT_TRIANGLE_TYPE,
        RT_SPHERE_TYPE,
        RT_IMPLICIT_TYPE,
        RT_QUADRIC_TYPE
    } RTPrimitiveTypes;

    class RTMaterial {
    public:
        // Borrowing some definitions from Java 3D (does it make any sense?)

        // Ambient color - the ambient RGB color reflected off the surface of the material.
        // The range of values is 0.0 to 1.0. The default ambient color is (0.2, 0.2, 0.2).

        // Diffuse color - the RGB color of the material when illuminated.
        // The range of values is 0.0 to 1.0. The default diffuse color is (1.0, 1.0, 1.0).

        // Specular color - the RGB specular color of the material (highlights).
        // The range of values is 0.0 to 1.0. The default specular color is (1.0, 1.0, 1.0).

        // Emissive color - the RGB color of the light the material emits, if any.
        // The range of values is 0.0 to 1.0. The default emissive color is (0.0, 0.0, 0.0).

        // Shininess - the material's shininess, in the range [1.0, 128.0]
        // with 1.0 being not shiny and 128.0 being very shiny.
        // Values outside this range are clamped. The default value for the material's shininess is 64.

    };
    _INLINE ostream& operator<<(ostream& out, const RTMaterial& m) {
        return out;
    }


    // =============================================================================
    // Basically, a pointer to array of Data types and its size.
    // If FirstIndex is 0, regular arrays are created,
    // If it is -1, then array indices start at -1 (and size is increased by 1).
    // =============================================================================
    #if defined(STL_MESH_VECTORS)
    template<typename Data, int FirstIndex = 0>
    class RTArray_t: public vector<Data> {
    public:
        RTArray_t(int n = 0) {
            // local one.
            reserve(n);
        }

        _INLINE       Data& operator[](int i)       { return vector<Data>::operator[](i - FirstIndex); }
        _INLINE const Data& operator[](int i) const { return vector<Data>::operator[](i - FirstIndex); }

        _INLINE int add(const Data& v) {
            push_back(v);
            return vector<Data>::size() - 1;
        }

        _INLINE int add() {
            return vector<Data>::size();
        }

        _INLINE int size() {
            return vector<Data>::size() - FirstIndex;
        }

        _INLINE void reserve(int n) {
            vector<Data>::reserve(n - FirstIndex);
        }
    };
    #else
    template<typename Data, int FirstIndex = 0>
    class RTArray_t {
    public:
        RTArray_t(int n = 0): m_used(0), m_capacity(n) {
            if (n) m_data = (new Data[n - FirstIndex]) - FirstIndex;
            else   m_data = 0;
        }
        ~RTArray_t() {
            reserve(0);
        }

        // operator[] ignores array boundaries (as is the basic one in C++ :)
        _INLINE       Data& operator[](int i)       { return m_data[i]; }
        _INLINE const Data& operator[](int i) const { return m_data[i]; }

        _INLINE int add(const Data& v) {
            m_data[add()] = v;
            return m_used;
        }

        _INLINE int add() {
            // Return index of the last entry+1, adjust sizes
            if (m_used == m_capacity) {
                m_capacity = m_capacity? 2*m_capacity : 4;
                Data* newdata = (new Data[m_capacity - FirstIndex]) - FirstIndex;
                memcpy(newdata + FirstIndex, m_data + FirstIndex, (m_used - FirstIndex) * sizeof(Data));
                delete [] (m_data + FirstIndex);
                m_data = newdata;
            }
            return m_used++;
        }

        _INLINE int size()     const { return m_used;     }
        _INLINE int capacity() const { return m_capacity; }

        _INLINE void reserve(int n) {
            if (m_capacity != n) {
                if (m_data) delete [] (m_data + FirstIndex);
                m_data = (m_capacity = n)? (new Data[n - FirstIndex]) - FirstIndex : 0;
            }
            m_used = 0;
        }
        _INLINE void clear() {
            m_used = 0;
        }

    protected:
        Data* m_data;
        int   m_used;
        int   m_capacity;
    };
    #endif

};

namespace RTTL {
    // Only RTTriangleMesh specializations (defined in "RTMesh.hxx") are allowed.
    template<long long mesh_descriptor>
    class RTTriangleMesh { private: RTTriangleMesh(); };
};

// =======================
// Indexed triangle layout
// =======================
#define RT_VERTEX_POSITION RT_INDEXED
#define RT_VERTEX_NORMAL   RT_INDEXED
#define RT_VERTEX_TEXTURE1 RT_INDEXED
#define RT_VERTEX_TEXTURE2 RT_NOTSET
#define RT_VERTEX_TEXTURE3 RT_NOTSET
#define RT_VERTEX_TEXTURE4 RT_NOTSET

#define RT_FACE_NORMAL     RT_INDEXED
#define RT_MATERIAL        RT_INDEXED

// Additional members (size in floats)
#define RT_VERTEX_DATA     4
#define RT_FACE_DATA       6

// Has to come in triples.
#include "RTMeshDescriptor.hxx"
static const int RT_INDEXED_MESH = RT_MESH_DESCRIPTOR;
#include "RTMeshInstantiation.hxx"

// =======================
// Fat vertex layout
// =======================
#define RT_VERTEX_POSITION RT_INDEXED
#define RT_VERTEX_NORMAL   RT_VERTEXDATA
#define RT_VERTEX_TEXTURE1 RT_VERTEXDATA
#define RT_VERTEX_TEXTURE2 RT_NOTSET
#define RT_VERTEX_TEXTURE3 RT_NOTSET
#define RT_VERTEX_TEXTURE4 RT_NOTSET

#define RT_FACE_NORMAL     RT_INDEXED
#define RT_MATERIAL        RT_INDEXED

// Has to come in triples.
#include "RTMeshDescriptor.hxx"
static const int RT_FAT_VERTEX_MESH = RT_MESH_DESCRIPTOR;
#include "RTMeshInstantiation.hxx"

// =======================
// Fat triangle layout
// =======================
#define RT_VERTEX_POSITION RT_EMBEDDED
#define RT_VERTEX_NORMAL   RT_EMBEDDED
#define RT_VERTEX_TEXTURE1 RT_EMBEDDED
#define RT_VERTEX_TEXTURE2 RT_NOTSET
#define RT_VERTEX_TEXTURE3 RT_NOTSET
#define RT_VERTEX_TEXTURE4 RT_NOTSET

#define RT_FACE_NORMAL     RT_EMBEDDED
#define RT_MATERIAL        RT_INDEXED

// Has to come in triples.
#include "RTMeshDescriptor.hxx"
static const int RT_FAT_TRIANGLE_MESH = RT_MESH_DESCRIPTOR;
#include "RTMeshInstantiation.hxx"


#include "RTObjReader.hxx"
