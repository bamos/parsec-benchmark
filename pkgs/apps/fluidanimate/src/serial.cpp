//Code written by Richard O. Lee and Christian Bienia
//Modified by Christian Fensch

#include <cstdlib>
#include <cstring>

#include <iostream>
#include <fstream>
#include <math.h>
#include <assert.h>

#include "fluid.hpp"
#include "cellpool.hpp"

#ifdef ENABLE_VISUALIZATION
#include "fluidview.hpp"
#endif

#ifdef ENABLE_PARSEC_HOOKS
#include <hooks.h>
#endif

//Uncomment to add code to check that Courant–Friedrichs–Lewy condition is satisfied at runtime
//#define ENABLE_CFL_CHECK

//Define ENABLE_STATISTICS to collect additional information about the particles at runtime.
//#define ENABLE_STATISTICS

////////////////////////////////////////////////////////////////////////////////

cellpool pool;

fptype restParticlesPerMeter, h, hSq;
fptype densityCoeff, pressureCoeff, viscosityCoeff;

int nx, ny, nz;     // number of grid cells in each dimension
Vec3 delta;         // cell dimensions
int numParticles = 0;
int numCells = 0;
Cell *cells = NULL;
Cell *cells2 = NULL;
int *cnumPars = 0;
int *cnumPars2 = 0;
Cell **last_cells = NULL; //helper array with pointers to last cell structure of "cells" array lists
#ifdef ENABLE_VISUALIZATION
Vec3 vMax(0.0,0.0,0.0);
Vec3 vMin(0.0,0.0,0.0);
#endif
////////////////////////////////////////////////////////////////////////////////

