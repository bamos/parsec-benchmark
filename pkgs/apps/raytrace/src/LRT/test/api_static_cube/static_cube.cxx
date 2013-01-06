#include "LRT/include/lrt.h"
#include <GL/gl.h>
#ifdef THIS_IS_APPLE
#include <glut.h>
#else
#include <GL/glut.h>
#endif

#include <iostream>
using namespace std;



#define WIDTH 512
#define HEIGHT 512

LRTFrameBufferHandle frameBuffer;
LRTCamera camera;
LRTContext context;

/*! \file newmesh.cxx a simple test program to create a mesh and
    supply it with data */

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

void idle()
{
  glutPostRedisplay();
}

void display()
{
  lrtRenderFrame(frameBuffer,context,camera); // call ray tracer to trace its frame
  lrtDisplayFB(frameBuffer); // display the FB via GL
  glutSwapBuffers(); // and swap GL(UT) buffer
  glutPostRedisplay();
}

mesh_t createCube(node_t root)
{
  /* allocate a new triangle mesh as single node in the scene graph*/
  mesh_t mesh = rtTriangleMesh(root);
  
  /* allocate vertex array of 3-float vertices (hope the mesh's
     implementation likes three-float vertices !)... */
  data_t vertex_array = rtNewCoordArray(mesh,RT_FLOAT3);
  assert(rtValidData(vertex_array));
  
  /*! ... and write data to it. as vertex array data is same as the
    format we write, there's not type conversion, but a single
    memcpy */
  rtCoords3f(vertex_array,&cube_vertex[0][0],8,RT_PRIVATE);
  
  /*! same for the connectivity data */
  data_t index_array = rtNewIndexArray(mesh,RT_INT3);
  assert(rtValidData(index_array));
  rtIndices3i(index_array,&cube_tris[0][0],12,RT_PRIVATE);
    
  return mesh;
};

int main(int ac, char **av)
{
  try {
    glutInit(&ac,av);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(0, 0);
//     glutIdleFunc(idle);
    glutCreateWindow("static cube in a glut window");

    glutDisplayFunc(display);
//     glutIdleFunc(idle);

    /* init ray tracer and allocate a new root object to contains "scene graph" */
    rtInit(&ac,av);
    frameBuffer = lrtCreateTextureFB(WIDTH,HEIGHT);
    camera = lrtCreateCamera();
    context = lrtCreateContext();

    node_t root = rtNewRoot(RT_VISIBLE);
    mesh_t cube = createCube(root);
    /* end geometry definition -- let ray tracer do the rest whenever display() is called */
    rtEndGeometry();

    lrtLookAt(camera,
              -4,-3,-2, // eye
              0,0,0,    // center
              0,1,0,     // up
	      64,
	      1
              );
    // last initialization step 
    lrtBuildContext(context);

    // enter the glut main loop, will automatically trigger rendering in display()
    cout << "entering glut main loop " << endl;
    glutMainLoop();
  } catch (...) {
    cerr << "ERROR: some kind of error has occurred in testing code '" <<__FILE__ << endl;
    return 1;
  }
  return 0;
};

