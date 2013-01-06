#ifndef RTTL_RT_H
#define RTTL_RT_H

/*! \file rt.h RTTL core api. @ingroup rt_api */



#include "ISG.hxx" // obviously, _that_ should eventually _not_ be visible to the app...



// namespace LRT {
using namespace RTTL;
using namespace ISG;



/**  @ingroup rt_api */
/*@{*/


/*! note: this is subject to change: eventually, node_t will be an int
  _referencing_ a node, not a pointer to the node itself (thus, we
  can use the same ID on both host and device */
typedef ISG::Node *node_t;

/*! a data array. see \ref notes_on_data_arrays */
typedef ISG::DataArray *data_t;

typedef ISG::BaseMesh *mesh_t;

typedef unsigned int uint_t;


// ======================================================= 
/* PARAMETER TYPES: strongly typed, to make coding easier -- but may
   eventually go back to #define'ing them and storing them as RTuint's
   like GL does... */

/*! 'default' is always 0 -- make sure that this is the case in all of
  the flags defined below ... */
#define RT_DEFAULT 0 

/*! the various parameter options for the 'flags' parameter in \ref rtNewRoot */
typedef enum { 
  RT_HIDDEN=RT_DEFAULT, 
  RT_VISIBLE 
} newroot_flags_t;
/*@}*/




/**  @ingroup rt_api */
/*@{*/

/*! probably something like rtInstantiate(root_node) in openrt lingo... -- at least for now */
void  rtShow(node_t node);

extern_inline void  rtHide(node_t node) { NOTIMPLEMENTED; };


/*! create a new root node (i.e., one that doesn't need a parent -- see \ref node_ownership 
  
  as long as we dont konw how (if?) we handle multiple
  instantiations (e.g., do we allow instances of instances ??),
  thi'll probably just be the same as an "object" in openrt lingo.
  
  \param flags flags for object creation. Currently, can be RT_VISIBLE or RT_HIDDEN (RT_DEFAULT==RT_HIDDEN)
*/
extern_inline node_t  rtNewRoot(newroot_flags_t flags)
{
  node_t root = new RootNode;
  if (flags & RT_VISIBLE)
    rtShow(root);
  return root;
}

/*! create a new mesh node */
extern_inline mesh_t  rtNewMesh(node_t parent, MeshType type)
{
  assert(parent);
  mesh_t mesh = new BaseMesh(parent,type); // that's all we
  GroupNode *group = dynamic_cast<GroupNode*>(parent);
  if (!group)
    FATAL("can only add meshes to group nodes ... at least right now ... ");
  group->addChild(mesh);
  // specify -- leave it
  // to the core to
  // decide what actual
  // implementatoin to
  // use, depending on
  // how the mesh's addtl
  // data is specified
  return mesh;
}

/*@}*/



extern_inline mesh_t  rtTriangleMesh(node_t parent)
{
  return rtNewMesh(parent,RT_TRIANGLE_MESH);
}

extern_inline mesh_t  rtQuadMesh(node_t parent)
{
  return rtNewMesh(parent,RT_QUAD_MESH);
}


