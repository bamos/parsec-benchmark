/* Compare two fluid files for mismatches
 * Written by Christian Bienia for the PARSEC Benchmark Suite
 * Use this program to verify correct execution of fluidanimate
 */

#include <cstdlib>
#include <iostream>
#include <fstream>

#include <string.h>
#include <math.h>
#include <assert.h>

#include "fluid.hpp"


////////////////////////////////////////////////////////////////////////////////

//Which error codes to return in the various cases
#define ERROR_OK 0
#define ERROR_FAIL 1
#define ERROR_OTHER -1

//Maximum number of mismatches to print for each test
int DEFAULT_MAX_MISMATCH_PRINT = 10;

//Configuration as derived from command line arguments
typedef struct {
  char *file;
  char *rfile;
  //details concerning output
  struct {
    bool verbose;
    int max;
  } output;
  //Position test
  struct {
    bool doTest;
    float tol;
  } ptest;
  //Velocity test
  struct {
    bool doTest;
    float tol;
  } vtest;
  //Bounding box test
  struct {
    bool doTest;
    float tol;
  } bbox;
} conf_t;


//A fluid as read in from a fluid file
typedef struct {
  float restParticlesPerMeter;
  int numParticles;
  Vec3 *p;
  Vec3 *hv;
  Vec3 *v;
  //Axis-aligned bounding box containing all particles
  struct {
    Vec3 min;
    Vec3 max;
  } bbox;
} fluid_t;

////////////////////////////////////////////////////////////////////////////////

//Allocates the arrays required to store a fluid of size `s'
void malloc_fluid(fluid_t *f, size_t size) {
  assert(f!=NULL);
  f->p = (Vec3 *)malloc(sizeof(Vec3) * size);
  f->hv = (Vec3 *)malloc(sizeof(Vec3) * size);
  f->v = (Vec3 *)malloc(sizeof(Vec3) * size);
  if(f->p == NULL || f->hv == NULL || f->v == NULL ) {
    printf("Error allocating memory for fluid\n");
    exit(1);
  }
}

//Free all dynamically allocated storage for a fluid
void free_fluid(fluid_t *f) {
  assert(f!=NULL);
  free(f->p);
  free(f->hv);
  free(f->v);
}

////////////////////////////////////////////////////////////////////////////////

void ReadFile(char const *fileName, fluid_t *f)
{
  assert(fileName);
  assert(f);

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
    f->restParticlesPerMeter = bswap_float(restParticlesPerMeter_le);
    f->numParticles          = bswap_int32(numParticles_le);
  } else {
    f->restParticlesPerMeter = restParticlesPerMeter_le;
    f->numParticles          = numParticles_le;
  }
#if defined(SPARC_SOLARIS)
  f->bbox.min.x = MAXFLOAT;
  f->bbox.min.y = MAXFLOAT;
  f->bbox.min.z = MAXFLOAT;
  f->bbox.max.x = -MAXFLOAT;
  f->bbox.max.y = -MAXFLOAT;
  f->bbox.max.z = -MAXFLOAT;
#else
  f->bbox.min.x = INFINITY;
  f->bbox.min.y = INFINITY;
  f->bbox.min.z = INFINITY;
  f->bbox.max.x = -INFINITY;
  f->bbox.max.y = -INFINITY;
  f->bbox.max.z = -INFINITY;
#endif
  malloc_fluid(f, f->numParticles);

  //Always use single precision float variables b/c file format uses single precision
  float px, py, pz, hvx, hvy, hvz, vx, vy, vz;
  for(int i = 0; i < f->numParticles; ++i)
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

    //add particle to fluid structure
    f->p[i].x = px;
    f->p[i].y = py;
    f->p[i].z = pz;
    f->hv[i].x = hvx;
    f->hv[i].y = hvy;
    f->hv[i].z = hvz;
    f->v[i].x = vx;
    f->v[i].y = vy;
    f->v[i].z = vz;

    //update bounding box
    if(px < f->bbox.min.x) f->bbox.min.x = px;
    if(py < f->bbox.min.y) f->bbox.min.y = py;
    if(pz < f->bbox.min.z) f->bbox.min.z = pz;
    if(px > f->bbox.max.x) f->bbox.max.x = px;
    if(py > f->bbox.max.y) f->bbox.max.y = py;
    if(pz > f->bbox.max.z) f->bbox.max.z = pz;
  }
}

////////////////////////////////////////////////////////////////////////////////

// Test functions
// All functions are independent from each other and return true if the test is passed, false otherwise.

