// Written by Christian Bienia
// This file aggregates definitions used across all versions of the program

#ifndef __FLUID_HPP__
#define __FLUID_HPP__ 1

#include <stddef.h>
#if defined(WIN32)
typedef __int64 int64_t;
typedef __int32 int32_t;
typedef __int16 int16_t;
typedef __int8 int8_t;
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;
#else
#include <stdint.h>
#endif
#include <math.h>



//Enable to use double precision in program for everything except disk I/O, otherwise single precision is used
//#define ENABLE_DOUBLE_PRECISION

//Our estimate for a cache line size on this machine
#define CACHELINE_SIZE 128

//Maximum number of particles in a physical cell
#define PARTICLES_PER_CELL 16



#ifndef ENABLE_DOUBLE_PRECISION
typedef float fptype;
#else
typedef double fptype;
#endif


static inline int isLittleEndian() {
  union {
    uint16_t word;
    uint8_t byte;
  } endian_test;

  endian_test.word = 0x00FF;
  return (endian_test.byte == 0xFF);
}

//NOTE: Use float variables even for double precision version b/c file format uses float
union __float_and_int {
  uint32_t i;
  float    f;
};

static inline float bswap_float(float x) {
  union __float_and_int __x;

   __x.f = x;
   __x.i = ((__x.i & 0xff000000) >> 24) | ((__x.i & 0x00ff0000) >>  8) |
           ((__x.i & 0x0000ff00) <<  8) | ((__x.i & 0x000000ff) << 24);

  return __x.f;
}

