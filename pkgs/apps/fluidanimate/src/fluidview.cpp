// Written by Christian Bienia and Richard O. Lee
// This file contains code to visualize the fluid interactively
// Requires the GLUT library

#include <cstdlib>
#include <iostream>
#include <GL/glut.h>

#include "fluid.hpp"
#include "fluidview.hpp"



float rotX = 0.f;
float rotY = 0.f;
float moveX = 0.f;
float moveY = 0.03f;
float depth = -0.2f;
bool rot = false;
bool move = false;
bool zoom = false;
int oldX, oldY;

//function pointer to function that advances fluid by one frame
void (*_AdvanceFrame)();

//pointers to fluid state variables
int *_numCells;
Cell **_cells;
int **_cnumPars;
extern Vec3 vMin;
extern Vec3 vMax;


void InitVisualizationMode(int *argc, char *argv[], void (*AdvanceFrame)(), int *numCells, Cell **cells, int **cnumPars)
{
  glutInit(argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(640, 480);
  glutInitWindowPosition(50, 50);
  glutCreateWindow("PARSEC Fluidanimate - Visualization Mode");

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_FLAT);
  glClearColor(0.f, 0.f, 0.f, 0.f);

  const float la[] = { 0.1f, 0.1f, 0.1f, 0.f };
  const float ld[] = { 0.5f, 0.5f, 0.5f, 0.f };
  glLightfv(GL_LIGHT0, GL_AMBIENT, la);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, ld);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);

  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

  _AdvanceFrame = AdvanceFrame;
  _numCells = numCells;
  _cells = cells;
  _cnumPars = cnumPars;
}

////////////////////////////////////////////////////////////////////////////////

void Reshape(int width, int height)
{
  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45., GLdouble(width)/GLdouble(height), 0.1, 100.);

  glMatrixMode(GL_MODELVIEW);
}

////////////////////////////////////////////////////////////////////////////////

void Idle()
{
  glutPostRedisplay();
}

////////////////////////////////////////////////////////////////////////////////
fptype prior_y_Max = 0;
fptype y_Tot = 0;
fptype prior_y_Tot = 0;
fptype prior_y_Ave = 0;
static int DisplayFrameNumber = 0;
void Display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  std::cout << "Advancing frame (" << DisplayFrameNumber++ << ")..." << std::endl << std::flush;
  _AdvanceFrame();

  glLoadIdentity();
  glTranslatef(moveX, moveY, depth);
  glRotatef(rotX, 1.f, 0.f, 0.f);
  glRotatef(rotY, 0.f, 1.f, 0.f);

  GLfloat lp[] = { 1.f, 2.f, 3.f, 0.f };
  glLightfv(GL_LIGHT0, GL_POSITION, lp);

  glColor3f(1,1,1);
  glLineWidth(3.f);
  glBegin(GL_LINES);
  const float yardstick = domainMin.y + (domainMax.y-domainMin.y)*0.11f;
  glVertex3f(domainMin.x, domainMin.y, domainMin.z);
  glVertex3f(domainMin.x, yardstick, domainMin.z);
  glVertex3f(domainMax.x, domainMin.y, domainMin.z);
  glVertex3f(domainMax.x, yardstick, domainMin.z);
  glVertex3f(domainMin.x, domainMin.y, domainMax.z);
  glVertex3f(domainMin.x, yardstick, domainMax.z);
  glVertex3f(domainMax.x, domainMin.y, domainMax.z);
  glVertex3f(domainMax.x, yardstick, domainMax.z);
  glEnd();

  glColor3f(0.f, 0.5f, 0.8f);
  glPointSize(2.f);
  glBegin(GL_POINTS);
  vMax.y = domainMin.y;
  y_Tot = 0;
  fptype surface = domainMin.y + prior_y_Ave*2.0;
  for(int i = 0; i < *_numCells; ++i)
  {
    Cell *cell = &(*_cells)[i];
    int np = (*_cnumPars)[i];
    for(int j = 0; j < np; ++j)
    {
#define USE_track_velocity
#if defined(USE_track_velocity)
		// track velocity
		Vec3	v = cell->v[j % PARTICLES_PER_CELL];
		vMin.x = std::min(vMin.x,v.x);
		vMax.x = std::max(vMax.x,v.x);
		vMin.y = std::min(vMin.y,v.y);
		vMax.y = std::max(vMax.y,v.y);
		vMin.z = std::min(vMin.z,v.z);
		vMax.z = std::max(vMax.z,v.z);
		glColor3f(
			(v.x - vMin.x) / (vMax.x - vMin.x),
			(v.y - vMin.y) / (vMax.y - vMin.y),
			(v.z - vMin.z) / (vMax.z - vMin.z));
#else
		// track y displacement

		fptype	y = cell->p[j % PARTICLES_PER_CELL].y;
		y_Tot += y - domainMin.y;
		vMax.y = std::max(vMax.y,y);
		prior_y_Max = std::max(vMax.y,prior_y_Max);
		glColor3f(
			0.5f,
			0.5f + (float)((y - surface) / (prior_y_Max - surface)),
			0.5f);
#endif
      glVertex3f(cell->p[j % PARTICLES_PER_CELL].x, cell->p[j % PARTICLES_PER_CELL].y, cell->p[j % PARTICLES_PER_CELL].z);
      //move pointer to next cell in list if end of array is reached
      if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1) {
        cell = cell->next;
      }

    }
  }
