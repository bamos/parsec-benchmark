//Code written by Richard O. Lee, Gilberto Contreras and Christian Bienia
//Modified by Christian Fensch

#include <cstdlib>
#include <cstring>

#include <iostream>
#include <fstream>
#include <math.h>
#include <assert.h>

#define NUM_TASKS     (8)
#include "tbb/blocked_range.h"
#include "tbb/parallel_reduce.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/spin_mutex.h"

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

////////////////////////////////////////////////////////////////////////////////

typedef tbb::spin_mutex tbbMutex;

cellpool *pools; //each thread has its private cell pool

fptype restParticlesPerMeter, h, hSq;
fptype densityCoeff, pressureCoeff, viscosityCoeff;

int nx, ny, nz;    // number of grid cells in each dimension
Vec3 delta;        // cell dimensions
int numParticles = 0;
int numCells = 0;
Cell *cells = 0;
Cell *cells2 = 0;
int *cnumPars = 0;
int *cnumPars2 = 0;
Cell **last_cells = NULL; //helper array with pointers to last cell structure of "cells" array lists
#ifdef ENABLE_VISUALIZATION
Vec3 vMax(0.0,0.0,0.0);
Vec3 vMin(0.0,0.0,0.0);
#endif

int XDIVS = 1;  // number of partitions in X
int ZDIVS = 1;  // number of partitions in Z

#define NUM_GRIDS  ((XDIVS) * (ZDIVS))
#define MUTEXES_PER_CELL 128
#define CELL_MUTEX_ID 0

struct Grid
{
  union {
    struct {
      int sx, sy, sz;
      int ex, ey, ez;
    };
    unsigned char pp[CACHELINE_SIZE];
  };
} *grids;
bool *border;      // flags which cells lie on grid boundaries
tbbMutex **mutex;  // used to lock cells in RebuildGrid and also particles in other functions

////////////////////////////////////////////////////////////////////////////////

/*
 * hmgweight
 *
 * Computes the hamming weight of x
 *
 * x      - input value
 * lsb    - if x!=0 position of smallest bit set, else -1
 *
 * return - the hamming weight
 */
unsigned int hmgweight(unsigned int x, int *lsb) {
  unsigned int weight=0;
  unsigned int mask= 1;
  unsigned int count=0;

  *lsb=-1;
  while(x > 0) {
    unsigned int temp;
    temp=(x&mask);
    if((x&mask) == 1) {
      weight++;
      if(*lsb == -1) *lsb = count;
    }
    x >>= 1;
    count++;
  }

  return weight;
}

