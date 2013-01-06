#include "RTTL/common/RTInclude.hxx"
#include "LRT/include/lrt.h"
#include "LRT/FrameBuffer.hxx"
#include "LRT/FrameBuffer/PBOFrameBuffer.hxx"
#include "Camera.hxx"
#include "ObjParser.hxx"
#include "RTTL/common/Timer.hxx"
#include "RTTL/common/MapOptions.hxx"

#include <string>
#include <stdlib.h>
#include <stdio.h>

#ifdef ENABLE_PARSEC_HOOKS
#include <hooks.h>
#endif

using namespace RTTL;
using namespace LRT;

enum MouseButton { M_NONE, M_LEFT, M_MIDDLE, M_RIGHT };

#define INIT_RES 1024

int resX = INIT_RES;
int resY = INIT_RES;

int mouseButton = M_NONE;
int lastMouseX = -1, lastMouseY = -1;
float moveScale = 0.02f;
float rotateScale = 0.1f;

/* set at the command line */
RTVec3f viewerOrigin = RTVec3f(0.0,2.0,0.5);
RTVec3f viewerDirection = RTVec3f(-0.000000,-0.970143,-0.242536);
RTVec3f viewerUp = RTVec3f(0.000000,-0.242536,0.970143);
float viewerAngle = 64;
bool setViewer = false;
Camera camera;

bool glDisplay = true;
int framesToRender = 1;
bool autoMoveCamera = false;

/* new LRT stuff */
LRTFrameBufferHandle lrtFrameBuffer;
LRTContext lrtContext;
LRTCamera  lrtCamera;

void InitGL()
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glShadeModel(GL_FLAT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
}

void render()
{
  RTVec3f eye    = camera.getOrigin();
  RTVec3f center = camera.getOrigin()+camera.getDirection();
  RTVec3f up     = camera.getUp();

  lrtLookAt(lrtCamera,eye.x,eye.y,eye.z,center.x,center.y,center.z,up.x,up.y,up.z,camera.getAngle(),(float)resX/resY);	    
  lrtRenderFrame(lrtFrameBuffer,lrtContext,lrtCamera); // call ray tracer to trace its frame

  //spin object slowly (by moving camera around it)
  if(autoMoveCamera) {
    const float autoMoveScale = 0.01;

    //move camera perpendicular to the object
    RTVec3f oldOrigin = camera.getOrigin();
    camera.moveLocal(autoMoveScale*oldOrigin.length()*up);
    RTVec3f newOrigin = camera.getOrigin();
    const float driftRatio = newOrigin.length() / oldOrigin.length();

    //rotate camera to point again at object
    oldOrigin.normalize();
    newOrigin.normalize();
    float alpha = acosf(dot(oldOrigin,newOrigin)) * 180.0 / M_PI;
    camera.rotateGlobal(up, alpha);

    //move camera towards object to compensate for drift
    RTVec3f newDirection = camera.getDirection();
    newDirection.normalize();
    camera.setOrigin(camera.getOrigin() + (driftRatio-1.0) * newDirection);
  }
}

void display(void)
{
  render();

  if (glDisplay) lrtDisplayFB(lrtFrameBuffer); // display the FB via GL

  glutSwapBuffers();
  glutPostRedisplay();
}

/* called if window is resized */
void reshape(int w, int h)
{
  resX = w;
  resY = h;

  cout << "reshape: " << w << " x " << h << endl;
  DBG_PRINT("Ignoring resize");
  //if (frameBuffer) frameBuffer->resize(resX,resY);
}

/* called for mouse events */
void mouse(int button, int state, int x, int y)
{
  lastMouseX = -1;
  lastMouseY = -1;

  switch(button) {
  case GLUT_LEFT_BUTTON:
    if(state == GLUT_DOWN) {
      mouseButton = M_LEFT;
    }
    break;

  case GLUT_RIGHT_BUTTON:
    if(state == GLUT_DOWN) {
      mouseButton = M_RIGHT;
    }
    break;

  default:
    mouseButton = M_NONE;
  }

  glutPostRedisplay();
}