/*! the various parameter options for the 'flags' parameter in,
  e.g., \ref rtVertexArray3f, \ref TriangleArray3i, etc
      
  different flags can be 'or'ed together. every bit should have a
  symbolic name for both its 'true' and its 'false' case (e.g.,
  RT_PRIVATE and RT_PERSISTENT, RT_FORCE_OVERWRITE and
  RT_NEW_ARRAY, etc)

  \note might need better names ....
*/
typedef enum { 
  RT_PRIVATE=RT_DEFAULT, 
  /*! if specified, data is _shard_ with the app, i.e., we actually
    point into the app's address space instead of copying the data
    ... */
  RT_PERSISTENT=1,
  RT_PERSISTENT_BIT=(RT_PERSISTENT),

  /*! if specified, _force_ overwriting the specific data array
    even though it may be shared. For instance, if two meshes m1 and
    m2 share a given vertex array, then rtVertexArray3f(m2, ...,
    RT_DEFAULT) would have m1 and m2 end up with two different
    vertex arrays (m1 has the old one, m2 the new one), whereas
    rtVertexArray(m2, ..., RT_FORCE_OVERWRITE would overwrite the
    _shared_ vertex array, thus having a implicit side-effect on m1,
    too.

    \note: may also be beneficial even if data array is not shared:
    e.g., assuming mesh m already has a vertex array of the proper
    size, then specifying RT_FORCE_OVERWRITE will actually force a
    direct memcopy, whereas not specifying it will first create a
    new array

    \note opposite of RT_NEW_ARRAY
  */
  RT_FORCE_OVERWRITE=RT_DEFAULT,
    
  /*! the opposite of RT_FORCE_OVERWRITE */
  RT_NEW_ARRAY=2,
  RT_NEW_ARRAY_BIT=(RT_NEW_ARRAY|RT_FORCE_OVERWRITE),
  RT_FORCE_OVERWRITE_BIT=(RT_NEW_ARRAY|RT_FORCE_OVERWRITE)
} DataArrayFlags;

/*! mapping modes for rtMap... calls */
typedef enum {
  RT_READ  = 0x1,
  RT_WRITE = 0x2,
  RT_RW=RT_READ|RT_WRITE
} MapMode;

// -------------------------------------------------------
/*! initialize the ray tracer -- eventually parse the options etc,
  but do nothing as yet */
extern_inline void  rtInit(int *acptr, char **avptr)
{ 
  cout << "initializing LRT ray tracer ..." << endl; 
};


// -------------------------------------------------------



// -------------------------------------------------------
/*! destroy a node, and all its dependents that do not have other owners -- see \ref node_ownership */
extern_inline void    rtDestroy(node_t node)
{
  /* not yet implemented ... */
}

  
extern_inline data_t  rtNewDataArray(node_t parent,
                                       SemanticType semanticType=RT_RAW_DATA,
                                       DataFormat dataFormat=RT_BYTE
                                       )
{
  DataArray *arr = new DataArray(parent,
                                 semanticType,
                                 dataFormat
                                 );
  return arr;
};

/* no type checking/conversion right now ... */
extern_inline void  rtWriteToArray(data_t target, 
				   SemanticType semanticType,
				   DataFormat dataFormat,
				   unsigned char *const ptr,
				   int count,
				   DataArrayFlags flags)
{
  assert(target);
  DataArray *array = dynamic_cast<DataArray *>(target);
  assert(array);

  /* actually, would have to do some type checking here, and either
     return an error (eg, when app tries to pass persistent array of
     wrong format, or convert on the fly */
    
  if ((flags & RT_PERSISTENT_BIT) == RT_PERSISTENT) 
    {
      // have array hard-point into app's memory space, to pointer specified by app
      array->set(semanticType,dataFormat,ptr,count,DataArray::APP_IS_OWNER);        
    }
  else
    {
      array->set(semanticType,dataFormat,ptr,count,DataArray::I_AM_OWNER);        
    }
}

extern_inline void  rtCoords3f(data_t target, const float *const coord, int coords, DataArrayFlags flags)
{
  rtWriteToArray(target,RT_COORDS,RT_FLOAT3,(unsigned char *)coord,coords,flags);
  //     rtWriteToArray(target,RT_COORD,RT_FLOAT3,(unsigned char *)ptr,cnt,flags);
}
extern_inline void  rtIndices3i(data_t target, const int *const coord, int coords, DataArrayFlags flags)
{
  rtWriteToArray(target,RT_INDICES,RT_INT3,(unsigned char *)coord,coords,flags);
}

extern_inline void  rtIndices4i(data_t target, const int *const coord, int coords, DataArrayFlags flags)
{
  FATAL("if mesh is triangular mesh then quad tesellation is required");
  rtWriteToArray(target,RT_INDICES,RT_INT4,(unsigned char *)coord,coords,flags);
}