void InitSim(char const *fileName, unsigned int threadnum)
{
  //Compute partitioning based on square root of number of threads
  //NOTE: Other partition sizes are possible as long as XDIVS * ZDIVS == threadnum,
  //      but communication is minimal (and hence optimal) if XDIVS == ZDIVS
  int lsb;
  if(hmgweight(threadnum,&lsb) != 1) {
    std::cerr << "Number of threads must be a power of 2" << std::endl;
    exit(1);
  }
  XDIVS = 1<<(lsb/2);
  ZDIVS = 1<<(lsb/2);
  if(XDIVS*ZDIVS != threadnum) XDIVS*=2;
  assert(XDIVS * ZDIVS == threadnum);

  grids = new struct Grid[NUM_GRIDS];
  pools = new cellpool[NUM_GRIDS*NUM_TASKS];

  //Load input particles
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
  for(int i=0; i<NUM_GRIDS*NUM_TASKS; i++) cellpool_init(&pools[i], numParticles/(NUM_GRIDS*NUM_TASKS));

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

  assert(nx >= XDIVS && nz >= ZDIVS);
  int gi = 0;
  int sx, sz, ex, ez;
  ex = 0;
  for(int i = 0; i < XDIVS; ++i)
  {
    sx = ex;
    ex = (int)((fptype)(nx)/(fptype)(XDIVS) * (i+1) + 0.5);
    assert(sx < ex);

    ez = 0;
    for(int j = 0; j < ZDIVS; ++j, ++gi)
    {
      sz = ez;
      ez = (int)((fptype)(nz)/(fptype)(ZDIVS) * (j+1) + 0.5);
      assert(sz < ez);

      grids[gi].sx = sx;
      grids[gi].ex = ex;
      grids[gi].sy = 0;
      grids[gi].ey = ny;
      grids[gi].sz = sz;
      grids[gi].ez = ez;
    }
  }
  assert(gi == NUM_GRIDS);

  border = new bool[numCells];
  for(int i = 0; i < NUM_GRIDS; ++i)
    for(int iz = grids[i].sz; iz < grids[i].ez; ++iz)
      for(int iy = grids[i].sy; iy < grids[i].ey; ++iy)
        for(int ix = grids[i].sx; ix < grids[i].ex; ++ix)
        {
          int index = (iz*ny + iy)*nx + ix;
          border[index] = false;
          for(int dk = -1; dk <= 1; ++dk)
            for(int dj = -1; dj <= 1; ++dj)
              for(int di = -1; di <= 1; ++di)
              {
                int ci = ix + di;
                int cj = iy + dj;
                int ck = iz + dk;

                if(ci < 0) ci = 0; else if(ci > (nx-1)) ci = nx-1;
                if(cj < 0) cj = 0; else if(cj > (ny-1)) cj = ny-1;
                if(ck < 0) ck = 0; else if(ck > (nz-1)) ck = nz-1;

                if( ci < grids[i].sx || ci >= grids[i].ex ||
                  cj < grids[i].sy || cj >= grids[i].ey ||
                  ck < grids[i].sz || ck >= grids[i].ez )
                  border[index] = true;
              }
        }


  mutex = new tbbMutex *[numCells];
  for(int i = 0; i < numCells; ++i)
  {
    assert(CELL_MUTEX_ID < MUTEXES_PER_CELL);
    int n = (border[i] ? MUTEXES_PER_CELL : CELL_MUTEX_ID+1);
    mutex[i] = new tbbMutex[n];
  }

  //make sure Cell structure is multiple of estiamted cache line size
  assert(sizeof(Cell) % CACHELINE_SIZE == 0);
  //make sure helper Cell structure is in sync with real Cell structure
  assert(offsetof(struct Cell_aux, padding) == offsetof(struct Cell, padding));

#if defined(WIN32)
  cells = (struct Cell*)_aligned_malloc(sizeof(struct Cell) * numCells, CACHELINE_SIZE);
  cells2 = (struct Cell*)_aligned_malloc(sizeof(struct Cell) * numCells, CACHELINE_SIZE);
  cnumPars = (int*)_aligned_malloc(sizeof(int) * numCells, CACHELINE_SIZE);
  cnumPars2 = (int*)_aligned_malloc(sizeof(int) * numCells, CACHELINE_SIZE);
  last_cells = (struct Cell **)_aligned_malloc(sizeof(struct Cell *) * numCells, CACHELINE_SIZE);
  assert((cells!=NULL) && (cells2!=NULL) && (cnumPars!=NULL) && (cnumPars2!=NULL) && (last_cells!=NULL));
#elif defined(SPARC_SOLARIS)
  cells = (Cell*)memalign(CACHELINE_SIZE, sizeof(struct Cell) * numCells);
  cells2 =  (Cell*)memalign(CACHELINE_SIZE, sizeof(struct Cell) * numCells);
  cnumPars =  (int*)memalign(CACHELINE_SIZE, sizeof(int) * numCells);
  cnumPars2 =  (int*)memalign(CACHELINE_SIZE, sizeof(int) * numCells);
  last_cells =  (Cell**)memalign(CACHELINE_SIZE, sizeof(struct Cell *) * numCells);
  assert((cells!=0) && (cells2!=0) && (cnumPars!=0) && (cnumPars2!=0) && (last_cells!=0));
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

  //Always use single precision float variables b/c file format uses single precision float
  int pool_id = 0;
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

    int ci = (int)((px - domainMin.x) / delta.x);
    int cj = (int)((py - domainMin.y) / delta.y);
    int ck = (int)((pz - domainMin.z) / delta.z);

    if(ci < 0) ci = 0; else if(ci > (nx-1)) ci = nx-1;
    if(cj < 0) cj = 0; else if(cj > (ny-1)) cj = ny-1;
    if(ck < 0) ck = 0; else if(ck > (nz-1)) ck = nz-1;

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
      //Get cells from pools in round-robin fashion to balance load during parallel phase
      cell->next = cellpool_getcell(&pools[pool_id]);
      pool_id = (pool_id+1) % (NUM_GRIDS*NUM_TASKS);
      cell = cell->next;
      np = np - PARTICLES_PER_CELL;
    }

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
    file.write((char *)&numParticles,      FILE_SIZE_INT);
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
		cellpool_returncell(&pools[0], temp);
	}
  }
  // now return cell pools
  //NOTE: Cells from cell pools can migrate to different pools during the parallel phase.
  //      This is no problem as long as all cell pools are destroyed together. Each pool
  //      uses its internal meta information to free exactly the cells which it allocated
  //      itself. This guarantees that all allocated cells will be freed but it might
  //      render other cell pools unusable so they also have to be destroyed.
  for(int i=0; i<NUM_GRIDS*NUM_TASKS; i++) cellpool_destroy(&pools[i]);

  for(int i = 0; i < numCells; ++i)
  {
    assert(CELL_MUTEX_ID < MUTEXES_PER_CELL);
    int n = (border[i] ? MUTEXES_PER_CELL : CELL_MUTEX_ID+1);
    delete[] mutex[i];
  }
  delete[] mutex;

  delete[] border;

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

  delete[] grids;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