/* called if mouse is moved */
void motion(int x, int y)
{

  switch(mouseButton) {
  case M_RIGHT:

    if (lastMouseX == -1 || lastMouseY == -1) break;

    camera.moveLocal((lastMouseY - y) * moveScale * RTVec3f(1.0f,0.0f,0.0f));
    camera.moveLocal((x - lastMouseX) * moveScale * RTVec3f(0.0f,1.0f,0.0f));

    break;

  case M_LEFT:
    if (lastMouseX == -1 || lastMouseY == -1) break;

    camera.rotateGlobal(camera.getUp(),rotateScale * (lastMouseX - x));
    camera.rotateLocal(RTVec3f(0.0f,1.0f,0.0f),rotateScale * (y - lastMouseY));
    break;
  }

  lastMouseX = x;
  lastMouseY = y;

  glutPostRedisplay();

}

/* for keys */
void keyboardfunc(unsigned char key, int x, int y)
{
  int mod = glutGetModifiers();
  printf("key %i %c\n",key,key);

  switch (key)
    {
    case '+':
    case '*':
      moveScale *= 5.0f;
      break;

    case '-':
    case '/':
      moveScale /= 5.0f;
      break;

    case '<':
      break;

    case '>':
      break;


    case '!':
      break;

    case 'q':
    case 27:
      printf("exiting application\n");
      exit(0);
      break;
    }
}

/* for special keys */
void specialfunc(int key, int x, int y)
{
  //int mod = glutGetModifiers();
  switch(key) {
  case GLUT_KEY_F3:
    {
      RTVec3f o = camera.getOrigin();
      RTVec3f d = camera.getDirection();
      RTVec3f u = camera.getUp();
      cout << "Current camera:" << endl;
      cout << " -vp " << o.x << " " << o.y << " " << o.z;
      cout << " -vd " << d.x << " " << d.y << " " << d.z;
      cout << " -vu " << u.x << " " << u.y << " " << u.z;
      cout << endl;
      break;
    }
  case GLUT_KEY_F4:
    cout << "Resetting camera" << endl;
    camera.initMatrix(viewerOrigin,viewerDirection,viewerUp,viewerAngle);
    break;
  case GLUT_KEY_F5:
    break;
  default:
    printf("No function associated - sorry!\n");
    break;
  }
}

void parseRTVec3f(int argc, char* argv[], int& p, RTVec3f& vec)
{
  if (p+3 > argc) {
    exit(-1);
  }

  vec[0] = atof(argv[p+0]);
  vec[1] = atof(argv[p+1]);
  vec[2] = atof(argv[p+2]);
  p+=3;
}

void parseCommandLine(int argc, char* argv[])
{
  options.parse(argc-1, argv+1);
  cout << "Options: " << options << endl;

  setViewer = options.defined("vp, vd, vi, vu, vangle");

  if (options.defined("vp")) {
    viewerOrigin    = options.getVec3f("vp");
  }

  if (options.defined("vd")) {
    viewerDirection = options.getVec3f("vd");
  }
  if (options.defined("vi")) {
    viewerDirection = options.getVec3f("vi");
    viewerDirection -= viewerOrigin;
  }
  if (options.defined("vu")) {
    viewerUp = options.getVec3f("vu");
  }
  viewerAngle = options.get("vangle", viewerAngle);
  glDisplay = options.defined("display");
  framesToRender = options.get("frames", framesToRender);
  autoMoveCamera = options.defined("automove");
  if (options.defined("res")) {
    RTVec2i newRes = options.getVec2i("res");
    resX = newRes.x;
    resY = newRes.y;
  }
}