#if defined(USE_track_velocity)
  vMin *= 0.99;
  // avoid vMin == vMax (causes divide by 0)
  vMax.x = std::max(vMax.x * 0.99, vMin.x + epsilon);
  vMax.y = std::max(vMax.y * 0.99, vMin.y + epsilon);
  vMax.z = std::max(vMax.z * 0.99, vMin.z + epsilon);
#else
  prior_y_Max = vMax.y;
  prior_y_Ave = y_Tot / (fptype)*_numCells;
#endif
  glEnd();

  glFlush();
  glutSwapBuffers();
}

////////////////////////////////////////////////////////////////////////////////

void ApplyImpulse(Vec3 const &impulse)
{
  for(int i = 0; i < *_numCells; ++i)
  {
    Cell *cell = &(*_cells)[i];
    int np = (*_cnumPars)[i];
    for(int j = 0; j < np; ++j)
    {
      cell->hv[j % PARTICLES_PER_CELL] += impulse;
      //move pointer to next cell in list if end of array is reached
      if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1) {
        cell = cell->next;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void Keyboard(unsigned char key, int x, int y)
{
  switch(key)
  {
    case 'a': ApplyImpulse(Vec3(-1.f, 0.f, 0.f)); break;
    case 'd': ApplyImpulse(Vec3(1.f, 0.f, 0.f)); break;
    case 'e': ApplyImpulse(Vec3(0.f, -1.f, 0.f)); break;
    case 'q': ApplyImpulse(Vec3(0.f, 1.f, 0.f)); break;
    case 'w': ApplyImpulse(Vec3(0.f, 0.f, -1.f)); break;
    case 's': ApplyImpulse(Vec3(0.f, 0.f, 1.f)); break;
    case 27:    // escape
    exit(0);
    break;
  }

  glutPostRedisplay();
}

////////////////////////////////////////////////////////////////////////////////

void Mouse(int button, int state, int x, int y)
{
  if(button == GLUT_LEFT_BUTTON)
    rot = (state == GLUT_DOWN);
  else if(button == GLUT_MIDDLE_BUTTON)
    move = (state == GLUT_DOWN);
  else if(button == GLUT_RIGHT_BUTTON)
  zoom = (state == GLUT_DOWN);

  oldX = x;
  oldY = y;
}

////////////////////////////////////////////////////////////////////////////////

void Motion(int x, int y)
{
  if(rot)
  {
    rotX += float(y - oldY) * 0.5f;
    rotY += float(x - oldX) * 0.5f;
  }

  if(move)
  {
    moveX += float(x - oldX) * 0.01f;
    moveY -= float(y - oldY) * 0.01f;
  }

  if(zoom)
    depth += float(y - oldY) * 0.05f;

  oldX = x;
  oldY = y;
}

void Visualize() {
  glutReshapeFunc(Reshape);
  glutIdleFunc(Idle);
  glutDisplayFunc(Display);
  glutKeyboardFunc(Keyboard);
  glutMouseFunc(Mouse);
  glutMotionFunc(Motion);

  glutMainLoop();
}