class WorkerLauncher: public tbb::task {
  bool is_continuation;
public:
  int i_;
  WorkerLauncher(int i):i_(i),is_continuation(false){}
  task *execute() {
    tbb::task_list list;
    int sz;
    int ez;

    if(!is_continuation) {
      recycle_as_continuation();
      set_ref_count(NUM_TASKS);
      sz = grids[i_].sz;
      ez = grids[i_].ez;

      for(int t=1;t<NUM_TASKS;t++)
        list.push_back( *new( tbb::task::allocate_child() ) T(t, i_, sz, ez)  );
      T &me = *new (tbb::task::allocate_child()) T(0,i_,sz,ez);
      spawn(list);
      is_continuation = true;
      return &me;
    } else {
      return NULL;
    }
  }
};

template<class T>
class GridLauncher: public tbb::task {
  bool is_continuation;
  int num_grids;
public:
  GridLauncher(int n_):num_grids(n_), is_continuation(false) {}

  task *execute() {
    tbb::task_list list;

    if(!is_continuation) {
      recycle_as_continuation();
      set_ref_count(num_grids);

      for(int i=1;i<num_grids;i++)
        list.push_back( *new( tbb::task::allocate_child() ) WorkerLauncher<T>(i) );
      WorkerLauncher<T> &me = *new (tbb::task::allocate_child()) WorkerLauncher<T>(0);
      spawn(list);
      is_continuation = true;
      return &me;
    } else {
      return NULL;
    }
  }
};


class ClearParticlesMTWorker: public tbb::task {
  int tid;
  int pid;
  int start;
  int end;
public:
  ClearParticlesMTWorker(int id, int tid_, int s, int e):
    pid(id), tid(tid_), start(s), end(e) {}

  task *execute() {
    int block = (end - start)/NUM_TASKS;
    int sz = start + block*pid;
    int ez = sz + block;

    if(pid==(NUM_TASKS-1))
       ez = end;

    for(int iz = sz; iz < ez; ++iz)
      for(int iy = grids[tid].sy; iy < grids[tid].ey; ++iy)
        for(int ix = grids[tid].sx; ix < grids[tid].ex; ++ix)
        {
          int index = (iz*ny + iy)*nx + ix;
          cnumPars[index] = 0;
		  cells[index].next = NULL;
          last_cells[index] = &cells[index];
        }

    return NULL;
  }
};


////////////////////////////////////////////////////////////////////////////////


class RebuildGridMTWorker: public tbb::task {
  int tid;
  int pid;
  int start;
  int end;
public:
  RebuildGridMTWorker(int id, int tid_, int s, int e):
    pid(id), tid(tid_), start(s), end(e) {}