mesh_t generateUnitCube(node_t root)
{
  static float v[] = {
    -1.0,-1.0,-1.0,
    1.0,-1.0,-1.0,
    1.0,1.0,-1.0,
    -1.0,1.0,-1.0,
    -1.0,-1.0,1.0,
    1.0,-1.0,1.0,
    1.0,1.0,1.0,
    -1.0,1.0,1.0
  };

  static int t[] = {
    0,3,2,
    0,2,1,
    4,0,1,
    4,1,5,
    7,3,0,
    7,0,4,
    3,7,6,
    3,6,2,
    5,6,7,
    5,7,4,
    5,1,2,
    5,2,6
  };

  mesh_t mesh = rtTriangleMesh(root);
  
  /* allocate vertex array of 3-float vertices (hope the mesh's
     implementation likes three-float vertices !)... */
  data_t vertex_array = rtNewCoordArray(mesh,RT_FLOAT3);
  assert(rtValidData(vertex_array));
  
  /*! ... and write data to it. as vertex array data is same as the
    format we write, there's not type conversion, but a single
    memcpy */
  rtCoords3f(vertex_array,v,8,RT_PRIVATE);
  
  /*! same for the connectivity data */
  data_t index_array = rtNewIndexArray(mesh,RT_INT3);
  assert(rtValidData(index_array));
  rtIndices3i(index_array,t,12,RT_PRIVATE);
    
  return mesh;
}

void SetAutoCamera(RTBox3f& box) {
  RTVec3f m_min = box[0];
  RTVec3f m_max = box[1];
  RTVec3f midpoint = 0.5f * (m_min + m_max);
  float height = m_max[1] - m_min[1];
  float width  = m_max[0] - m_min[0];
  float max_size = (height < width) ? width : height;
  float dist = 0.5f * (max_size) / tanf(0.5f * viewerAngle * M_PI / 180.f);
  viewerOrigin    = RTVec3f(midpoint[0], midpoint[1], m_max[2] + dist);
  viewerUp        = RTVec3f(0,1,0);
  viewerDirection = RTVec3f(0, 0, -1);
}

