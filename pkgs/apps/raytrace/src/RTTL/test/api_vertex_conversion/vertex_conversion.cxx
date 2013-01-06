#include "RTTL/API/rt.h"

/*! \file newmesh.cxx a simple test program to create a mesh and
    supply it with data 
    
    this example is almost the same as newmesh.c, except that we store
    data internally in a different format, thus triggering an implicit
    type conversion upon uploading the vertices

*/

float cube_vertex [8][3] = { 
  { -1, -1, -1 },
  { -1, -1, +1 },
  { -1, +1, -1 },
  { -1, +1, +1 },
  { +1, -1, -1 },
  { +1, -1, +1 },
  { +1, +1, -1 },
  { +1, +1, +1 } };

int cube_tris[12][3] = {
  // -X : 0,1,2,3; +x : 4,5,6,7
  { 0, 1, 3 },
  { 0, 2, 3 },
  { 4, 5, 7 },
  { 4, 6, 7 },
  
  // -Y : 0,1,4,5: +y : 2,3,6,7
  { 0, 1, 5 },
  { 0, 4, 5 },
  { 2, 3, 7 },
  { 2, 6, 7 },
  
  // -Z : 0,2,4,6: +z : 1,3,5,7
  { 0, 2, 6 },
  { 0, 4, 6 },
  { 1, 3, 7 },
  { 1, 5, 7 }
};

int main(int ac, char **av)
{
  try {
    rtInit(&ac,av);
    
    /* allocate a new root object to contains "scene graph" */
    node_t root = rtNewRoot(RT_HIDDEN);
    
    /* allocate a new triangle mesh as single node in the scene graph*/
    mesh_t mesh = rtTriangleMesh(root);

    /* allocate vertex array of 4-float vertices (maybe the mesh likes
       SSE 4-float format better than float3's ?)...  since we'll soon
       upload them in float3 format, we'll trigger a type conversion*/
    data_t vertex_array = rtNewCoordArray(mesh,RT_FLOAT4);
    assert(rtValidData(vertex_array));

    /*! ... and write data to it. as vertex array data is same as the
        format we write, there's not type conversion, but a single
        memcpy */
    rtCoords3f(vertex_array,&cube_vertex[0][0],8,RT_PRIVATE);

    /*! same for the connectivity data */
    data_t index_array = rtNewIndexArray(mesh,RT_INT3);
    assert(rtValidData(index_array));
    rtIndices3i(index_array,&cube_tris[0][0],12,RT_PRIVATE);

    /*! don't do anything with it, just test if we can create it ... */
  

    // test if it arrived in the correct form:
    cout << "this should print the 8 vertices of a cube in 4-float format (last float being unused) ... (if not, something is broken):" << endl;
    vec4f *mapped_vertex = (vec4f*)rtMapBuffer(rtGetMeshCoords(mesh),RT_READ);
    for (int i=0;i<8;i++)
      cout << "vtx " << i << " : " << mapped_vertex[i] << endl;
    rtUnmapBuffer((data_t)mapped_vertex);

    rtDestroy(root); // clean up, will also delete 
  } catch (...) {
    cerr << "ERROR: some kind of error has occurred in testing code '" <<__FILE__ << endl;
    return 1;
  }
  return 0;
};