/*
  void rtMeshCoords(mesh_t meshRef, data_t coord)
  {
  BaseMesh *mesh = dynamic_cast<BaseMesh *>(meshRef);
  assert(mesh);

  assert(coord);

  mesh->setCoords(coord);
  };

  void rtMeshIndices(mesh_t meshRef, data_t index)
  {
  BaseMesh *mesh = dynamic_cast<BaseMesh *>(meshRef);
  assert(mesh);
    
  assert(index);
    
  mesh->setIndices(index);
  };
*/

/*! returns a _reference_ to the mesh's coord arra. note that you
  still have to call rtMapBuffer on that to get the actual pointer
  (at least, if you want to stay within the API's memory model
  (the data array might live on the DEVICE!), so returning a
  reference to it is different from returning a pointer to an
  actual buffer that contains the data (which might require an
  actual device-to-host memcopy!)

  \param node should reference a valid node that contains a vertex
  array (such as a mesh). if that is not the case (i.e., node
  references a transform node), it will return an invalid
  reference (NULL, or whatever reference corresponds to it)

*/
extern_inline data_t  rtGetMeshCoords(mesh_t m)
{
  BaseMesh *mesh = dynamic_cast<BaseMesh *>(m);
  //     PRINT(mesh);
  assert(mesh);
  //     PRINT(mesh->coord);
  return mesh->coord;
};
extern_inline data_t  rtGetMeshIndices(mesh_t m)
{
  BaseMesh *mesh = dynamic_cast<BaseMesh *>(m);
  assert(mesh);
  return mesh->index;
};

extern_inline data_t  rtNewCoordArray(node_t parent, DataFormat internalFormat)
{
  data_t data = new DataArray(parent, RT_COORDS, internalFormat);
  mesh_t mesh = dynamic_cast<BaseMesh*>(parent);
  if (mesh)
    mesh->setCoords(data);
  return data;
}
extern_inline data_t  rtNewIndexArray(node_t parent, DataFormat internalFormat)
{
  data_t data = new DataArray(parent, RT_INDICES, internalFormat);
  mesh_t mesh = dynamic_cast<BaseMesh*>(parent);
  if (mesh)
    mesh->setIndices(data);
  return data;
}


/*! returns whether some data is valid (i.e., non-null). note that
  since data_t might be a _reference_ to a pointer on another
  device, we can't (shouldn't!) directly compare to 'NULL'.... */
extern_inline bool  rtValidData(data_t data)
{ 
  return data != NULL; /* as long as all data is on the host, we can
                          do that -- but do _NOT_ do that in the
                          app! */
}

/*! map a data reference. since we're not yet double-buffering,
  anyway, right now we'll just return a pointer to the data. will
  eventually have to create copies if mapping to buffers that are
  still being used internally) */
extern_inline void * rtMapBuffer(data_t data, MapMode mapMode)
{
  if (!data) return NULL;
  return data->m_ptr;
}
/*! the couterpart to rtMapBuffer. should be _exactly_ the same
  pointer as returned by rtMapBuffer. do not fail to unmap any
  mapped item, do not unmap more than once. */
extern_inline void  rtUnmapBuffer(data_t data) 
{};

// #ifdef __cplusplus
//   /*! c++ convenience fct */
//   vec3f *rtMapCoords_vec3f(mesh_t node, MapMode mapMode)
//   {
//     data_t data = rtGetMeshCoords(node);
//     assert(rtValidData(data));
//     return (vec3f*)rtMapBuffer(data,mapMode);
//   }
//   /*! c++ convenience fct */
//   template <class T>
//   void rtUnmapBuffer(T *data)
//   { rtUnmapBuffer((void*)data); };
// #endif


extern_inline void  rtEndGeometry() { /* do nothing right now -- supposed to flag
                                           that we can now start building whatever
                                           data structures we need ... */ };
// }; // end namespace


#endif