void InitSim(char const *fileName)
{
  std::cout << "Loading file \"" << fileName << "\"..." << std::endl;
  std::ifstream file(fileName, std::ios::binary);
  if(!file) {
    std::cerr << "Error opening file. Aborting." << std::endl;
    exit(1);
  }

  //Always use single precision float variables b/c file format uses single precision
  float restParticlesPerMeter_le;
  int numParticles_le;
  file.read((char *)&restParticlesPerMeter_le, FILE_SIZE_FLOAT);
  file.read((char *)&numParticles_le, FILE_SIZE_INT);
  if(!isLittleEndian()) {
    restParticlesPerMeter = bswap_float(restParticlesPerMeter_le);
    numParticles          = bswap_int32(numParticles_le);
  } else {
    restParticlesPerMeter = restParticlesPerMeter_le;
    numParticles          = numParticles_le;
  }
  cellpool_init(&pool, numParticles);

  h = kernelRadiusMultiplier / restParticlesPerMeter;
  hSq = h*h;

#ifndef ENABLE_DOUBLE_PRECISION
  fptype coeff1 = 315.0 / (64.0*pi*powf(h,9.0));
  fptype coeff2 = 15.0 / (pi*powf(h,6.0));
  fptype coeff3 = 45.0 / (pi*powf(h,6.0));
#else
  fptype coeff1 = 315.0 / (64.0*pi*pow(h,9.0));
  fptype coeff2 = 15.0 / (pi*pow(h,6.0));
  fptype coeff3 = 45.0 / (pi*pow(h,6.0));
#endif //ENABLE_DOUBLE_PRECISION
  fptype particleMass = 0.5*doubleRestDensity / (restParticlesPerMeter*restParticlesPerMeter*restParticlesPerMeter);
  densityCoeff = particleMass * coeff1;
  pressureCoeff = 3.0*coeff2 * 0.50*stiffnessPressure * particleMass;
  viscosityCoeff = viscosity * coeff3 * particleMass;

  Vec3 range = domainMax - domainMin;
  nx = (int)(range.x / h);
  ny = (int)(range.y / h);
  nz = (int)(range.z / h);
  assert(nx >= 1 && ny >= 1 && nz >= 1);
  numCells = nx*ny*nz;
  std::cout << "Number of cells: " << numCells << std::endl;
  delta.x = range.x / nx;
  delta.y = range.y / ny;
  delta.z = range.z / nz;
  assert(delta.x >= h && delta.y >= h && delta.z >= h);

  //make sure Cell structure is multiple of estiamted cache line size
  assert(sizeof(Cell) % CACHELINE_SIZE == 0);
  //make sure helper Cell structure is in sync with real Cell structure
#pragma warning( disable : 1684) // warning #1684: conversion from pointer to same-sized integral type (potential portability problem)
  assert(offsetof(struct Cell_aux, padding) == offsetof(struct Cell, padding));

#if defined(WIN32)
  cells = (struct Cell*)_aligned_malloc(sizeof(struct Cell) * numCells, CACHELINE_SIZE);
  cells2 = (struct Cell*)_aligned_malloc(sizeof(struct Cell) * numCells, CACHELINE_SIZE);
  cnumPars = (int*)_aligned_malloc(sizeof(int) * numCells, CACHELINE_SIZE);
  cnumPars2 = (int*)_aligned_malloc(sizeof(int) * numCells, CACHELINE_SIZE);
  last_cells = (struct Cell **)_aligned_malloc(sizeof(struct Cell *) * numCells, CACHELINE_SIZE);
  assert((cells!=NULL) && (cells2!=NULL) && (cnumPars!=NULL) && (cnumPars2!=NULL) && (last_cells!=NULL)); 
#else
  int rv0 = posix_memalign((void **)(&cells), CACHELINE_SIZE, sizeof(struct Cell) * numCells);
  int rv1 = posix_memalign((void **)(&cells2), CACHELINE_SIZE, sizeof(struct Cell) * numCells);
  int rv2 = posix_memalign((void **)(&cnumPars), CACHELINE_SIZE, sizeof(int) * numCells);
  int rv3 = posix_memalign((void **)(&cnumPars2), CACHELINE_SIZE, sizeof(int) * numCells);
  int rv4 = posix_memalign((void **)(&last_cells), CACHELINE_SIZE, sizeof(struct Cell *) * numCells);
  assert((rv0==0) && (rv1==0) && (rv2==0) && (rv3==0) && (rv4==0));
#endif

  // because cells and cells2 are not allocated via new
  // we construct them here
  for(int i=0; i<numCells; ++i)
  {
	  new (&cells[i]) Cell;
	  new (&cells2[i]) Cell;
  }

  memset(cnumPars, 0, numCells*sizeof(int));

  //Always use single precision float variables b/c file format uses single precision
  float px, py, pz, hvx, hvy, hvz, vx, vy, vz;
  for(int i = 0; i < numParticles; ++i)
  {
    file.read((char *)&px, FILE_SIZE_FLOAT);
    file.read((char *)&py, FILE_SIZE_FLOAT);
    file.read((char *)&pz, FILE_SIZE_FLOAT);
    file.read((char *)&hvx, FILE_SIZE_FLOAT);
    file.read((char *)&hvy, FILE_SIZE_FLOAT);
    file.read((char *)&hvz, FILE_SIZE_FLOAT);
    file.read((char *)&vx, FILE_SIZE_FLOAT);
    file.read((char *)&vy, FILE_SIZE_FLOAT);
    file.read((char *)&vz, FILE_SIZE_FLOAT);
    if(!isLittleEndian()) {
      px  = bswap_float(px);
      py  = bswap_float(py);
      pz  = bswap_float(pz);
      hvx = bswap_float(hvx);
      hvy = bswap_float(hvy);
      hvz = bswap_float(hvz);
      vx  = bswap_float(vx);
      vy  = bswap_float(vy);
      vz  = bswap_float(vz);
    }
    int ci = (int)(((fptype)px - domainMin.x) / delta.x);
    int cj = (int)(((fptype)py - domainMin.y) / delta.y);
    int ck = (int)(((fptype)pz - domainMin.z) / delta.z);

    if(ci < 0) ci = 0; else if(ci >= nx) ci = nx-1;
    if(cj < 0) cj = 0; else if(cj >= ny) cj = ny-1;
    if(ck < 0) ck = 0; else if(ck >= nz) ck = nz-1;

    int index = (ck*ny + cj)*nx + ci;
    Cell *cell = &cells[index];

    //go to last cell structure in list
    int np = cnumPars[index];
    while(np > PARTICLES_PER_CELL) {
      cell = cell->next;
      np = np - PARTICLES_PER_CELL;
    }
    //add another cell structure if everything full
    if( (np % PARTICLES_PER_CELL == 0) && (cnumPars[index] != 0) ) {
      cell->next = cellpool_getcell(&pool);
      cell = cell->next;
      np = np - PARTICLES_PER_CELL; // np = 0;
    }

    //add particle to cell
    cell->p[np].x = px;
    cell->p[np].y = py;
    cell->p[np].z = pz;
    cell->hv[np].x = hvx;
    cell->hv[np].y = hvy;
    cell->hv[np].z = hvz;
    cell->v[np].x = vx;
    cell->v[np].y = vy;
    cell->v[np].z = vz;
#ifdef ENABLE_VISUALIZATION
	vMin.x = std::min(vMin.x, cell->v[np].x);
	vMax.x = std::max(vMax.x, cell->v[np].x);
	vMin.y = std::min(vMin.y, cell->v[np].y);
	vMax.y = std::max(vMax.y, cell->v[np].y);
	vMin.z = std::min(vMin.z, cell->v[np].z);
	vMax.z = std::max(vMax.z, cell->v[np].z);
#endif
    ++cnumPars[index];
  }

  std::cout << "Number of particles: " << numParticles << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void SaveFile(char const *fileName)
{
  std::cout << "Saving file \"" << fileName << "\"..." << std::endl;

  std::ofstream file(fileName, std::ios::binary);
  assert(file);

  //Always use single precision float variables b/c file format uses single precision
  if(!isLittleEndian()) {
    float restParticlesPerMeter_le;
    int   numParticles_le;

    restParticlesPerMeter_le = bswap_float((float)restParticlesPerMeter);
    numParticles_le      = bswap_int32(numParticles);
    file.write((char *)&restParticlesPerMeter_le, FILE_SIZE_FLOAT);
    file.write((char *)&numParticles_le,      FILE_SIZE_INT);
  } else {
    file.write((char *)&restParticlesPerMeter, FILE_SIZE_FLOAT);
    file.write((char *)&numParticles, FILE_SIZE_INT);
  }

  int count = 0;
  for(int i = 0; i < numCells; ++i)
  {
    Cell *cell = &cells[i];
    int np = cnumPars[i];
    for(int j = 0; j < np; ++j)
    {
      //Always use single precision float variables b/c file format uses single precision
      float px, py, pz, hvx, hvy, hvz, vx,vy, vz;
      if(!isLittleEndian()) {
        px  = bswap_float((float)(cell->p[j % PARTICLES_PER_CELL].x));
        py  = bswap_float((float)(cell->p[j % PARTICLES_PER_CELL].y));
        pz  = bswap_float((float)(cell->p[j % PARTICLES_PER_CELL].z));
        hvx = bswap_float((float)(cell->hv[j % PARTICLES_PER_CELL].x));
        hvy = bswap_float((float)(cell->hv[j % PARTICLES_PER_CELL].y));
        hvz = bswap_float((float)(cell->hv[j % PARTICLES_PER_CELL].z));
        vx  = bswap_float((float)(cell->v[j % PARTICLES_PER_CELL].x));
        vy  = bswap_float((float)(cell->v[j % PARTICLES_PER_CELL].y));
        vz  = bswap_float((float)(cell->v[j % PARTICLES_PER_CELL].z));
      } else {
        px  = (float)(cell->p[j % PARTICLES_PER_CELL].x);
        py  = (float)(cell->p[j % PARTICLES_PER_CELL].y);
        pz  = (float)(cell->p[j % PARTICLES_PER_CELL].z);
        hvx = (float)(cell->hv[j % PARTICLES_PER_CELL].x);
        hvy = (float)(cell->hv[j % PARTICLES_PER_CELL].y);
        hvz = (float)(cell->hv[j % PARTICLES_PER_CELL].z);
        vx  = (float)(cell->v[j % PARTICLES_PER_CELL].x);
        vy  = (float)(cell->v[j % PARTICLES_PER_CELL].y);
        vz  = (float)(cell->v[j % PARTICLES_PER_CELL].z);
      }
      file.write((char *)&px,  FILE_SIZE_FLOAT);
      file.write((char *)&py,  FILE_SIZE_FLOAT);
      file.write((char *)&pz,  FILE_SIZE_FLOAT);
      file.write((char *)&hvx, FILE_SIZE_FLOAT);
      file.write((char *)&hvy, FILE_SIZE_FLOAT);
      file.write((char *)&hvz, FILE_SIZE_FLOAT);
      file.write((char *)&vx,  FILE_SIZE_FLOAT);
      file.write((char *)&vy,  FILE_SIZE_FLOAT);
      file.write((char *)&vz,  FILE_SIZE_FLOAT);
      ++count;

      //move pointer to next cell in list if end of array is reached
      if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1) {
        cell = cell->next;
      }
    }
  }
  assert(count == numParticles);
}

