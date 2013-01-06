// ================================================================================
// RT_MESH_DESCRIPTOR is persistent (will exist in the includer)
// ================================================================================
#undef  RT_MESH_DESCRIPTOR
#define RT_MESH_DESCRIPTOR (\
    ((long long)RT_VERTEX_POSITION << 0*3) | \
    ((long long)RT_VERTEX_NORMAL   << 1*3) | \
    ((long long)RT_VERTEX_TEXTURE1 << 2*3) | \
    ((long long)RT_VERTEX_TEXTURE2 << 3*3) | \
    ((long long)RT_VERTEX_TEXTURE3 << 4*3) | \
    ((long long)RT_VERTEX_TEXTURE4 << 5*3) | \
    ((long long)RT_FACE_NORMAL     << 6*3) | \
    ((long long)RT_MATERIAL        << 7*3) | \
    0L)

// ================================================================================
// Vertex is an array of floats of size VERTEX_SIZE.
// VERTEX_SIZE is persistent (will exist in the includer)
// ================================================================================
#undef  VERTEX_SIZE
#define VERTEX_SIZE ((0                                                \
        + ((RT_VERTEX_POSITION == RT_INDEXED   )? sizeof(RTVec3f) : 0) \
        + ((RT_VERTEX_NORMAL   == RT_VERTEXDATA)? sizeof(RTVec3f) : 0) \
        + ((RT_VERTEX_TEXTURE1 == RT_VERTEXDATA)? sizeof(RTVec2f) : 0) \
        + ((RT_VERTEX_TEXTURE2 == RT_VERTEXDATA)? sizeof(RTVec2f) : 0) \
        + ((RT_VERTEX_TEXTURE3 == RT_VERTEXDATA)? sizeof(RTVec2f) : 0) \
        + ((RT_VERTEX_TEXTURE4 == RT_VERTEXDATA)? sizeof(RTVec2f) : 0) \
        + (RT_VERTEX_DATA*sizeof(float)) \
    )/sizeof(float))

