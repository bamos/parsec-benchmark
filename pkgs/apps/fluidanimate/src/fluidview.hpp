// Written by Christian Bienia and Richard O. Lee
// This file contains code to visualize the fluid interactively
// Requires the GLUT library

#ifndef __FLUIDVIEWER_HPP__
#define __FLUIDVIEWER_HPP__ 1

#include <GL/glut.h>
#include "fluid.hpp"

//Initialize visualization mode
//argc and argv are the command line argument passed to the program
//AdvanceFrame is a pointer to the function which advances the the fluid by one frame
//numCells, cells and cnumPars are pointers to the fluid state variables
void InitVisualizationMode(int *argc, char *argv[], void (*AdvanceFrame)(), int *numCells, Cell **cells, int **cnumPars);

//Start computing the frames with AdvanceFrame function pionter and
//visualize the fluid after each step
void Visualize();

#endif //__FLUIDVIEWER_HPP__