////////////////////////////////////////////////////////////////////////////////

void CleanUpSim()
{
  // first return extended cells to cell pools
  for(int i=0; i< numCells; ++i)
  {
    Cell& cell = cells[i];
	while(cell.next)
	{
		Cell *temp = cell.next;
		cell.next = temp->next;
		cellpool_returncell(&pool, temp);
	}
  }
  // now return cell pools
  cellpool_destroy(&pool);

#if defined(WIN32)
  _aligned_free(cells);
  _aligned_free(cells2);
  _aligned_free(cnumPars);
  _aligned_free(cnumPars2);
  _aligned_free(last_cells);
#else
  free(cells);
  free(cells2);
  free(cnumPars);
  free(cnumPars2);
  free(last_cells);
#endif
}

////////////////////////////////////////////////////////////////////////////////

void RebuildGrid()
{
  //swap src and dest arrays with particles
  std::swap(cells, cells2);
  //swap src and dest arrays with counts of particles
  std::swap(cnumPars, cnumPars2);
  // Note, in parallel versions the above swaps may
  // occure outside RebuildGrid()

  //initialize destination data structures
  memset(cnumPars, 0, numCells*sizeof(int));
  for(int i=0; i<numCells; i++)
  {
    cells[i].next = NULL;
    last_cells[i] = &cells[i];
  }

  //iterate through source cell lists
  for(int i = 0; i < numCells; ++i)
  {
    Cell *cell2 = &cells2[i];
    int np2 = cnumPars2[i];
    //iterate through source particles
    for(int j = 0; j < np2; ++j)
    {
      //get destination for source particle
      int ci = (int)((cell2->p[j % PARTICLES_PER_CELL].x - domainMin.x) / delta.x);
      int cj = (int)((cell2->p[j % PARTICLES_PER_CELL].y - domainMin.y) / delta.y);
      int ck = (int)((cell2->p[j % PARTICLES_PER_CELL].z - domainMin.z) / delta.z);
	  // confine to domain
	  // Note, if ProcessCollisions() is working properly these tests are useless
      if(ci < 0) ci = 0; else if(ci >= nx) ci = nx-1;
      if(cj < 0) cj = 0; else if(cj >= ny) cj = ny-1;
      if(ck < 0) ck = 0; else if(ck >= nz) ck = nz-1;

#ifdef ENABLE_CFL_CHECK
      //check that source cell is a neighbor of destination cell
      bool cfl_cond_satisfied=false;
      for(int di = -1; di <= 1; ++di)
        for(int dj = -1; dj <= 1; ++dj)
          for(int dk = -1; dk <= 1; ++dk)
          {
            int ii = ci + di;
            int jj = cj + dj;
            int kk = ck + dk;
            if(ii >= 0 && ii < nx && jj >= 0 && jj < ny && kk >= 0 && kk < nz)
            {
              int index = (kk*ny + jj)*nx + ii;
              if(index == i)
              {
                cfl_cond_satisfied=true;
                break;
              }
            }
          }
      if(!cfl_cond_satisfied)
      {
        std::cerr << "FATAL ERROR: Courant–Friedrichs–Lewy condition not satisfied." << std::endl;
        exit(1);
      }
#endif //ENABLE_CFL_CHECK

      //get last pointer in correct destination cell list
      int index = (ck*ny + cj)*nx + ci;
      Cell *cell = last_cells[index];
      int np = cnumPars[index];

      //add another cell structure if everything full
      if( (np % PARTICLES_PER_CELL == 0) && (cnumPars[index] != 0) ) {
        cell->next = cellpool_getcell(&pool);
        cell = cell->next;
        last_cells[index] = cell;
      }
      ++cnumPars[index];

      //copy source to destination particle
      cell->p[np % PARTICLES_PER_CELL].x = cell2->p[j % PARTICLES_PER_CELL].x;
      cell->p[np % PARTICLES_PER_CELL].y = cell2->p[j % PARTICLES_PER_CELL].y;
      cell->p[np % PARTICLES_PER_CELL].z = cell2->p[j % PARTICLES_PER_CELL].z;
      cell->hv[np % PARTICLES_PER_CELL].x = cell2->hv[j % PARTICLES_PER_CELL].x;
      cell->hv[np % PARTICLES_PER_CELL].y = cell2->hv[j % PARTICLES_PER_CELL].y;
      cell->hv[np % PARTICLES_PER_CELL].z = cell2->hv[j % PARTICLES_PER_CELL].z;
      cell->v[np % PARTICLES_PER_CELL].x = cell2->v[j % PARTICLES_PER_CELL].x;
      cell->v[np % PARTICLES_PER_CELL].y = cell2->v[j % PARTICLES_PER_CELL].y;
      cell->v[np % PARTICLES_PER_CELL].z = cell2->v[j % PARTICLES_PER_CELL].z;

      //move pointer to next source cell in list if end of array is reached
      if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1) {
        Cell *temp = cell2;
        cell2 = cell2->next;
        //return cells to pool that are not statically allocated head of lists
        if(temp != &cells2[i]) {
          cellpool_returncell(&pool, temp);
        }
      }

    } // for(int j = 0; j < np2; ++j)
    //return cells to pool that are not statically allocated head of lists
    if((cell2 != NULL) && (cell2 != &cells2[i])) {
      cellpool_returncell(&pool, cell2);
    }
  } // for(int i = 0; i < numCells; ++i)
}