  task *execute() {
    int block = (end - start)/NUM_TASKS;
    int sz = start + block*pid;
    int ez = sz + block;

    if(pid==(NUM_TASKS-1))
       ez = end;

    //iterate through source cell lists
    for(int iz = sz; iz < ez; ++iz)
	{
      for(int iy = grids[tid].sy; iy < grids[tid].ey; ++iy)
	  {
        for(int ix = grids[tid].sx; ix < grids[tid].ex; ++ix)
        {
          int index2 = (iz*ny + iy)*nx + ix;
          Cell *cell2 = &cells2[index2];
          int np2 = cnumPars2[index2];
          //iterate through source particles
          for(int j = 0; j < np2; ++j)
          {
            //get destination for source particle
            int ci = (int)((cell2->p[j % PARTICLES_PER_CELL].x - domainMin.x) / delta.x);
            int cj = (int)((cell2->p[j % PARTICLES_PER_CELL].y - domainMin.y) / delta.y);
            int ck = (int)((cell2->p[j % PARTICLES_PER_CELL].z - domainMin.z) / delta.z);

            if(ci < 0) ci = 0; else if(ci > (nx-1)) ci = nx-1;
            if(cj < 0) cj = 0; else if(cj > (ny-1)) cj = ny-1;
            if(ck < 0) ck = 0; else if(ck > (nz-1)) ck = nz-1;
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
                    if(index == index2)
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

            Cell *cell;
            int np;
            int index = (ck*ny + cj)*nx + ci;
            { //NOTE: Braces are important for scoped lock
              // this assumes that particles cannot travel more than one grid cell per time step
              if(border[index])
                tbbMutex::scoped_lock lock(mutex[index][CELL_MUTEX_ID]);
              cell = last_cells[index];
              np = cnumPars[index];

              //add another cell structure if everything full
              if( (np % PARTICLES_PER_CELL == 0) && (cnumPars[index] != 0) ) {
                cell->next = cellpool_getcell(&pools[tid*NUM_TASKS+pid]);
                cell = cell->next;
                last_cells[index] = cell;
              }
              ++cnumPars[index];
            } //NOTE: Scoped lock is automatically released right after brace

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
              if(temp != &cells2[index2]) {
                //NOTE: This is thread-safe because temp and pool are thread-private, no need to synchronize
                cellpool_returncell(&pools[tid*NUM_TASKS+pid], temp);
              }
            }
          } // for(int j = 0; j < np2; ++j)
          //return cells to pool that are not statically allocated head of lists
          if((cell2 != NULL) && (cell2 != &cells2[index2]))
		  {
            cellpool_returncell(&pools[tid], cell2);
          }
		} // for(int ix = grids[tid].sx; ix < grids[tid].ex; ++ix)
	  } // for(int iy = grids[tid].sy; iy < grids[tid].ey; ++iy)
	} // for(int iz = sz; iz < ez; ++iz)
    return NULL;
  }
};

////////////////////////////////////////////////////////////////////////////////

int InitNeighCellList(int ci, int cj, int ck, int *neighCells)
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

class InitDensitiesAndForcesMTWorker: public tbb::task {
  int tid;
  int pid;
  int start;
  int end;
public:
  InitDensitiesAndForcesMTWorker(int id, int tid_, int s, int e):
    pid(id), tid(tid_), start(s), end(e){}

  task *execute() {
    int block = (end - start)/NUM_TASKS;
    int sz = start + block*pid;
    int ez = sz + block;

    if(pid==(NUM_TASKS-1))
       ez = end;

    for(int iz = sz; iz < ez; ++iz)
      for(int iy = grids[tid].sy; iy < grids[tid].ey; ++iy)
        for(int ix = grids[tid].sx; ix < grids[tid].ex; ++ix)
        {
          int index = (iz*ny + iy)*nx + ix;
          Cell *cell = &cells[index];
          int np = cnumPars[index];
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
    return NULL;
  }
};


////////////////////////////////////////////////////////////////////////////////

class ComputeDensitiesMTWorker: public tbb::task {
  int tid;
  int pid;
  int start;
  int end;
public:
  ComputeDensitiesMTWorker(int id, int tid_, int s, int e):
    pid(id), tid(tid_), start(s), end(e) {}

