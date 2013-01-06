#ifndef RTTL_ISG_HXX
#define RTTL_ISG_HXX

/*! \file ISG.hxx descrbes the _I_nternal _S_cene _G_raph */

#include "RTTL/common/RTInclude.hxx"
#include "RTTL/common/RTVec.hxx"
#include "RTTL/common/RTMatrix.hxx"
#include "RTTL/common/RTBox.hxx"
#include "RTTL/common/Timer.hxx"
#include <cstring>
#include <vector>
#include <map>





namespace ISG
{
  extern int current_timestamp; /*!< to track whether a node is dirty ... */

  using RTTL::Matrix4x4;

  typedef RTTL::RTVec3i vec3i;
  typedef RTTL::RTVec3f vec3f;
  typedef RTTL::RTVec4i vec4i;
  typedef RTTL::RTVec4f vec4f;
  

  /*! base class for all nodes in the scene graph */
  class Node
  {
    int last_modified;
  public:
    Node() {};
    Node(Node *parent) {
      addParent(parent);
    };
    virtual ~Node()
    {};

    template<class nodetype>
    static void removeParent(nodetype *&from, Node *what)
    {
      if (from == NULL) 
        return;
      from->removeParent(what);
      if (from->parent.size() == 0) {
        delete from;
        from = NULL;
      };
    };
    void addParent(Node *node)
    { 
      if (find(parent.begin(),parent.end(),node) == parent.end())
        {
          parent.push_back(node); 
        }
    }
  
    void removeParent(Node *node)
    { 
      vector<Node*>::iterator where = find(parent.begin(),parent.end(),node);
      assert(where != parent.end()); /* otherwise the node was not a parent! */
      parent.erase(where);
    }
    
    /*! notification (usually by one of the children) that one of the
        children (or one of the children thereof) was changed. might
        eventually have to propagate more information on what was
        actually changed (transform, connectivity, vertex poss,
        creation/deletion, etc), but for now it's a single bool, and
        the renderer has to figure out what exactly was changed, and
        how to react to it... */
    virtual void childWasChanged(Node *value)
    {
      markAsDirty();
    };
    /*! check whether node is (already) marked as dirty. */
    bool markedAsDirty()
    { 
      return last_modified >= current_timestamp; 
    };
    /*! mark node as 'dirty' (i.e, 'something' has changed that
        requires update by the renderer), and notify all parents of
        the change, too */
    void markAsDirty()
    {
      if (!markedAsDirty()) {
        // not yet dirty, so mark it and notify all parents...
        last_modified = current_timestamp;
        for (int i=0; i<parent.size();i++)
          parent[i]->childWasChanged(this);
      }
    }
  protected:
    vector<Node *> parent; /*!< list of nodes referencing this node. node
                             gets automatically deleted if all parents
                             got deleted (if you want to avoid that,
                             get another node to reference this one.
                           
                             \note yes, this consumes addtl memory,
                             but if we want to propagate changes (e.g., in
                             bbox size) up to the root, we probably need
                             it
                           */
  };



  /*! transformation that may be attached to _any_ node. might
    eventually make special 'XfmNode' class, but right now we decided
    to be able to attach xfms to _any_ node 

    \warning matrix _must_ be invertible (but not checking that)
  */
  struct TransformNode : public Node
  {
    Matrix4x4 matrix;
    bool is_visible; /*!< for hiding the subtree ... */
    bool has_transform; /*!< if transform is _not_ the identity */

    TransformNode(Node *t)
    {
      target = t;
      t->addParent(this);
    };
    
    void setTransform(Matrix4x4 &m)
    { 
      has_transform = true; /*! actually, might set has_transform = m.isIdentity() */
      matrix = m;
    }
    void clearTransform()
    {
      has_transform = false; 
#ifdef DEBUG
      matrix.setIdentity();
#endif
    }
    Node *target;
  };




  struct GroupNode : public Node
  {
  public:
    void addChild(Node *node)
    { 
      children.push_back(node); 
      node->addParent(this);
    }
    /*! remove a child from children list */
    void removeChild(Node *node)
    {
      vector<Node*>::iterator where = find(children.begin(),children.end(),node);
      assert(where != children.end()); /* otherwise the node was not a parent! */
      children.erase(where);
      removeParent(node,this);
    }
  