////////////////////////////////////////////////////////////////////////////////

int GetNeighborCells(int ci, int cj, int ck, int *neighCells)
{
  int numNeighCells = 0;

  // have the nearest particles first -> help branch prediction
  int my_index = (ck*ny + cj)*nx + ci;
  neighCells[numNeighCells] = my_index;
  ++numNeighCells;

  for(int di = -1; di <= 1; ++di)
    for(int dj = -1; dj <= 1; ++dj)
      for(int dk = -1; dk <= 1; ++dk)
      {
        int ii = ci + di;
        int jj = cj + dj;
        int kk = ck + dk;
        if(ii >= 0 && ii < nx && jj >= 0 && jj < ny && kk >= 0 && kk < nz)
        {
          int index = (kk*ny + jj)*nx + ii;
          if((index < my_index) && (cnumPars[index] != 0))
          {
            neighCells[numNeighCells] = index;
            ++numNeighCells;
          }
        }
      }

  return numNeighCells;
}

////////////////////////////////////////////////////////////////////////////////

void ComputeForces()
{
  for(int i = 0; i < numCells; ++i)
  {
    Cell *cell = &cells[i];
    int np = cnumPars[i];
    for(int j = 0; j < np; ++j)
    {
      cell->density[j % PARTICLES_PER_CELL] = 0.0;
      cell->a[j % PARTICLES_PER_CELL] = externalAcceleration;
      //move pointer to next cell in list if end of array is reached
      if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1) {
        cell = cell->next;
      }
    }
  }

  int neighCells[3*3*3];

  int cindex = 0;
  for(int ck = 0; ck < nz; ++ck)
    for(int cj = 0; cj < ny; ++cj)
      for(int ci = 0; ci < nx; ++ci, ++cindex)
      {
        int np = cnumPars[cindex];
        if(np == 0)
          continue;

        int numNeighCells = GetNeighborCells(ci, cj, ck, neighCells);

        Cell *cell = &cells[cindex];
        for(int ipar = 0; ipar < np; ++ipar)
        {
          for(int inc = 0; inc < numNeighCells; ++inc)
          {
            int cindexNeigh = neighCells[inc];
            Cell *neigh = &cells[cindexNeigh];
            int numNeighPars = cnumPars[cindexNeigh];
            for(int iparNeigh = 0; iparNeigh < numNeighPars; ++iparNeigh)
            {
              //Check address to make sure densities are computed only once per pair
              if(&neigh->p[iparNeigh % PARTICLES_PER_CELL] < &cell->p[ipar % PARTICLES_PER_CELL])
              {
                fptype distSq = (cell->p[ipar % PARTICLES_PER_CELL] - neigh->p[iparNeigh % PARTICLES_PER_CELL]).GetLengthSq();
                if(distSq < hSq)
                {
                  fptype t = hSq - distSq;
                  fptype tc = t*t*t;
                  cell->density[ipar % PARTICLES_PER_CELL] += tc;
                  neigh->density[iparNeigh % PARTICLES_PER_CELL] += tc;
                }
              }
              //move pointer to next cell in list if end of array is reached
              if(iparNeigh % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1) {
                neigh = neigh->next;
              }
            }
          }
          //move pointer to next cell in list if end of array is reached
          if(ipar % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1) {
            cell = cell->next;
          }
        }
      }

  const fptype tc = hSq*hSq*hSq;
  for(int i = 0; i < numCells; ++i)
  {
    Cell *cell = &cells[i];
    int np = cnumPars[i];
    for(int j = 0; j < np; ++j)
    {
      cell->density[j % PARTICLES_PER_CELL] += tc;
      cell->density[j % PARTICLES_PER_CELL] *= densityCoeff;
      //move pointer to next cell in list if end of array is reached
      if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1) {
        cell = cell->next;
      }
    }
  }

  cindex = 0;
  for(int ck = 0; ck < nz; ++ck)
    for(int cj = 0; cj < ny; ++cj)
      for(int ci = 0; ci < nx; ++ci, ++cindex)
      {
        int np = cnumPars[cindex];
        if(np == 0)
          continue;

        int numNeighCells = GetNeighborCells(ci, cj, ck, neighCells);

        Cell *cell = &cells[cindex];
        for(int ipar = 0; ipar < np; ++ipar)
        {
          for(int inc = 0; inc < numNeighCells; ++inc)
          {
            int cindexNeigh = neighCells[inc];
            Cell *neigh = &cells[cindexNeigh];
            int numNeighPars = cnumPars[cindexNeigh];
            for(int iparNeigh = 0; iparNeigh < numNeighPars; ++iparNeigh)
            {
              //Check address to make sure forces are computed only once per pair
              if(&neigh->p[iparNeigh % PARTICLES_PER_CELL] < &cell->p[ipar % PARTICLES_PER_CELL])
              {
                Vec3 disp = cell->p[ipar % PARTICLES_PER_CELL] - neigh->p[iparNeigh % PARTICLES_PER_CELL];
                fptype distSq = disp.GetLengthSq();
                if(distSq < hSq)
                {
#ifndef ENABLE_DOUBLE_PRECISION
                  fptype dist = sqrtf(std::max(distSq, (fptype)1e-12));
#else
                  fptype dist = sqrt(std::max(distSq, 1e-12));
#endif //ENABLE_DOUBLE_PRECISION
                  fptype hmr = h - dist;

                  Vec3 acc = disp * pressureCoeff * (hmr*hmr/dist) * (cell->density[ipar % PARTICLES_PER_CELL]+neigh->density[iparNeigh % PARTICLES_PER_CELL] - doubleRestDensity);
                  acc += (neigh->v[iparNeigh % PARTICLES_PER_CELL] - cell->v[ipar % PARTICLES_PER_CELL]) * viscosityCoeff * hmr;
                  acc /= cell->density[ipar % PARTICLES_PER_CELL] * neigh->density[iparNeigh % PARTICLES_PER_CELL];

                  cell->a[ipar % PARTICLES_PER_CELL] += acc;
                  neigh->a[iparNeigh % PARTICLES_PER_CELL] -= acc;
                }
              }
              //move pointer to next cell in list if end of array is reached
              if(iparNeigh % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1) {
                neigh = neigh->next;
              }
            }
          }
          //move pointer to next cell in list if end of array is reached
          if(ipar % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1) {
            cell = cell->next;
          }
        }
      }
}