  task *execute() {
    int neighCells[3*3*3];
    int block = (end - start)/NUM_TASKS;
    int sz = start + block*pid;
    int ez = sz + block;

    if(pid==(NUM_TASKS-1))
       ez = end;

    for(int iz = sz; iz < ez; ++iz)
      for(int iy = grids[tid].sy; iy < grids[tid].ey; ++iy)
        for(int ix = grids[tid].sx; ix < grids[tid].ex; ++ix)
        {
          int index = (iz*ny + iy)*nx + ix;
          int np = cnumPars[index];
          if(np == 0)
            continue;

          int numNeighCells = InitNeighCellList(ix, iy, iz, neighCells);

          Cell *cell = &cells[index];

          for(int ipar = 0; ipar < np; ++ipar)
          {
            for(int inc = 0; inc < numNeighCells; ++inc)
            {
              int indexNeigh = neighCells[inc];
              Cell *neigh = &cells[indexNeigh];
              int numNeighPars = cnumPars[indexNeigh];
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

                    if(border[index])
                    {
                      tbbMutex::scoped_lock lock(mutex[index][ipar % MUTEXES_PER_CELL]);
                      cell->density[ipar % PARTICLES_PER_CELL] += tc;
                    }
                    else
                      cell->density[ipar % PARTICLES_PER_CELL] += tc;

                    if(border[indexNeigh])
                    {
                      tbbMutex::scoped_lock lock(mutex[indexNeigh][iparNeigh % MUTEXES_PER_CELL]);
                      neigh->density[iparNeigh % PARTICLES_PER_CELL] += tc;
                    }
                    else
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
    return NULL;
  }
};

////////////////////////////////////////////////////////////////////////////////

class ComputeDensities2MTWorker: public tbb::task {
  int tid;
  int pid;
  int start;
  int end;
public:
  ComputeDensities2MTWorker(int id, int tid_, int s, int e):
    pid(id), tid(tid_), start(s), end(e) {}

  task *execute() {
    const fptype tc = hSq*hSq*hSq;
    int block = (end - start)/NUM_TASKS;
    int sz = start + block*pid;
    int ez = sz + block;

    if(pid==(NUM_TASKS-1))
       ez = end;

    for(int iz = sz; iz < ez; ++iz)
      for(int iy = grids[tid].sy; iy < grids[tid].ey; ++iy)
        for(int ix = grids[tid].sx; ix < grids[tid].ex; ++ix)
        {
          int index = (iz*ny + iy)*nx + ix;
          Cell *cell = &cells[index];
          int np = cnumPars[index];
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
    return NULL;
  }
};


////////////////////////////////////////////////////////////////////////////////

class ComputeForcesMTWorker: public tbb::task {
  int tid;
  int pid;
  int start;
  int end;
public:
  ComputeForcesMTWorker(int id, int tid_, int s, int e):
    pid(id), tid(tid_), start(s), end(e){}

  task *execute() {
    int neighCells[3*3*3];
    int block = (end - start)/NUM_TASKS;
    int sz = start + block*pid;
    int ez = sz + block;

    if(pid==(NUM_TASKS-1))
       ez = end;

    for(int iz = sz; iz < ez; ++iz)
      for(int iy = grids[tid].sy; iy < grids[tid].ey; ++iy)
        for(int ix = grids[tid].sx; ix < grids[tid].ex; ++ix)
        {
          int index = (iz*ny + iy)*nx + ix;
          int np = cnumPars[index];
          if(np == 0)
            continue;

          int numNeighCells = InitNeighCellList(ix, iy, iz, neighCells);

          Cell *cell = &cells[index];
          for(int ipar = 0; ipar < np; ++ipar)
          {
            for(int inc = 0; inc < numNeighCells; ++inc)
            {
              int indexNeigh = neighCells[inc];
              Cell *neigh = &cells[indexNeigh];
              int numNeighPars = cnumPars[indexNeigh];
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

                    if(border[index])
                    {
                      tbbMutex::scoped_lock lock(mutex[index][ipar % MUTEXES_PER_CELL]);
                      cell->a[ipar % PARTICLES_PER_CELL] += acc;
                    }
                    else
                      cell->a[ipar % PARTICLES_PER_CELL] += acc;

                    if(border[indexNeigh])
                    {
                      tbbMutex::scoped_lock lock(mutex[indexNeigh][iparNeigh % MUTEXES_PER_CELL]);
                      neigh->a[iparNeigh % PARTICLES_PER_CELL] -= acc;
                    }
                    else
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
    return NULL;
  }
};


////////////////////////////////////////////////////////////////////////////////


class ProcessCollisionsMTWorker: public tbb::task {
  int tid;
  int pid;
  int start;
  int end;
public:
  ProcessCollisionsMTWorker(int id, int tid_, int s, int e):
    pid(id), tid(tid_), start(s), end(e){}

  task *execute() {
    int block = (end - start)/NUM_TASKS;
    int sz = start + block*pid;
    int ez = sz + block;

    if(pid==(NUM_TASKS-1))
       ez = end;
// ProcessCollisions() with container walls
// Under the assumptions that
// a) a particle will not penetrate a wall
// b) a particle will not migrate further than once cell
// c) the parSize is smaller than a cell
// then only the particles at the perimiters may be influenced by the walls
#if 0
    for(int iz = sz; iz < ez; ++iz)
	{
      for(int iy = grids[tid].sy; iy < grids[tid].ey; ++iy)
	  {
        for(int ix = grids[tid].sx; ix < grids[tid].ex; ++ix)
        {
          int index = (iz*ny + iy)*nx + ix;
          Cell *cell = &cells[index];
          int np = cnumPars[index];
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
            if(j % PARTICLES_PER_CELL == PARTICLES_PER_CELL-1) {
              cell = cell->next;
            }
          }
        }
	  }
	}
#else
  for(int iz = grids[tid].sz; iz < grids[tid].ez; ++iz)
  {
    for(int iy = grids[tid].sy; iy < grids[tid].ey; ++iy)
	{
      for(int ix = grids[tid].sx; ix < grids[tid].ex; ++ix)
      {
	    if(!((ix==0)||(iy==0)||(iz==0)||(ix==(nx-1))||(iy==(ny-1))==(iz==(nz-1))))
			continue;	// not on domain wall
        int index = (iz*ny + iy)*nx + ix;
        Cell *cell = &cells[index];
        int np = cnumPars[index];
        for(int j = 0; j < np; ++j)
        {
		  int ji = j % PARTICLES_PER_CELL;
          Vec3 pos = cell->p[ji] + cell->hv[ji] * timeStep;

		  if(ix==0)
		  {
            fptype diff = parSize - (pos.x - domainMin.x);
		    if(diff > epsilon)
              cell->a[ji].x += stiffnessCollisions*diff - damping*cell->v[ji].x;
		  }
		  if(ix==(nx-1))
		  {
            fptype diff = parSize - (domainMax.x - pos.x);
            if(diff > epsilon)
              cell->a[ji].x -= stiffnessCollisions*diff + damping*cell->v[ji].x;
		  }
		  if(iy==0)
		  {
            fptype diff = parSize - (pos.y - domainMin.y);
            if(diff > epsilon)
              cell->a[ji].y += stiffnessCollisions*diff - damping*cell->v[ji].y;
		  }
		  if(iy==(ny-1))
		  {
            fptype diff = parSize - (domainMax.y - pos.y);
            if(diff > epsilon)
              cell->a[ji].y -= stiffnessCollisions*diff + damping*cell->v[ji].y;
		  }
		  if(iz==0)
		  {
            fptype diff = parSize - (pos.z - domainMin.z);
            if(diff > epsilon)
              cell->a[ji].z += stiffnessCollisions*diff - damping*cell->v[ji].z;
		  }
		  if(iz==(nz-1))
		  {
            fptype diff = parSize - (domainMax.z - pos.z);
            if(diff > epsilon)
              cell->a[ji].z -= stiffnessCollisions*diff + damping*cell->v[ji].z;
		  }
          //move pointer to next cell in list if end of array is reached
          if(ji == PARTICLES_PER_CELL-1) {
            cell = cell->next;
          }
        }
      }
	}
  }
#endif
    return NULL;
  }
};

#define USE_ImpeneratableWall
#if defined(USE_ImpeneratableWall)
class ProcessCollisions2MTWorker: public tbb::task {
  int tid;
  int pid;
  int start;
  int end;
public:
  ProcessCollisions2MTWorker(int id, int tid_, int s, int e):
    pid(id), tid(tid_), start(s), end(e){}

  task *execute() {
    int block = (end - start)/NUM_TASKS;
    int sz = start + block*pid;
    int ez = sz + block;

    if(pid==(NUM_TASKS-1))
       ez = end;
  for(int iz = grids[tid].sz; iz < grids[tid].ez; ++iz)
  {
    for(int iy = grids[tid].sy; iy < grids[tid].ey; ++iy)
	{
      for(int ix = grids[tid].sx; ix < grids[tid].ex; ++ix)
      {
#if 0
// Chris, the following test should be valid
// *** provided that a particle does not migrate more than 1 cell
// *** per integration step. This does not appear to be the case
// *** in the pthreads version. Serial version it seems to be OK
	    if(!((ix==0)||(iy==0)||(iz==0)||(ix==(nx-1))||(iy==(ny-1))==(iz==(nz-1))))
			continue;	// not on domain wall
#endif
        int index = (iz*ny + iy)*nx + ix;
        Cell *cell = &cells[index];
        int np = cnumPars[index];
        for(int j = 0; j < np; ++j)
        {
		  int ji = j % PARTICLES_PER_CELL;
          Vec3 pos = cell->p[ji];

		  if(ix==0)
		  {
            fptype diff = pos.x - domainMin.x;
		    if(diff < Zero)
			{
				cell->p[ji].x = domainMin.x - diff;
				cell->v[ji].x = -cell->v[ji].x;
				cell->hv[ji].x = -cell->hv[ji].x;
			}
		  }
		  if(ix==(nx-1))
		  {
            fptype diff = domainMax.x - pos.x;
 			if(diff < Zero)
			{
				cell->p[ji].x = domainMax.x + diff;
				cell->v[ji].x = -cell->v[ji].x;
				cell->hv[ji].x = -cell->hv[ji].x;
			}
		  }
		  if(iy==0)
		  {
            fptype diff = pos.y - domainMin.y;
		    if(diff < Zero)
			{
				cell->p[ji].y = domainMin.y - diff;
				cell->v[ji].y = -cell->v[ji].y;
				cell->hv[ji].y = -cell->hv[ji].y;
			}
		  }
		  if(iy==(ny-1))
		  {
            fptype diff = domainMax.y - pos.y;
 			if(diff < Zero)
			{
				cell->p[ji].y = domainMax.y + diff;
				cell->v[ji].y = -cell->v[ji].y;
				cell->hv[ji].y = -cell->hv[ji].y;
			}
		  }
		  if(iz==0)
		  {
            fptype diff = pos.z - domainMin.z;
		    if(diff < Zero)
			{
				cell->p[ji].z = domainMin.z - diff;
				cell->v[ji].z = -cell->v[ji].z;
				cell->hv[ji].z = -cell->hv[ji].z;
			}
		  }
		  if(iz==(nz-1))
		  {
            fptype diff = domainMax.z - pos.z;
 			if(diff < Zero)
			{
				cell->p[ji].z = domainMax.z + diff;
				cell->v[ji].z = -cell->v[ji].z;
				cell->hv[ji].z = -cell->hv[ji].z;
			}
		  }
          //move pointer to next cell in list if end of array is reached
          if(ji == PARTICLES_PER_CELL-1) {
            cell = cell->next;
          }
        }
      }
	}
  }
    return NULL;
  }
};
#endif

////////////////////////////////////////////////////////////////////////////////

class AdvanceParticlesMTWorker: public tbb::task {
  int tid;
  int pid;
  int start;
  int end;
public:
  AdvanceParticlesMTWorker(int id, int tid_, int s, int e):
    pid(id), tid(tid_), start(s), end(e) {}

  task *execute() {
    int block = (end - start)/NUM_TASKS;
    int sz = start + block*pid;
    int ez = sz + block;

    if(pid==(NUM_TASKS-1))
       ez = end;

    for(int iz = sz; iz < ez; ++iz)
      for(int iy = grids[tid].sy; iy < grids[tid].ey; ++iy)
        for(int ix = grids[tid].sx; ix < grids[tid].ex; ++ix)
        {
          int index = (iz*ny + iy)*nx + ix;
          Cell *cell = &cells[index];
          int np = cnumPars[index];
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
    return NULL;
  }
};

////////////////////////////////////////////////////////////////////////////////

void AdvanceFrame()
{
  void *status;

  //swap src and dest arrays with particles
  std::swap(cells, cells2);
  std::swap(cnumPars, cnumPars2);

  GridLauncher<ClearParticlesMTWorker> &cp = *new (tbb::task::allocate_root()) GridLauncher<ClearParticlesMTWorker>(NUM_GRIDS);
  tbb::task::spawn_root_and_wait(cp);

  GridLauncher<RebuildGridMTWorker> &rg = *new (tbb::task::allocate_root()) GridLauncher<RebuildGridMTWorker>(NUM_GRIDS);
  tbb::task::spawn_root_and_wait(rg);

  GridLauncher<InitDensitiesAndForcesMTWorker> &id = *new (tbb::task::allocate_root()) GridLauncher<InitDensitiesAndForcesMTWorker>(NUM_GRIDS);
  tbb::task::spawn_root_and_wait(id);

  GridLauncher<ComputeDensitiesMTWorker> &cd = *new (tbb::task::allocate_root()) GridLauncher<ComputeDensitiesMTWorker>(NUM_GRIDS);
  tbb::task::spawn_root_and_wait(cd);

  GridLauncher<ComputeDensities2MTWorker> &cd2 = *new (tbb::task::allocate_root()) GridLauncher<ComputeDensities2MTWorker>(NUM_GRIDS);
  tbb::task::spawn_root_and_wait(cd2);

  GridLauncher<ComputeForcesMTWorker> &cf = *new (tbb::task::allocate_root()) GridLauncher<ComputeForcesMTWorker>(NUM_GRIDS);
  tbb::task::spawn_root_and_wait(cf);

  GridLauncher<ProcessCollisionsMTWorker> &pc = *new (tbb::task::allocate_root()) GridLauncher<ProcessCollisionsMTWorker>(NUM_GRIDS);
  tbb::task::spawn_root_and_wait(pc);

  GridLauncher<AdvanceParticlesMTWorker> &ap = *new (tbb::task::allocate_root()) GridLauncher<AdvanceParticlesMTWorker>(NUM_GRIDS);
  tbb::task::spawn_root_and_wait(ap);

#if defined(USE_ImpeneratableWall)
  // N.B. The integration of the position can place the particle
  // outside the domain. We now make a pass on the perimiter cells
  // to account for particle migration beyond domain.
  GridLauncher<ProcessCollisions2MTWorker> &pc2 = *new (tbb::task::allocate_root()) GridLauncher<ProcessCollisions2MTWorker>(NUM_GRIDS);
  tbb::task::spawn_root_and_wait(pc2);
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
  if(threadnum < 1) {
    std::cerr << "<threadnum> must at least be 1" << std::endl;
    return -1;
  }
  if(framenum < 1) {
    std::cerr << "<framenum> must at least be 1" << std::endl;
    return -1;
  }

#ifdef ENABLE_CFL_CHECK
  std::cout << "WARNING: Check for Courant–Friedrichs–Lewy condition enabled. Do not use for performance measurements." << std::endl;
#endif

  InitSim(argv[3], threadnum);
  tbb::task_scheduler_init init(threadnum);
#ifdef ENABLE_VISUALIZATION
  InitVisualizationMode(&argc, argv, &AdvanceFrame, &numCells, &cells, &cnumPars);
#endif

#ifndef ENABLE_VISUALIZATION

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