    /*! find given node in child list, 'attach' a transform node to that
      child (i.e., create a new transform node for that child, and
      replace direct child ref with ref to transform */
    TransformNode *attachTransformTo(Node *child)
    {
      // find child reference
      vector<Node*>::iterator where = find(children.begin(),children.end(),child);
      assert(where != children.end()); /* otherwise the node was not a parent! */
      
      // create new xfm node
      TransformNode *xfm = new TransformNode(child);
    
      // replace direct child ref w/ ref to transform node
      removeChild(child);
      addChild(xfm);
      
      return xfm;
    }
    
    inline int getNumChildren() { return children.size(); };
    inline Node *getChild(int i)
    {
      assert(i >= 0 && i < children.size());
      return children[i];
    }
  protected:
    vector<Node *> children;
  };


  struct World;

  /*! node that does not need a parent to live. each root node has a
    "visible" flag that defines whether the node contains some
    actually visible geometry, or whether it is a "dummy" node for
    storing stuff that will only be referenced from other
    subtrees. (i.e., if you want top have 10 skinned instances of a
    given base mesh, it would be natural to have a "dummy" (i.e.,
    invisible) subtree that defines that base mesh, and reference that
    from 10 properly parameterized and transformed skinning shaders
    somewhere else in other, visible subtrees
  */
  struct RootNode : public GroupNode
  {
    /*! constructor, will eventually demand reference to a world it lives in */
    RootNode(/* World *world */) {};
  };



  /*! something that specifies a material _ID_ (or material
  pointer). The _core_ does not even have the concept of a
  (recurisve?) shader or material, or what the difference between
  shaders and materials are, not what the app should support
  etcpp. All the _core_ supports is the concept that there is
  something that is responsible for the appearance of an object, and
  that this is a unique thing.  */
  struct Material: public Node 
  { 
    union {
      int materialID; 
      void *materialPtr; 
    };
    
    Material(int materialID=0) 
      : materialID(materialID)
    {
    };
  };
  

 
  // can specialize certain types yourself if you like to ....
  template<class target_t, class source_t>
  _INLINE void single_convert(target_t &target, source_t &source)
  { FATAL("this kind of conversion ont implemented ... "); };

  template<>
  _INLINE void single_convert(vec4f &target, vec3f &source)
  { (vec3f&)target = source; }

  template<>
  _INLINE void single_convert(vec3f &target, vec4f &source)
  { target = (vec3f&)source; }

  template<class target_t, class source_t>
  _INLINE void array_convert(target_t *target, source_t *source, int count)
  {
    for (int i=0;i<count;i++)
      single_convert(target[i],source[i]);
  };


  /*! the main context for the entire world/scene graph. just to keep things together 

    a world can have multiple

  */
  struct World
  {
    vector<RootNode *> rootNode;

    static World *getDefaultWorld()
    { if (m_default == NULL) { World *w = new World; w->makeDefault(); }; return m_default; };
    virtual void makeDefault() { m_default = this; };

  private:
    static World *m_default;
  };

  /*! format of a mesh */
  typedef enum {
    RT_TRIANGLE_MESH, 
    RT_QUAD_MESH
  } MeshType;

  /*! if you add formats here, be sure to add a size entry into RTDataSizeOf ! */
  typedef enum {
    RT_ANY_FORMAT = 0/*! format not (yet) specified, accept all format */,
    RT_UCHAR,
    RT_BYTE,
    RT_INT,
    RT_INT2,
    RT_INT3,
    RT_INT4,
    RT_FLOAT,
    RT_FLOAT2,
    RT_FLOAT3,
    RT_FLOAT4 /*!< has to be 16-byte aligned */
  } DataFormat;

  /*! \warning ordering of entires has to correspond to enum DataFormat! */
  static const int RTDataSizeOf[] = 
    {
      1,
      1,
      1,
      1*sizeof(int),
      2*sizeof(int),
      3*sizeof(int),
      4*sizeof(int),
      1*sizeof(float),
      2*sizeof(float),
      3*sizeof(float),
      4*sizeof(float)
    };


  typedef enum {
    RT_RAW_DATA /*!< raw, unformatted data ... */,
    RT_COORDINATE /*!< a point in space, transfomed as a point */,
    RT_COORDS=RT_COORDINATE,
    RT_VECTOR,
    RT_NORMAL /*!< a normal, transfomred with the inverse of the transposed transform */,
    RT_TEXCOORD,
    RT_INDEX,
    RT_INDICES=RT_INDEX
  } SemanticType;
  