////////////////////////////////////////////////////////////////////////////////
// ProcessCollisions() with container walls
// Under the assumptions that
// a) a particle will not penetrate a wall
// b) a particle will not migrate further than once cell
// c) the parSize is smaller than a cell
// then only the particles at the perimiters may be influenced by the walls
#if 0
void ProcessCollisions()
{
	for(int i = 0; i < numCells; ++i)
	{
        Cell *cell = &cells[i];
        int np = cnumPars[i];
        for(int j = 0; j < np; ++j)
        {
          Vec3 pos = cell->p[j % PARTICLES_PER_CELL] + cell->hv[j % PARTICLES_PER_CELL] * timeStep;

          fptype diff = parSize - (pos.x - domainMin.x);
          if(diff > epsilon)
            cell->a[j % PARTICLES_PER_CELL].x += stiffnessCollisions*diff - damping*cell->v[j % PARTICLES_PER_CELL].x;

          diff = parSize - (domainMax.x - pos.x);
          if(diff > epsilon)
            cell->a[j % PARTICLES_PER_CELL].x -= stiffnessCollisions*diff + damping*cell->v[j % PARTICLES_PER_CELL].x;

          diff = parSize - (pos.y - domainMin.y);
          if(diff > epsilon)
            cell->a[j % PARTICLES_PER_CELL].y += stiffnessCollisions*diff - damping*cell->v[j % PARTICLES_PER_CELL].y;

          diff = parSize - (domainMax.y - pos.y);
          if(diff > epsilon)
            cell->a[j % PARTICLES_PER_CELL].y -= stiffnessCollisions*diff + damping*cell->v[j % PARTICLES_PER_CELL].y;

          diff = parSize - (pos.z - domainMin.z);
          if(diff > epsilon)
            cell->a[j % PARTICLES_PER_CELL].z += stiffnessCollisions*diff - damping*cell->v[j % PARTICLES_PER_CELL].z;

          diff = parSize - (domainMax.z - pos.z);
          if(diff > epsilon)
            cell->a[j % PARTICLES_PER_CELL].z -= stiffnessCollisions*diff + damping*cell->v[j % PARTICLES_PER_CELL].z;

          //move pointer to next cell in list if end of array is reached
          if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1)
            cell = cell->next;
		}
	}
}
#else
// Notes on USE_ImpeneratableWall
// When particle is detected beyond cell wall it is repositioned at cell wall
// velocity is not changed, thus conserving momentum.
// What this means though it the prior AdvanceParticles had positioned the
// particle beyond the cell wall and thus the visualization will show these
// as artifacts. The proper place for USE_ImpeneratableWall is after AdvanceParticles.
// This would entail a 2nd pass on the perimiters after AdvanceParticles (as opposed
// to inside AdvanceParticles). Your fluid dynamisist should properly devise the
// equasions. 
void ProcessCollisions()
{
	int x,y,z, index;
	x=0;	// along the domainMin.x wall
	for(y=0; y<ny; ++y)
	{
		for(z=0; z<nz; ++z)
		{
			int ci = (z*ny + y)*nx + x;
			Cell *cell = &cells[ci];
			int np = cnumPars[ci];
			for(int j = 0; j < np; ++j)
			{
				int ji = j % PARTICLES_PER_CELL;
				fptype pos_x = cell->p[ji].x + cell->hv[ji].x * timeStep;
				fptype diff = parSize - (pos_x - domainMin.x);
				if(diff > epsilon)
					cell->a[ji].x += stiffnessCollisions*diff - damping*cell->v[ji].x;
				//move pointer to next cell in list if end of array is reached
				if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1)
					cell = cell->next;
			}
		}
	}
	x=nx-1;	// along the domainMax.x wall
	for(y=0; y<ny; ++y)
	{
		for(z=0; z<nz; ++z)
		{
			int ci = (z*ny + y)*nx + x;
			Cell *cell = &cells[ci];
			int np = cnumPars[ci];
			for(int j = 0; j < np; ++j)
			{
				int ji = j % PARTICLES_PER_CELL;
				fptype pos_x = cell->p[ji].x + cell->hv[ji].x * timeStep;
				fptype diff = parSize - (domainMax.x - pos_x);
				if(diff > epsilon)
					cell->a[ji].x -= stiffnessCollisions*diff + damping*cell->v[ji].x;
				//move pointer to next cell in list if end of array is reached
				if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1)
					cell = cell->next;
			}
		}
	}
	y=0;	// along the domainMin.y wall
	for(x=0; x<nx; ++x)
	{
		for(z=0; z<nz; ++z)
		{
			int ci = (z*ny + y)*nx + x;
			Cell *cell = &cells[ci];
			int np = cnumPars[ci];
			for(int j = 0; j < np; ++j)
			{
				int ji = j % PARTICLES_PER_CELL;
				fptype pos_y = cell->p[ji].y + cell->hv[ji].y * timeStep;
				fptype diff = parSize - (pos_y - domainMin.y);
				if(diff > epsilon)
					cell->a[ji].y += stiffnessCollisions*diff - damping*cell->v[ji].y;
				//move pointer to next cell in list if end of array is reached
				if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1)
					cell = cell->next;
			}
		}
	}
	y=ny-1;	// along the domainMax.y wall
	for(x=0; x<nx; ++x)
	{
		for(z=0; z<nz; ++z)
		{
			int ci = (z*ny + y)*nx + x;
			Cell *cell = &cells[ci];
			int np = cnumPars[ci];
			for(int j = 0; j < np; ++j)
			{
				int ji = j % PARTICLES_PER_CELL;
				fptype pos_y = cell->p[ji].y + cell->hv[ji].y * timeStep;
				fptype diff = parSize - (domainMax.y - pos_y);
				if(diff > epsilon)
					cell->a[ji].y -= stiffnessCollisions*diff + damping*cell->v[ji].y;
				//move pointer to next cell in list if end of array is reached
				if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1)
					cell = cell->next;
			}
		}
	}
	z=0;	// along the domainMin.z wall
	for(x=0; x<nx; ++x)
	{
		for(y=0; y<ny; ++y)
		{
			int ci = (z*ny + y)*nx + x;
			Cell *cell = &cells[ci];
			int np = cnumPars[ci];
			for(int j = 0; j < np; ++j)
			{
				int ji = j % PARTICLES_PER_CELL;
				fptype pos_z = cell->p[ji].z + cell->hv[ji].z * timeStep;
				fptype diff = parSize - (pos_z - domainMin.z);
				if(diff > epsilon)
					cell->a[ji].z += stiffnessCollisions*diff - damping*cell->v[ji].z;
				//move pointer to next cell in list if end of array is reached
				if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1)
					cell = cell->next;
			}
		}
	}
	z=nz-1;	// along the domainMax.z wall
	for(x=0; x<nx; ++x)
	{
		for(y=0; y<ny; ++y)
		{
			int ci = (z*ny + y)*nx + x;
			Cell *cell = &cells[ci];
			int np = cnumPars[ci];
			for(int j = 0; j < np; ++j)
			{
				int ji = j % PARTICLES_PER_CELL;
				fptype pos_z = cell->p[ji].z + cell->hv[ji].z * timeStep;
				fptype diff = parSize - (domainMax.z - pos_z);
				if(diff > epsilon)
					cell->a[ji].z -= stiffnessCollisions*diff + damping*cell->v[ji].z;
				//move pointer to next cell in list if end of array is reached
				if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1)
					cell = cell->next;
			}
		}
	}
}
#define USE_ImpeneratableWall
#if defined(USE_ImpeneratableWall)
void ProcessCollisions2()
{
	int x,y,z, index;
	x=0;	// along the domainMin.x wall
	for(y=0; y<ny; ++y)
	{
		for(z=0; z<nz; ++z)
		{
			int ci = (z*ny + y)*nx + x;
			Cell *cell = &cells[ci];
			int np = cnumPars[ci];
			for(int j = 0; j < np; ++j)
			{
				int ji = j % PARTICLES_PER_CELL;
				fptype diff = cell->p[ji].x - domainMin.x;
				if(diff < Zero)
				{
					cell->p[ji].x = domainMin.x - diff;
					cell->v[ji].x = -cell->v[ji].x;
					cell->hv[ji].x = -cell->hv[ji].x;
				}
				//move pointer to next cell in list if end of array is reached
				if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1)
					cell = cell->next;
			}
		}
	}
	x=nx-1;	// along the domainMax.x wall
	for(y=0; y<ny; ++y)
	{
		for(z=0; z<nz; ++z)
		{
			int ci = (z*ny + y)*nx + x;
			Cell *cell = &cells[ci];
			int np = cnumPars[ci];
			for(int j = 0; j < np; ++j)
			{
				int ji = j % PARTICLES_PER_CELL;
				fptype diff = domainMax.x - cell->p[ji].x;
				if(diff < Zero)
				{
					cell->p[ji].x = domainMax.x + diff;
					cell->v[ji].x = -cell->v[ji].x;
					cell->hv[ji].x = -cell->hv[ji].x;
				}
				//move pointer to next cell in list if end of array is reached
				if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1)
					cell = cell->next;
			}
		}
	}
	y=0;	// along the domainMin.y wall
	for(x=0; x<nx; ++x)
{
	for(z=0; z<nz; ++z)
		{
			int ci = (z*ny + y)*nx + x;
			Cell *cell = &cells[ci];
			int np = cnumPars[ci];
			for(int j = 0; j < np; ++j)
			{
				int ji = j % PARTICLES_PER_CELL;
				fptype diff = cell->p[ji].y - domainMin.y;
				if(diff < Zero)
				{
					cell->p[ji].y = domainMin.y - diff;
					cell->v[ji].y = -cell->v[ji].y;
					cell->hv[ji].y = -cell->hv[ji].y;
				}
				//move pointer to next cell in list if end of array is reached
				if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1)
					cell = cell->next;
			}
		}
	}
	y=ny-1;	// along the domainMax.y wall
	for(x=0; x<nx; ++x)
	{
		for(z=0; z<nz; ++z)
		{
			int ci = (z*ny + y)*nx + x;
			Cell *cell = &cells[ci];
			int np = cnumPars[ci];
			for(int j = 0; j < np; ++j)
			{
				int ji = j % PARTICLES_PER_CELL;
				fptype diff = domainMax.y - cell->p[ji].y;
				if(diff < Zero)
				{
					cell->p[ji].y = domainMax.y + diff;
					cell->v[ji].y = -cell->v[ji].y;
					cell->hv[ji].y = -cell->hv[ji].y;
				//move pointer to next cell in list if end of array is reached
				if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1)
					cell = cell->next;
				}
			}
		}
	}
	z=0;	// along the domainMin.z wall
	for(x=0; x<nx; ++x)
	{
		for(y=0; y<ny; ++y)
		{
			int ci = (z*ny + y)*nx + x;
			Cell *cell = &cells[ci];
			int np = cnumPars[ci];
			for(int j = 0; j < np; ++j)
			{
				int ji = j % PARTICLES_PER_CELL;
				fptype diff = cell->p[ji].z - domainMin.z;
				if(diff < Zero)
				{
					cell->p[ji].z = domainMin.z - diff;
					cell->v[ji].z = -cell->v[ji].z;
					cell->hv[ji].z = -cell->hv[ji].z;
				}
				//move pointer to next cell in list if end of array is reached
				if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1)
					cell = cell->next;
			}
		}
	}
	z=nz-1;	// along the domainMax.z wall
	for(x=0; x<nx; ++x)
	{
		for(y=0; y<ny; ++y)
		{
			int ci = (z*ny + y)*nx + x;
			Cell *cell = &cells[ci];
			int np = cnumPars[ci];
			for(int j = 0; j < np; ++j)
			{
				int ji = j % PARTICLES_PER_CELL;
				fptype diff = domainMax.z - cell->p[ji].z;
				if(diff < Zero)
				{
					cell->p[ji].z = domainMax.z + diff;
					cell->v[ji].z = -cell->v[ji].z;
					cell->hv[ji].z = -cell->hv[ji].z;
				}
				//move pointer to next cell in list if end of array is reached
				if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1)
					cell = cell->next;
			}
		}
	}
}
#endif
#endif
////////////////////////////////////////////////////////////////////////////////