// Verify positions
bool verify_ptest(fluid_t *fluid, fluid_t *rfluid, conf_t *conf) {
  bool result = true;
  bool presult;
  int ecount = 0;
  int i;

  for(i=0; i<fluid->numParticles; i++) {
    presult = (fluid->p[i].x >= rfluid->p[i].x - conf->ptest.tol) &&
              (fluid->p[i].y >= rfluid->p[i].y - conf->ptest.tol) &&
              (fluid->p[i].z >= rfluid->p[i].z - conf->ptest.tol) &&
              (fluid->p[i].x <= rfluid->p[i].x + conf->ptest.tol) &&
              (fluid->p[i].y <= rfluid->p[i].y + conf->ptest.tol) &&
              (fluid->p[i].z <= rfluid->p[i].z + conf->ptest.tol);
    if(!presult && conf->output.verbose && (ecount < conf->output.max)) {
      std::cout << "Position mismatch: Expected <" << rfluid->p[i].x << "," << rfluid->p[i].y << "," << rfluid->p[i].z << ">" << std::endl;
      std::cout << "                   Received <" << fluid->p[i].x << "," << fluid->p[i].y << "," << fluid->p[i].z << ">" << std::endl;
    }
    if(!presult) ecount++;
    result &= presult;
  }

  return result;
}

// Verify velocities
bool verify_vtest(fluid_t *fluid, fluid_t *rfluid, conf_t *conf) {
  bool result = true;
  bool presult;
  int ecount = 0;
  int i;

  for(i=0; i<fluid->numParticles; i++) {
    presult = (fluid->v[i].x >= rfluid->v[i].x - conf->vtest.tol) &&
              (fluid->v[i].y >= rfluid->v[i].y - conf->vtest.tol) &&
              (fluid->v[i].z >= rfluid->v[i].z - conf->vtest.tol) &&
              (fluid->v[i].x <= rfluid->v[i].x + conf->vtest.tol) &&
              (fluid->v[i].y <= rfluid->v[i].y + conf->vtest.tol) &&
              (fluid->v[i].z <= rfluid->v[i].z + conf->vtest.tol);
    if(!presult && conf->output.verbose && (ecount < conf->output.max)) {
      std::cout << "Velocity mismatch: Expected <" << rfluid->v[i].x << "," << rfluid->v[i].y << "," << rfluid->v[i].z << ">" << std::endl;
      std::cout << "                   Received <" << fluid->v[i].x << "," << fluid->v[i].y << "," << fluid->v[i].z << ">" << std::endl;
    }
    if(!presult) ecount++;
    result &= presult;
  }

  return result;
}

// Verify spatial extent
bool verify_bbox(fluid_t *fluid, fluid_t *rfluid, conf_t *conf) {
  bool result;

  result = (fluid->bbox.min.x >= rfluid->bbox.min.x - conf->bbox.tol) &&
           (fluid->bbox.min.y >= rfluid->bbox.min.y - conf->bbox.tol) &&
           (fluid->bbox.min.z >= rfluid->bbox.min.z - conf->bbox.tol) &&
           (fluid->bbox.min.x <= rfluid->bbox.min.x + conf->bbox.tol) &&
           (fluid->bbox.min.y <= rfluid->bbox.min.y + conf->bbox.tol) &&
           (fluid->bbox.min.z <= rfluid->bbox.min.z + conf->bbox.tol) &&
           (fluid->bbox.max.x >= rfluid->bbox.max.x - conf->bbox.tol) &&
           (fluid->bbox.max.y >= rfluid->bbox.max.y - conf->bbox.tol) &&
           (fluid->bbox.max.z >= rfluid->bbox.max.z - conf->bbox.tol) &&
           (fluid->bbox.max.x <= rfluid->bbox.max.x + conf->bbox.tol) &&
           (fluid->bbox.max.y <= rfluid->bbox.max.y + conf->bbox.tol) &&
           (fluid->bbox.max.z <= rfluid->bbox.max.z + conf->bbox.tol);

  if(!result && conf->output.verbose) {
    std::cout << "Bounding box mismatch: Expected <" << rfluid->bbox.min.x << "," << rfluid->bbox.min.y << "," << rfluid->bbox.min.z << "> - <" << rfluid->bbox.min.x << "," << rfluid->bbox.min.y << "," << rfluid->bbox.min.z << ">" << std::endl;
    std::cout << "                       Received <" << fluid->bbox.min.x << "," << fluid->bbox.min.y << "," << fluid->bbox.min.z << "> - <" << fluid->bbox.min.x << "," << fluid->bbox.min.y << "," << fluid->bbox.min.z << ">" << std::endl;
  }

  return result;
}


////////////////////////////////////////////////////////////////////////////////