static inline int bswap_int32(int x) {
  return ( (((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |
           (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24) );
}


//#define VEC3_VECTORIZED

#if defined (__ICC) && defined (VEC3_VECTORIZED)

#include "smmintrin.h"

#ifndef ENABLE_DOUBLE_PRECISION

class Vec3 // single precision 
{
private:
  Vec3(fptype val) { data = _mm_set_ps1(val); padd = 0.f; }
public:    
  union  {
    struct { fptype x, y, z, padd; };
    __m128 data __attribute__ (( aligned(16)));
  };

  Vec3() { data = _mm_set_ps1(0.f); }

  Vec3(fptype _x, fptype _y, fptype _z) : x(_x), y(_y), z(_z), padd(0.f) {}
  Vec3( const Vec3& oth) { data = oth.data; }

  //fptype  GetLengthSq() const         { return x*x + y*y + z*z; }
  fptype  GetLengthSq() const  {
      fptype tmp0 = _mm_cvtss_f32( _mm_dp_ps(data, data, 0x77) ); 
      return tmp0; 
  }

  fptype  GetLength() const           { return sqrtf(GetLengthSq()); }
  Vec3 &  Normalize()                 { return *this /= GetLength(); }

  bool    operator == (Vec3 const &v) { return (x == v.x) && (y == v.y) && (z += v.z); }
  
  Vec3 &  operator = (Vec3 const &v) { 
     data = v.data;
     return *this;
  }

//Vec3 &  operator += (Vec3 const &v) { x += v.x;  y += v.y; z += v.z; return *this; }
  Vec3 &  operator += (Vec3 const &v) { 
     data = _mm_add_ps(data, v.data);
     return *this;
  }
   
  Vec3 &  operator = (fptype val) { 
     data = _mm_set_ps1(val); padd = 0.f; 
     return *this;
  }
 
  //Vec3 &  operator -= (Vec3 const &v) { x -= v.x;  y -= v.y; z -= v.z; return *this; }
  Vec3 &  operator -= (Vec3 const &v) { 
     data = _mm_sub_ps(data, v.data);
     return *this;
  }

  //Vec3 &  operator *= (fptype s)      { x *= s;  y *= s; z *= s; return *this; }
  Vec3 &  operator *= (fptype s)      {
     Vec3 tmp(s);
     data = _mm_mul_ps(data, tmp.data);
     return *this; 
  } 

  //Vec3 &  operator /= (fptype s)   { x /= s;  y /= s; z /= s; return *this; }
  Vec3 &  operator /= (fptype s)   {
     fptype tmp = 1.f/s;
     (*this) *= tmp;   
     return *this;
  }

  //Vec3    operator + (Vec3 const &v) const    { return Vec3(x+v.x, y+v.y, z+v.z); }
  Vec3    operator + (Vec3 const &v) const    {
     Vec3 res;
     res.data = _mm_add_ps(data, v.data);
     return res;
  }

  //Vec3    operator + (fptype const &f) const  { return Vec3(x+f, y+f, z+f); }
  Vec3    operator + (fptype const &f) const {
     Vec3 tmp(f);
     tmp.data = _mm_add_ps(data, tmp.data);
     return tmp; 
  }

  //Vec3    operator - () const                 { return Vec3(-x, -y, -z); }
  Vec3    operator - () const     { 
     Vec3 tmp;
     tmp.data = _mm_sub_ps(tmp.data, data);
     return tmp; 
  }

 
  //Vec3    operator - (Vec3 const &v) const    { return Vec3(x-v.x, y-v.y, z-v.z); }
  Vec3    operator - (Vec3 const &v) const    { 
     Vec3 res;
     res.data = _mm_sub_ps(data, v.data);
     return res;
  }


  //Vec3    operator * (fptype s) const         { return Vec3(x*s, y*s, z*s); }
  Vec3    operator * (fptype s) const     { 
     Vec3 tmp(s);
     tmp.data = _mm_mul_ps(data, tmp.data);
     return tmp; 
  } 
 
  //Vec3    operator / (fptype s) const         { return Vec3(x/s, y/s, z/s); }
  Vec3    operator / (fptype s) const   { 
     fptype tmp = 1.f/s;
     Vec3 tmpVec = (*this) * tmp;   
     return tmpVec;
  }


  //fptype  operator * (Vec3 const &v) const    { return x*v.x + y*v.y + z*v.z; }
  fptype  operator * (Vec3 const &v) const    { 
      fptype tmp0 = _mm_cvtss_f32( _mm_dp_ps(data, v.data, 0x77) ); 
      return tmp0; 
  }

} ;

#else   // double precision  

class Vec3
{
private:
  Vec3(fptype val) { data[0] = _mm_set1_pd(val); z = val; padd = 0.f; }
public:    
  union  {
    struct { fptype x, y, z, padd; };
    __m128d data[2] __attribute__ (( aligned(16)));
  };

  Vec3() { data[0] = _mm_set1_pd(0.f); data[1] = _mm_set1_pd(0.f); }

  Vec3(fptype _x, fptype _y, fptype _z) : x(_x), y(_y), z(_z), padd(0.f) {}
  Vec3( const Vec3& oth) { memcpy(data, oth.data, 2*sizeof(__m128d) ); }
  //Vec3( const Vec3& oth) { data[0] = oth.data[0]; data[1] = oth.data[1]; }

  //fptype  GetLengthSq() const         { return x*x + y*y + z*z; }
  fptype  GetLengthSq() const  {
      fptype tmp0 = _mm_cvtsd_f64( _mm_dp_pd(data[0], data[0], 0x33) ); 
      //fptype tmp1 = _mm_cvtsd_f64( _mm_dp_pd(data[1], data[1], 0x33) ); 
      fptype tmp1 = z*z;
      return tmp0 + tmp1; 
  }

  fptype  GetLength() const           { return sqrtf(GetLengthSq()); }
  Vec3 &  Normalize()                 { return *this /= GetLength(); }

  bool    operator == (Vec3 const &v) { return (x == v.x) && (y == v.y) && (z += v.z); }
  
  Vec3 &  operator = (Vec3 const &v) { 
     data[0] = v.data[0];
     data[1] = v.data[1];
     return *this;
  }

//Vec3 &  operator += (Vec3 const &v) { x += v.x;  y += v.y; z += v.z; return *this; }
  Vec3 &  operator += (Vec3 const &v) { 
     data[0] = _mm_add_pd(data[0], v.data[0]);
     data[1] = _mm_add_pd(data[1], v.data[1]);
     return *this;
  }
   
  Vec3 &  operator = (fptype val) { 
     data[0] = _mm_set1_pd(val); z = val; padd = 0.f; 
     return *this;
  }
 
  //Vec3 &  operator -= (Vec3 const &v) { x -= v.x;  y -= v.y; z -= v.z; return *this; }
  Vec3 &  operator -= (Vec3 const &v) { 
     data[0] = _mm_sub_pd(data[0], v.data[0]);
     data[1] = _mm_sub_pd(data[1], v.data[1]);
     return *this;
  }

  //Vec3 &  operator *= (fptype s)      { x *= s;  y *= s; z *= s; return *this; }
  Vec3 &  operator *= (fptype s)      {
     Vec3 tmp(s);
     data[0] = _mm_mul_pd(data[0], tmp.data[0]);
     data[1] = _mm_mul_pd(data[1], tmp.data[1]);
     return *this; 
  } 

  //Vec3 &  operator /= (fptype s)   { x /= s;  y /= s; z /= s; return *this; }
  Vec3 &  operator /= (fptype s)   {
     fptype tmp = 1.f/s;
     (*this) *= tmp;   
     return *this;
  }

  //Vec3    operator + (Vec3 const &v) const    { return Vec3(x+v.x, y+v.y, z+v.z); }
  Vec3    operator + (Vec3 const &v) const    {
     Vec3 res;
     res.data[0] = _mm_add_pd(data[0], v.data[0]);
     res.data[1] = _mm_add_pd(data[1], v.data[1]);
     return res;
  }

  //Vec3    operator + (fptype const &f) const  { return Vec3(x+f, y+f, z+f); }
  Vec3    operator + (fptype const &f) const {
     Vec3 tmp(f);
     tmp.data[0] = _mm_add_pd(data[0], tmp.data[0]);
     tmp.data[1] = _mm_add_pd(data[1], tmp.data[1]);
     return tmp; 
  }

  //Vec3    operator - () const                 { return Vec3(-x, -y, -z); }
  Vec3    operator - () const     { 
     Vec3 tmp;
     tmp.data[0] = _mm_sub_pd(tmp.data[0], data[0]);
     tmp.data[1] = _mm_sub_pd(tmp.data[1], data[1]);
     return tmp; 
  }

 
  //Vec3    operator - (Vec3 const &v) const    { return Vec3(x-v.x, y-v.y, z-v.z); }
  Vec3    operator - (Vec3 const &v) const    { 
     Vec3 res;
     res.data[0] = _mm_sub_pd(data[0], v.data[0]);
     res.data[1] = _mm_sub_pd(data[1], v.data[1]);
     return res;
  }


  //Vec3    operator * (fptype s) const         { return Vec3(x*s, y*s, z*s); }
  Vec3    operator * (fptype s) const     { 
     Vec3 tmp(s);
     tmp.data[0] = _mm_mul_pd(data[0], tmp.data[0]);
     tmp.data[1] = _mm_mul_pd(data[1], tmp.data[1]);
     return tmp; 
  } 
 
  //Vec3    operator / (fptype s) const         { return Vec3(x/s, y/s, z/s); }
  Vec3    operator / (fptype s) const   { 
     fptype tmp = 1.f/s;
     Vec3 tmpVec = (*this) * tmp;   
     return tmpVec;
  }


  //fptype  operator * (Vec3 const &v) const    { return x*v.x + y*v.y + z*v.z; }
  fptype  operator * (Vec3 const &v) const    { 
      fptype tmp0 = _mm_cvtsd_f64( _mm_dp_pd(data[0], v.data[0], 0x33) ); 
      fptype tmp1 = _mm_cvtsd_f64( _mm_dp_pd(data[1], v.data[1], 0x33) ); 
      return tmp0 + tmp1; 
  }
} ;

#endif


#else

// NOTE: icc-optimized version of this class gave 15% more
// performance than our hand-optimized SSE3 implementation
class Vec3
{
public:
  fptype x, y, z;

  Vec3() {}
  Vec3(fptype _x, fptype _y, fptype _z) : x(_x), y(_y), z(_z) {}

  fptype  GetLengthSq() const         { return x*x + y*y + z*z; }
  fptype  GetLength() const           { return sqrtf(GetLengthSq()); }
  Vec3 &  Normalize()                 { return *this /= GetLength(); }

  bool    operator == (Vec3 const &v) { return (x == v.x) && (y == v.y) && (z += v.z); }
  Vec3 &  operator += (Vec3 const &v) { x += v.x;  y += v.y; z += v.z; return *this; }
  Vec3 &  operator -= (Vec3 const &v) { x -= v.x;  y -= v.y; z -= v.z; return *this; }
  Vec3 &  operator *= (fptype s)      { x *= s;  y *= s; z *= s; return *this; }
  Vec3 &  operator /= (fptype s)      { fptype tmp = 1.f/s; x *= tmp;  y *= tmp; z *= tmp; return *this; }

  Vec3    operator + (Vec3 const &v) const    { return Vec3(x+v.x, y+v.y, z+v.z); }
  Vec3    operator + (fptype const &f) const  { return Vec3(x+f, y+f, z+f); }
  Vec3    operator - () const                 { return Vec3(-x, -y, -z); }
  Vec3    operator - (Vec3 const &v) const    { return Vec3(x-v.x, y-v.y, z-v.z); }
  Vec3    operator * (fptype s) const         { return Vec3(x*s, y*s, z*s); }
  Vec3    operator / (fptype s) const         { fptype tmp = 1.f/s; return Vec3(x*tmp, y*tmp, z*tmp); }

  fptype  operator * (Vec3 const &v) const    { return x*v.x + y*v.y + z*v.z; }
};

#endif 



////////////////////////////////////////////////////////////////////////////////

// We define two Cell structures - one helper structure without padding and one
// "real" structure with padding to be used by the program. The helper structure
// is needed because compilers can insert different amounts of auto-generated
// padding and we need to know the exact amount to calculate the cache line
// padding accurately. By having two structures we can reference that amount
// for the padding calculations. Both structures must have the same amount
// of payload data, which we check with an assert in the program. Make
// sure to keep both structures in sync.

// NOTE: Please note the difference between a logical cell and a physical
// cell. A logical cell corresponds to a 3D region in space and contains all
// the particles in that region. A physical cell is the Cell structure defined
// below. Each logical cell is implemented of a linked list of physical cells.

//Actual particle data stored in the cells
#define CELL_CONTENTS \
  Vec3 p[PARTICLES_PER_CELL]; \
  Vec3 hv[PARTICLES_PER_CELL]; \
  Vec3 v[PARTICLES_PER_CELL]; \
  Vec3 a[PARTICLES_PER_CELL]; \
  fptype density[PARTICLES_PER_CELL];

//Helper structure for padding calculation, not used directly by the program
struct Cell_aux {
  CELL_CONTENTS
  Cell_aux *next;
  //dummy variable so we can reference the end of the payload data
  char padding;
};

//Real Cell structure
struct Cell {
  CELL_CONTENTS
  Cell *next;
  //padding to force cell size to a multiple of estimated cache line size
#pragma warning( disable : 1684) // warning #1684: conversion from pointer to same-sized integral type (potential portability problem)
  char padding[CACHELINE_SIZE - (offsetof(struct Cell_aux, padding) % CACHELINE_SIZE)];
  Cell() { next = NULL; }
};

////////////////////////////////////////////////////////////////////////////////

static const fptype pi = 3.14159265358979;

static const fptype parSize = 0.0002;
static const fptype epsilon = 1e-10;
static const fptype stiffnessPressure = 3.0;
static const fptype stiffnessCollisions = 30000.0;
static const fptype damping = 128.0;
static const fptype viscosity = 0.4;

static fptype timeStep = 0.001;
static const fptype doubleRestDensity = 2000.0;
static const fptype kernelRadiusMultiplier = 1.695;
static const Vec3 externalAcceleration(0.0, -9.8, 0.0);
static const Vec3 domainMin(-0.065, -0.08, -0.065);
static const Vec3 domainMax(0.065, 0.1, 0.065);
static const fptype Zero = 0.0;
//Constants for file I/O
#define FILE_SIZE_INT 4
#define FILE_SIZE_FLOAT 4

#endif //__FLUID_HPP__