void AdvanceParticles()
{
  for(int i = 0; i < numCells; ++i)
  {
    Cell *cell = &cells[i];
    int np = cnumPars[i];
    for(int j = 0; j < np; ++j)
    {
      Vec3 v_half = cell->hv[j % PARTICLES_PER_CELL] + cell->a[j % PARTICLES_PER_CELL]*timeStep;
#if defined(USE_ImpeneratableWall)
	  // N.B. The integration of the position can place the particle
	  // outside the domain. Although we could place a test in this loop
	  // we would be unnecessarily testing particles on interior cells.
	  // Therefore, to reduce the amount of computations we make a later
	  // pass on the perimiter cells to account for particle migration
	  // beyond domain
#endif
      cell->p[j % PARTICLES_PER_CELL] += v_half * timeStep;
      cell->v[j % PARTICLES_PER_CELL] = cell->hv[j % PARTICLES_PER_CELL] + v_half;
      cell->v[j % PARTICLES_PER_CELL] *= 0.5;
      cell->hv[j % PARTICLES_PER_CELL] = v_half;

      //move pointer to next cell in list if end of array is reached
      if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1) {
        cell = cell->next;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void AdvanceFrame()
{
  RebuildGrid();
  ComputeForces();
  ProcessCollisions();
  AdvanceParticles();
#if defined(USE_ImpeneratableWall)
  // N.B. The integration of the position can place the particle
  // outside the domain. We now make a pass on the perimiter cells
  // to account for particle migration beyond domain.
  ProcessCollisions2();
#endif
#ifdef ENABLE_STATISTICS
  float mean, stddev;
  int i;

  mean = (float)numParticles/(float)numCells;
  stddev = 0.0;
  for(i=0; i<numCells; i++) {
    stddev += (mean-cnumPars[i])*(mean-cnumPars[i]);
  }
  stddev = sqrtf(stddev);
  std::cout << "Cell statistics: mean=" << mean << " particles, stddev=" << stddev << " particles." << std::endl;
#endif
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
#ifdef PARSEC_VERSION
#define __PARSEC_STRING(x) #x
#define __PARSEC_XSTRING(x) __PARSEC_STRING(x)
        std::cout << "PARSEC Benchmark Suite Version "__PARSEC_XSTRING(PARSEC_VERSION) << std::endl << std::flush;
#else
        std::cout << "PARSEC Benchmark Suite" << std::endl << std::flush;
#endif //PARSEC_VERSION
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_bench_begin(__parsec_fluidanimate);
#endif

  if(argc < 4 || argc >= 6)
  {
    std::cout << "Usage: " << argv[0] << " <threadnum> <framenum> <.fluid input file> [.fluid output file]" << std::endl;
    return -1;
  }

  int threadnum = atoi(argv[1]);
  int framenum = atoi(argv[2]);

  //Check arguments
  if(threadnum != 1) {
    std::cerr << "<threadnum> must be 1 (serial version)" << std::endl;
    return -1;
  }
  if(framenum < 1) {
    std::cerr << "<framenum> must at least be 1" << std::endl;
    return -1;
  }

#ifdef ENABLE_CFL_CHECK
  std::cout << "WARNING: Check for Courant–Friedrichs–Lewy condition enabled. Do not use for performance measurements." << std::endl;
#endif

  InitSim(argv[3]);
#ifdef ENABLE_VISUALIZATION
  InitVisualizationMode(&argc, argv, &AdvanceFrame, &numCells, &cells, &cnumPars);
#endif

#ifndef ENABLE_VISUALIZATION

//core of benchmark program (the Region-of-Interest)
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_begin();
#endif
  for(int i = 0; i < framenum; ++i)
    AdvanceFrame();
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_end();
#endif

#else //ENABLE_VISUALIZATION
  Visualize();
#endif //ENABLE_VISUALIZATION

  if(argc > 4)
    SaveFile(argv[4]);
  CleanUpSim();

#ifdef ENABLE_PARSEC_HOOKS
  __parsec_bench_end();
#endif

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