// Print usage information
void print_usage(const char *name) {
  std::cout << "Usage: " << name << " FILE RFILE [options]" << std::endl;
  std::cout << "  Compares the fluid contained in FILE to the one in reference file RFILE using the tests specified by the options." << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  --verbose     Print out details about any mismatches" << std::endl;
  std::cout << "  --maxout INT  Maximum number of mismatches to print (Default: " << DEFAULT_MAX_MISMATCH_PRINT << ")" << std::endl;
  std::cout << "  --ptol FLOAT  Compare positions with absolute tolerance FLOAT" << std::endl; 
  std::cout << "  --vtol FLOAT  Compare velocities with absolute tolerance FLOAT" << std::endl;
  std::cout << "  --bbox FLOAT  Compare bounding boxes with absolute tolerance FLOAT" << std::endl;
}

// Parse command line arguments
bool parse_args(conf_t *conf, int argc, char *argv[]) {
  int i,j;

  //Initialize configuration
  assert(conf!=NULL);
  conf->file = NULL;
  conf->rfile = NULL;
  conf->output.verbose = false;
  conf->output.max = DEFAULT_MAX_MISMATCH_PRINT;
  conf->ptest.doTest = false;
  conf->ptest.tol = 0.0;
  conf->vtest.doTest = false;
  conf->vtest.tol = 0.0;
  conf->bbox.doTest = false;
  conf->bbox.tol = 0.0;

  //need at least two input files
  if(argc < 3) return false;
  conf->file = argv[1];
  conf->rfile = argv[2];

  for(i=3; i<argc; i++) {
    if(!strcmp(argv[i],"--verbose")) {
      conf->output.verbose = true;
    } else if(!strcmp(argv[i],"--maxout")) {
      if(i+1>=argc) return false;
      conf->output.max = atoi(argv[i+1]);
      i++;
    } else if(!strcmp(argv[i],"--ptol")) {
      if(i+1>=argc) return false;
      conf->ptest.doTest = true;
      conf->ptest.tol = atof(argv[i+1]);
      i++;
    } else if(!strcmp(argv[i],"--vtol")) {
      if(i+1>=argc) return false;
      conf->vtest.doTest = true;
      conf->vtest.tol = atof(argv[i+1]);
      i++;
    } else if(!strcmp(argv[i],"--bbox")) {
      if(i+1>=argc) return false;
      conf->bbox.doTest = true;
      conf->bbox.tol = atof(argv[i+1]);
      i++;
    } else {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

//Returns one of the error codes defined above to indicate any issues (besides text output)
int main(int argc, char *argv[]) {
  conf_t conf;
  fluid_t rfluid;
  fluid_t fluid;
  struct {
    bool ptest;
    bool vtest;
    bool bbox;
  } results;

  //parse arguments
  if(!parse_args(&conf, argc, argv)) {
    std::cerr << "Error parsing arguments. " << std::endl;
    print_usage(argv[0]);
    return ERROR_OTHER;
  }

  //load fluids
  if(conf.output.verbose) std::cout << "Loading fluid \"" << conf.file << "\"..." << std::endl;
  ReadFile(conf.file, &fluid);
  if(conf.output.verbose) std::cout << "Loading reference fluid \"" << conf.rfile << "\"..." << std::endl;
  ReadFile(conf.rfile, &rfluid);

  //checking fluid compatibility
  if(fluid.restParticlesPerMeter != rfluid.restParticlesPerMeter) {
    std::cout << "Rest particles per meter values differ (" << fluid.restParticlesPerMeter << " vs. " << rfluid.restParticlesPerMeter << ")." << std::endl;
    return ERROR_FAIL;
  }
  if(fluid.numParticles != rfluid.numParticles)
  {
    std::cout << "Number of particles differ (" << fluid.numParticles << " vs. " << rfluid.numParticles << ")." << std::endl;
    return ERROR_FAIL;
  }

  //verify positions
  if(conf.ptest.doTest) {
    results.ptest = verify_ptest(&fluid, &rfluid, &conf);
  } else {
    results.ptest = true;
  }

  //verify velocities
  if(conf.vtest.doTest) {
    results.vtest = verify_vtest(&fluid, &rfluid, &conf);
  } else {
    results.vtest = true;
  }

  //verify bounding box
  if(conf.bbox.doTest) {
    results.bbox = verify_bbox(&fluid, &rfluid, &conf);
  } else {
    results.bbox = true;
  }

  free_fluid(&rfluid);
  free_fluid(&fluid);

  //print result of verification
  if(conf.ptest.doTest) {
    std::cout << "Position test:        " << (results.ptest ? "PASS" : "FAIL") << std::endl;
  }
  if(conf.vtest.doTest) {
    std::cout << "Velocity test:        " << (results.vtest ? "PASS" : "FAIL") << std::endl;
  }
  if(conf.bbox.doTest) {
    std::cout << "Bounding box test:    " << (results.bbox ? "PASS" : "FAIL") << std::endl;
  }
  return (results.ptest && results.vtest &&results.bbox) ? ERROR_OK : ERROR_FAIL;
}

////////////////////////////////////////////////////////////////////////////////