  /*! wrapper class for all "bulky" items of data that the ray tracer
      might need. the data array will store data items of a given
      semantic type (coord, normal, ...) in a specified format
      (RT_FLOAT3, RT_INT4, ...). <ul><li>Data arrays can be created
      and written to via the API, including automatic format
      conversions, and, eventually, automatic double-buffering and
      (asynchronous) LRB uploads. <li>Data arrays can be shared among
      different scene graph entities, giving an additional layer of
      instantiation. Writing to a shared data array will trigger
      (intentional!) side effects<li> data arrays will track whether
      they get modified, and will propagate that information upwards
      to their parents</ul>

      \note The data array's own format is called the "internal
      format" as it is what the ray tracer will operate on. Writing
      data in a different format will automatically trigger a format
      conversion (ie, rtCoord3f writing to RT_FLOAT4 will
      automatically convert 3-floats to 4-floats. Not all conversions
      will be supported).

      \note In a certain sense, the data array is the central data
      container in ISG, and as many objects as possible should make
      use of it. Then, once we plug onto LRB, we'll have to implement
      the double-buffering etc _only_ for the data array, not for 20
      different types...

      \warning Note that the ray tracer's performance may (will!)
      depend on whether it "likes" the specified data format. Right
      now, however, it is too early to say which internal formats are
      better than others.
   */
  struct DataArray : public Node
  {
    typedef enum {
      APP_IS_OWNER,
      I_AM_OWNER,
      NO_DATA_TO_OWN
    } OwnerType;

    SemanticType type;
    DataFormat format;
    int units; // size in _items_, not in bytes 
    int size_in_bytes;

    void *m_ptr; // actual type depends on 'format' and 'type'

    OwnerType owner; /*! if true, we own the data, and will take it with us (to the grave...) when we die */
    
    DataArray(Node *parent, SemanticType type, DataFormat format) 
      : Node(parent), 
        type(type), 
        format(format), 
        units(0), 
        m_ptr(NULL), 
        owner(NO_DATA_TO_OWN) 
    {};
    ~DataArray() { clear(); };
    
    void clear() 
    { 
      if (owner == I_AM_OWNER 
          && 
          m_ptr != NULL) 
        free(m_ptr);
      m_ptr = NULL; 
      units = 0; 
      size_in_bytes = 0;
      owner = NO_DATA_TO_OWN;
    };

//     template<class T>
//     void allocate(T *t, int newUnits)
//     {
//       clear();
//       units = newUnits;
//       m_ptr = malloc(units * sizeof(T));
//       owner = I_AM_OWNER;
//     };
    
//     template<class T>
    void set(SemanticType type, DataFormat sourceFormat, void *ptr, int units, OwnerType owner)
    {
      if (I_AM_OWNER)
        {
          DataFormat &internalFormat = this->format;

          if (this->type != type) {
            if (this->type == RT_RAW_DATA)
              this->type = type;
            else
              FATAL("writing data of incompatible format to a data array");
          }
          
      

          if (internalFormat == RT_ANY_FORMAT)
            // if we didn't care so far, let's just use what's being
            // presented (not sure this can ever happen, though ...
            internalFormat = sourceFormat;

          long new_size_in_bytes = units * RTDataSizeOf[internalFormat];
          m_ptr = realloc(m_ptr,new_size_in_bytes);
          size_in_bytes = new_size_in_bytes;
          this->units = units;
          if (internalFormat == sourceFormat)
            memcpy(m_ptr,ptr,new_size_in_bytes);
          else
            switch (internalFormat) {
            case RT_FLOAT4:
              switch (sourceFormat) {
              case RT_FLOAT3:
                array_convert((vec4f*)m_ptr,(vec3f*)ptr,units);
                break;
              default:
                FATAL("data conversion from this source format to FLOAT4 not implemented, yet");
              }
              break;
            default:
              FATAL("data conversion to this internal format not implemented, yet");
            }
        }
      else 
        {
          /* APP_IS_OWNER, and tells us to use this format no matter
             what -- ignore all type checks, just use the data (note:
             might actually change semantics here, and do a
             copy-n-format-conversoin if types do not match */
          clear();
          // app owns it (and has promised it's of the correct type), so let's just use it
          m_ptr = ptr;
          size_in_bytes = 0; /*! dunno about what the app alloced, actually ... */
          this->type = type;
          this->format = format;
          this->units = units;
          this->owner = owner;
        }
    };
  };

//   struct IndexArray : public DataArray
//   {
//     IndexArray(Node *parent) : DataArray(parent) {};
//   };

//   struct CoordArray : public DataArray
//   {
//     CoordArray(Node *parent) : DataArray(parent) {};
//   };


