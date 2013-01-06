#include "../RTTL/common/RTInclude.hxx"
#include <stdio.h>
#include <GL/gl.h>
#ifdef THIS_IS_APPLE
#include <glut.h>
#else
#include <GL/glut.h>
#endif

#include <iostream>
using namespace std;

void display(void)
{
  printf("display -- press ctrl-c to cancel\n");
}

void idle(void)
{
  glutPostRedisplay();
}


int main(int ac, char **av)
{
  glutInit(&ac,av);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  glutInitWindowSize(512, 512);
  glutInitWindowPosition(0, 0);
  glutCreateWindow("rtview");

  cout << "hello (glut-)world -- press ctrl-c to cancel" << endl;
  glutDisplayFunc(display);
  glutIdleFunc(idle);

  glutMainLoop();

  exit(0);
}