int main(int argc, char* argv[])
{
#ifdef PARSEC_VERSION
#define __PARSEC_STRING(x) #x
#define __PARSEC_XSTRING(x) __PARSEC_STRING(x)
  cout << "PARSEC Benchmark Suite Version "__PARSEC_XSTRING(PARSEC_VERSION) << endl << flush;
#else
  cout << "PARSEC Benchmark Suite" << endl << flush;
#endif //PARSEC_VERSION
#ifdef ENABLE_PARSEC_HOOKS
  atexit(__parsec_bench_end);
  __parsec_bench_begin(__parsec_raytrace);
#endif

  rtInit(&argc, argv);
  lrtInit(&argc, argv); // idea is that rltInit can parse what it
                        // wants, strip the stuff it knows from the
                        // command line (before the app can actually
                        // see it), and return the rest to the app for
                        // processing. if we want to use the
                        // mapoptions class for that task, we'd have
                        // to extend it to do just that -- strip the
                        // parameters it is queried about, and return
                        // the rest -- then, we can use the mapoptions
                        // class in several different instances (once
                        // in the kernel, once in the app)

  resX = LRT::FrameBuffer::Options::defaultRes.x;
  resY = LRT::FrameBuffer::Options::defaultRes.y;

  parseCommandLine(argc, argv);

  if (glDisplay) {
    /* initialize glut */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(resX, resY);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("rtview");
    InitGL();
  } else
    LRT::FrameBuffer::Options::useMemoryFB = true;

  /* create LRT 'objects' */
  lrtContext     = lrtCreateContext();
  lrtCamera      = lrtCreateCamera();
  lrtFrameBuffer = lrtCreateTextureFB(resX,resY);

  /* initialize MiniRT */
  bool subdiv = options.defined("subdiv");
  if (subdiv) FATAL("Subdivision surface mode currently not available");

  int numThreads = options.get("nthreads", 1);
  lrtSetRenderThreads(lrtContext,numThreads);
  
  int nfiles = options.vector_size("files");

  node_t root = rtNewRoot(RT_VISIBLE);


  if (nfiles == 0) 
    generateUnitCube(root); /* if no obj file is specified use unit cube */
  else
    {
      ObjParser parser;

      for (int fi = 0; fi < nfiles; fi++) {
	const string& fn = (*options["files"])[fi];
	cout << "Adding obj file: " << fn << endl;
	parser.Parse(fn.c_str());
      }

      RTBox3f sceneAABB = parser.getSceneAABB();
      /* initialize camera */
      if (!setViewer)
	{
	  cout << "Using auto camera..." << endl;
	  SetAutoCamera(sceneAABB);
	}

      assert(parser.vertices());

      mesh_t mesh = rtTriangleMesh(root);

      data_t vertex_array = rtNewCoordArray(mesh,RT_FLOAT3);
      assert(rtValidData(vertex_array));
      rtCoords3f(vertex_array,(const float*)parser.getVertexPtr(),parser.vertices(),RT_PRIVATE);


      if (parser.triangles())
	{
	  /*! same for the connectivity data */
	  data_t index_array = rtNewIndexArray(mesh,RT_INT3);
	  assert(rtValidData(index_array));
	  rtIndices3i(index_array,(const int*)parser.getTrianglePtr(),parser.triangles(),RT_PRIVATE);
	}

      if (parser.quads())
	{
	  /*! same for the connectivity data */
	  data_t index_array = rtNewIndexArray(mesh,RT_INT4);
	  assert(rtValidData(index_array));
	  rtIndices4i(index_array,(const int*)parser.getQuadPtr(),parser.quads(),RT_PRIVATE);
	}

      /* -- transfer vertices -- */
      //miniRT.addVertices(parser.getVertexPtr(),parser.getTextureCoordinatePtr(),parser.vertices());
      /* -- transfer materials -- */
      //miniRT.addMaterials(parser.getMaterialPtr(),parser.materials());
      /* -- transfer textures  -- */
      for (int i=0;i<parser.textures();i++)
	{
	  ImagePPM *txt = parser.getTexture(i);
	  //miniRT.addTexture(txt->width(),txt->height(),txt->data(),RT_TEXTURE_FORMAT_RGB_UCHAR);
	}     
      /* -- transfer triangles -- */
      //if (parser.triangles()) miniRT.addTriangleMesh(parser.getTrianglePtr(),parser.triangles(),parser.getTriangleShaderPtr());
      /* -- transfer quads -- */
      //if (parser.quads()) miniRT.addQuadMesh(parser.getQuadPtr(),parser.quads(),parser.getQuadShaderPtr());
      parser.Free();
    }
  if (options.defined("exitafterbuild")) exit(0);


  camera.initMatrix(viewerOrigin,viewerDirection,viewerUp,viewerAngle);
  lrtBuildContext(lrtContext);

  if (glDisplay) {
    cout << "Starting glut..." << endl; 

    /* open glut window */
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutSpecialFunc(specialfunc);
    glutKeyboardFunc(keyboardfunc);

    /* start rendering */
#ifdef ENABLE_PARSEC_HOOKS
    atexit(__parsec_roi_end);
    __parsec_roi_begin();
#endif
    glutMainLoop();
  } else {
    cout << "Rendering " << framesToRender << " frames... " << endl << flush;
#ifdef ENABLE_PARSEC_HOOKS
    __parsec_roi_begin();
#endif
    int frame = 0;
    do {
      render();
      frame++;
    } while(!(__builtin_expect(framesToRender > 0,0) && frame >= framesToRender));
#ifdef ENABLE_PARSEC_HOOKS
    __parsec_roi_end();
#endif
    cout << "Done" << endl << flush;
  }

  return 0;
}