  /*! for everything that's based on cages/meshes of (indexed) vertices:
    note: could eventually template by coord type, normal type, color
    type, etc. 
    
    Note that this is a placeholder for various _different_ kinds of
    meshes (triangle mesh, polygon mesh, quad mesh, subdiv cage, etc),
    and that it's up to the _backend_ to decide which actual type to
    generate. this is not particular "nice" from an implementation
    standpoint, but it gives lots of flexibility to programmer to
    change various things on the fly, and to use a consistent way of
    allocating a "mesh" (whatever the app thinks that means) without
    having to differentiate from calls like
    rtNewTriangleMeshSameShader(),
    rtNewTriangleMeshWithOneShaderPerTri(),
    rtNewQuadMeshWith.... etcpp. Might still change that, though, I
    don't have a strong preference, here... (might at least have to
    specify the type during creation like
    rtNewMesh(RT_TRIANGLEMESH,RT_SHADER_PER_TRIANGLE) or something
    like that -- feel free to change that if you want)
  */
  struct BaseMesh : public Node
  {
//     struct Triangle3 { 
//       int a;
//       int b;
//       int c;
//     };
//     struct Triangle4 {
//       int a;
//       int b;
//       int c;
//       int shaderID;
//     };
    
//     typedef enum {
//       RT_TRIANGLE3,
//       RT_TRIANGLE4,
//       RT_QUAD4,
//       RT_QUAD5,
//       RT_POLYGON
//     } PrimitiveType;
    
//     PrimitiveType type;

    //     CoordArray    *vtxCoord;
    //     NormalArray   *vtxNormal;
    //     ColorArray    *vtxColor;
    //     TexCoordArray *texCoord;
    
    /*! connectivity is shared even for different types. the base
        primitive type (triangle, quad, polygon, ...) is specified by
        the _type_ of the _mesh_ itself (i.e., whether it is a
        TriangleMesh, PolygonMesh, etc). The actual data layout of the
        connectivity array then defines what actual format the
        specific primitives have. e.g.:

        for a triangle mesh:
        
        - int3 : Mesh of Triangle3's, shader ID in separate array (or constnat for mesh)

        - int4 : each tri has vertex IDs and shader ID

        - none : "fat" triangles --> every three vertices are one triangle

        quad mesh: int4 and int4, as above

        polygonmesh: only int-array allowed (right now, that is), with each poly terminated by '-1'


        of course, all 'int's could also appear in 'byte' or 'short' form ....

        not all derived meshes will support all connectivity formats
        (e.g., int2 does not make sense for TriangleMesh, but also
        short3 might not be implemented (implementation specific)
    */
    DataArray  *index;  

    /*! vertex coordinates. also see comments on indices field about what those mean 
      data array for coordinates. note that we do not perform any
      type checking whether the actual derived mesh (trianglemesh vs
      quadmesh vs subdivmesh vs ...) supports that actual format
      (e.g., might not support halfs or uchars for coords, etc) */
    DataArray *coord;


    /*! most derived mesh implementations will likely interpre this as
        a per-primitive shader ID (though some primitives like
        Triannlge4i might use their own format for the shader
        ID). most "should" assume that if this list has length 1, the
        value contained in that list applies to _every_ primitive. */
    DataArray  *shaderList;

    void setCoords(DataArray *c)
    {
      removeParent(coord,this);
      coord = c;
      coord->addParent(this);
      // NO type checking here, as connectivity could still change afterwards...
    }
    void setIndices(DataArray *i) 
    {
      removeParent(index,this);
      index = i;
      index->addParent(this);
      // NO type checking here, as connectivity could still change afterwards...
    }
    
    MeshType type;

    BaseMesh(Node *parent, MeshType type ) 
      : Node(parent), type(type), index(NULL),coord(NULL),shaderList(NULL)
    {};
  };

//   struct GeometryShader : public Node
//   {
//   };

//   struct MeshInterpolator: public GeometryShader
//   {
//     Node *mesh_t0; /*!< mesh at time t=0 */
//     Node *mesh_t1; /*!< mesh at time t=1 */
//     float t; /*! time for interpolatoin. has to be 0<=t<=1 */
//   };


} // namespace





#endif
