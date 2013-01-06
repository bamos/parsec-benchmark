#ifndef RTTL_SSE_H
#define RTTL_SSE_H

#include <float.h>

// MS VS compiler specific settings (also used in emulation mode)
#if (defined(_MSC_VER) && (_MSC_VER <= 1400)) || defined(RT_EMULATE_SSE)
// These intrinsics are not defined in VS prior to 2007.
static _INLINE sse_f _mm_castsi128_ps(sse_i n) { return *(sse_f *) &n; }
static _INLINE sse_i _mm_castps_si128(sse_f  n) { return *(sse_i*) &n; }
#endif

#define M128_FLOAT(s,x) ((float*)&(s))[x]
#define M128_INT(s,x)   ((int*)&(s))[x]
#define M128_UINT(s,x)  ((unsigned int*)&(s))[x]

// Overloaded function is deduced from argument.
_INLINE sse_i cast(sse_f v)    { return _mm_castps_si128(v); }
_INLINE sse_f cast(sse_i v)    { return _mm_castsi128_ps(v); }
_INLINE sse_i convert(sse_f v) { return _mm_cvtps_epi32(v);  }
_INLINE sse_f convert(sse_i v) { return _mm_cvtepi32_ps(v);  }

// The following two definitions will cause error once compiler will start
// supporting SSE4. In this case, just remove them.
#define _mm_blendv_ps(v1, v2, f)   _mm_or_ps(   _mm_and_ps   (f, v1), _mm_andnot_ps   (f, v2))
#define _mm_blendv_epi8(v1, v2, f) _mm_or_si128(_mm_and_si128(f, v1), _mm_andnot_si128(f, v2))

// (valid for NANs)
#define _mm_all_equal(in) (_mm_movemask_ps(cast(_mm_cmpeq_epi32(cast(_mm_shuffle_ps(in, in, 0x00)), cast(in)))) == 0xf)

_INLINE sse_f rcp(const sse_f src) {
    sse_f tgt;
    tgt = _mm_rcp_ps(src);
    tgt = _mm_sub_ps(_mm_add_ps(tgt,tgt), _mm_mul_ps(_mm_mul_ps(tgt,tgt), src));
    return tgt;
}

static const sse_f maxFloat = _mm_set_ps1(FLT_MAX);

_INLINE sse_f rcp_save(const sse_f src) {
    sse_f tgt;
    tgt = _mm_rcp_ps(src);
    tgt = _mm_sub_ps(_mm_add_ps(tgt,tgt), _mm_mul_ps(_mm_mul_ps(tgt,tgt), src));
    tgt = _mm_blendv_ps(tgt,maxFloat,_mm_cmpneq_ps(src,_mm_setzero_ps()));
    return tgt;
}

// All comparisons are for *ALL* four values (as executing _and_ for componentwise operations).

_INLINE bool   operator==(sse_f a, sse_f b) { return _mm_movemask_ps(_mm_cmpeq_ps(a, b)) == 0xf; }
_INLINE bool   operator!=(sse_f a, sse_f b) { return _mm_movemask_ps(_mm_cmpeq_ps(a, b)) != 0xf; }
_INLINE bool   operator< (sse_f a, sse_f b) { return _mm_movemask_ps(_mm_cmplt_ps(a, b)) == 0xf; }
_INLINE bool   operator<=(sse_f a, sse_f b) { return _mm_movemask_ps(_mm_cmple_ps(a, b)) == 0xf; }
_INLINE bool   operator> (sse_f a, sse_f b) { return _mm_movemask_ps(_mm_cmpgt_ps(a, b)) == 0xf; }
_INLINE bool   operator>=(sse_f a, sse_f b) { return _mm_movemask_ps(_mm_cmpge_ps(a, b)) == 0xf; }
_INLINE bool   operator==(sse_f a, float b)  { return a == convert<sse_f>(b); }
_INLINE bool   operator!=(sse_f a, float b)  { return a != convert<sse_f>(b); }
_INLINE bool   operator< (sse_f a, float b)  { return a <  convert<sse_f>(b); }
_INLINE bool   operator<=(sse_f a, float b)  { return a <= convert<sse_f>(b); }
_INLINE bool   operator> (sse_f a, float b)  { return a >  convert<sse_f>(b); }
_INLINE bool   operator>=(sse_f a, float b)  { return a >= convert<sse_f>(b); }
_INLINE bool   operator==(float a, sse_f b)  { return convert<sse_f>(a) == b; }
_INLINE bool   operator!=(float a, sse_f b)  { return convert<sse_f>(a) != b; }
_INLINE bool   operator< (float a, sse_f b)  { return convert<sse_f>(a) <  b; }
_INLINE bool   operator<=(float a, sse_f b)  { return convert<sse_f>(a) <= b; }
_INLINE bool   operator> (float a, sse_f b)  { return convert<sse_f>(a) >  b; }
_INLINE bool   operator>=(float a, sse_f b)  { return convert<sse_f>(a) >= b; }

_INLINE sse_f operator& (sse_f a, sse_f b) { return _mm_and_ps(a, b); }
_INLINE sse_f operator| (sse_f a, sse_f b) { return _mm_or_ps (a, b); }
_INLINE sse_f operator+ (sse_f a, sse_f b) { return _mm_add_ps(a, b); }
_INLINE sse_f operator- (sse_f a, sse_f b) { return _mm_sub_ps(a, b); }
_INLINE sse_f operator* (sse_f a, sse_f b) { return _mm_mul_ps(a, b); }
_INLINE sse_f operator/ (sse_f a, sse_f b) { return _mm_mul_ps(a, rcp(b)); }
_INLINE sse_f operator& (sse_f a, float b)  { return _mm_and_ps(a, convert<sse_f>(b)); }
_INLINE sse_f operator| (sse_f a, float b)  { return _mm_or_ps (a, convert<sse_f>(b)); }
_INLINE sse_f operator+ (sse_f a, float b)  { return _mm_add_ps(a, convert<sse_f>(b)); }
_INLINE sse_f operator- (sse_f a, float b)  { return _mm_sub_ps(a, convert<sse_f>(b)); }
_INLINE sse_f operator* (sse_f a, float b)  { return _mm_mul_ps(a, convert<sse_f>(b)); }
_INLINE sse_f operator/ (sse_f a, float b)  { return _mm_mul_ps(a, convert<sse_f>(1.0f/b)); }
_INLINE sse_f operator& (float a, sse_f b)  { return _mm_and_ps(convert<sse_f>(a), b); }
_INLINE sse_f operator| (float a, sse_f b)  { return _mm_or_ps (convert<sse_f>(a), b); }
_INLINE sse_f operator+ (float a, sse_f b)  { return _mm_add_ps(convert<sse_f>(a), b); }
_INLINE sse_f operator- (float a, sse_f b)  { return _mm_sub_ps(convert<sse_f>(a), b); }
_INLINE sse_f operator* (float a, sse_f b)  { return _mm_mul_ps(convert<sse_f>(a), b); }
_INLINE sse_f operator/ (float a, sse_f b)  { return _mm_mul_ps(convert<sse_f>(a), rcp(b)); }

_INLINE const sse_f& operator&=(sse_f& a, sse_f b) { return (a = _mm_and_ps(a, b)); }
_INLINE const sse_f& operator|=(sse_f& a, sse_f b) { return (a = _mm_or_ps (a, b)); }
_INLINE const sse_f& operator+=(sse_f& a, sse_f b) { return (a = _mm_add_ps(a, b)); }
_INLINE const sse_f& operator-=(sse_f& a, sse_f b) { return (a = _mm_sub_ps(a, b)); }
_INLINE const sse_f& operator*=(sse_f& a, sse_f b) { return (a = _mm_mul_ps(a, b)); }
_INLINE const sse_f& operator/=(sse_f& a, sse_f b) { return (a = _mm_mul_ps(a, rcp(b))); }
_INLINE const sse_f& operator&=(sse_f& a, float b)  { return (a = _mm_and_ps(a, convert<sse_f>(b))); }
_INLINE const sse_f& operator|=(sse_f& a, float b)  { return (a = _mm_or_ps (a, convert<sse_f>(b))); }
_INLINE const sse_f& operator+=(sse_f& a, float b)  { return (a = _mm_add_ps(a, convert<sse_f>(b))); }
_INLINE const sse_f& operator-=(sse_f& a, float b)  { return (a = _mm_sub_ps(a, convert<sse_f>(b))); }
_INLINE const sse_f& operator*=(sse_f& a, float b)  { return (a = _mm_mul_ps(a, convert<sse_f>(b))); }
_INLINE const sse_f& operator/=(sse_f& a, float b)  { return (a = _mm_mul_ps(a, convert<sse_f>(1.0f/b))); }

_INLINE sse_f abs(sse_f a) {
  static const sse_f sse_sign  = cast(convert<sse_i>(0x7fffffff));
  return _mm_and_ps(a, sse_sign);
}


_INLINE sse_f min(sse_f a, sse_f b) {
    return _mm_min_ps(a,b);
}

_INLINE sse_f max(sse_f a, sse_f b) {
    return _mm_max_ps(a,b);
}

_INLINE sse_f rcp_ss(float a) {
    sse_f src = _mm_set_ss(a);
    sse_f dst = _mm_rcp_ss(src);
    src = _mm_mul_ss(src, dst);
    src = _mm_mul_ss(src, dst);
    dst = _mm_add_ss(dst, dst);
    dst = _mm_sub_ss(dst, src);
    return dst;
}

/// Overloaded sqrt (similar to scalar versions).
_INLINE sse_f sqrt(sse_f a) {
    return _mm_sqrt_ps(a);
}

/// Returns 1/a
_INLINE sse_f rsqrt(sse_f a) {
    static const sse_f sse_three =    convert(3.0f);
    static const sse_f sse_minushalf =    convert(-0.5f);
    sse_f rsqrta = _mm_rsqrt_ps(a);
    return (((a*rsqrta)*rsqrta - sse_three)*sse_minushalf*rsqrta);
}

/// Overloaded scalar version.
_INLINE float rsqrt(float a) {
  return 1.0f / sqrtf(a);
}


_INLINE std::ostream& operator<<(std::ostream& out, const sse_f& t) {
    float* f = (float*)&t;
    out << "[" << f[0] << "," << f[1] << "," << f[2] << "," << f[3] << "] ";
    return out;
}

// Integers.

_INLINE bool operator==(sse_i a, sse_i b) { return _mm_movemask_epi8(_mm_cmpeq_epi32(a, b)) == 0xffff; }
_INLINE bool operator!=(sse_i a, sse_i b) { return !(a == b); }
_INLINE bool operator< (sse_i a, sse_i b) { return _mm_movemask_epi8(_mm_cmplt_epi32(a, b)) == 0xffff; }
_INLINE bool operator<=(sse_i a, sse_i b) { return _mm_movemask_epi8(_mm_cmpgt_epi32(a, b)) == 0x0; }
_INLINE bool operator> (sse_i a, sse_i b) { return _mm_movemask_epi8(_mm_cmpgt_epi32(a, b)) == 0xffff; }
_INLINE bool operator>=(sse_i a, sse_i b) { return _mm_movemask_epi8(_mm_cmplt_epi32(a, b)) == 0x0; }
_INLINE bool operator==(sse_i a, int b)     { return a == convert<sse_i>(b); }
_INLINE bool operator!=(sse_i a, int b)     { return a != convert<sse_i>(b); }
_INLINE bool operator< (sse_i a, int b)     { return a <  convert<sse_i>(b); }
_INLINE bool operator<=(sse_i a, int b)     { return a <= convert<sse_i>(b); }
_INLINE bool operator> (sse_i a, int b)     { return a >  convert<sse_i>(b); }
_INLINE bool operator>=(sse_i a, int b)     { return a >= convert<sse_i>(b); }
_INLINE bool operator==(int a, sse_i b)     { return convert<sse_i>(a) == b; }
_INLINE bool operator!=(int a, sse_i b)     { return convert<sse_i>(a) != b; }
_INLINE bool operator< (int a, sse_i b)     { return convert<sse_i>(a) <  b; }
_INLINE bool operator<=(int a, sse_i b)     { return convert<sse_i>(a) <= b; }
_INLINE bool operator> (int a, sse_i b)     { return convert<sse_i>(a) >  b; }
_INLINE bool operator>=(int a, sse_i b)     { return convert<sse_i>(a) >= b; }

_INLINE sse_i operator&(sse_i a, sse_i b) { return _mm_and_si128(a, b); }
_INLINE sse_i operator|(sse_i a, sse_i b) { return _mm_or_si128 (a, b); }
_INLINE sse_i operator+(sse_i a, sse_i b) { return _mm_add_epi32(a, b); }
_INLINE sse_i operator-(sse_i a, sse_i b) { return _mm_sub_epi32(a, b); }
_INLINE sse_i operator*(sse_i a, sse_i b) { return convert(convert(a) * convert(b)); }
_INLINE sse_i operator&(sse_i a,  int b)    { return _mm_and_si128(a, convert<sse_i>(b)); }
_INLINE sse_i operator|(sse_i a,  int b)    { return _mm_or_si128 (a, convert<sse_i>(b)); }
_INLINE sse_i operator+(sse_i a, int b)     { return _mm_add_epi32(a, convert<sse_i>(b)); }
_INLINE sse_i operator-(sse_i a, int b)     { return _mm_sub_epi32(a, convert<sse_i>(b)); }
_INLINE sse_i operator*(sse_i a, int b)     { return a * convert<sse_i>(b); }
_INLINE sse_i operator&(int a, sse_i b)     { return _mm_and_si128(convert<sse_i>(a), b); }
_INLINE sse_i operator|(int a, sse_i b)     { return _mm_or_si128 (convert<sse_i>(a), b); }
_INLINE sse_i operator+(int a, sse_i b)     { return _mm_add_epi32(convert<sse_i>(a), b); }
_INLINE sse_i operator-(int a, sse_i b)     { return _mm_sub_epi32(convert<sse_i>(a), b); }
_INLINE sse_i operator*(int a, sse_i b)     { return convert<sse_i>(a) * b; }

_INLINE const sse_i& operator&=(sse_i& a, sse_i b) { return (a = _mm_and_si128(a, b)); }
_INLINE const sse_i& operator|=(sse_i& a, sse_i b) { return (a = _mm_or_si128 (a, b)); }
_INLINE const sse_i& operator+=(sse_i& a, sse_i b) { return (a = _mm_add_epi32(a, b)); }
_INLINE const sse_i& operator-=(sse_i& a, sse_i b) { return (a = _mm_sub_epi32(a, b)); }
_INLINE const sse_i& operator*=(sse_i& a, sse_i b) { return (a = a * b); }
_INLINE const sse_i& operator&=(sse_i& a, int b)     { return (a = _mm_and_si128(a, convert<sse_i>(b))); }
_INLINE const sse_i& operator|=(sse_i& a, int b)     { return (a = _mm_or_si128 (a, convert<sse_i>(b))); }
_INLINE const sse_i& operator+=(sse_i& a, int b)     { return (a = _mm_add_epi32(a, convert<sse_i>(b))); }
_INLINE const sse_i& operator-=(sse_i& a, int b)     { return (a = _mm_sub_epi32(a, convert<sse_i>(b))); }
_INLINE const sse_i& operator*=(sse_i& a, int b)     { return (a = a * convert<sse_i>(b)); }

_INLINE sse_i abs(sse_i a) { return _mm_srli_epi32(_mm_slli_epi32(a,1),1); }

_INLINE std::ostream& operator<<(std::ostream& out, const sse_i& t) {
    int* f = (int*)&t;
    out << "[" << f[0] << "," << f[1] << "," << f[2] << "," << f[3] << "] ";
    return out;
}


_INLINE bool isNAN(const sse_f& a)
{
  return a == _mm_set_ps1(numeric_limits<float>::quiet_NaN());
}
_INLINE bool isNaN(const sse_f& a)
{
  return isNAN(a);
}

_INLINE bool isINF(const sse_f& a)
{
  return a == _mm_set_ps1(numeric_limits<float>::infinity());
  //return isinff(M128_FLOAT(a,0)) | isinff(M128_FLOAT(a,1)) | isinff(M128_FLOAT(a,2)) | isinff(M128_FLOAT(a,3));
}


_INLINE float minHorizontal3f(sse_f a)
{
  return min(M128_FLOAT(a,0),min(M128_FLOAT(a,1),M128_FLOAT(a,2)));
}

_INLINE float maxHorizontal3f(sse_f a)
{
  return max(M128_FLOAT(a,0),max(M128_FLOAT(a,1),M128_FLOAT(a,2)));
}

_INLINE sse_f setHorizontalMin3f(const sse_f& x,const sse_f& y,const sse_f& z, const float v=0.0f)
{
  return _mm_setr_ps(minHorizontal3f(x),minHorizontal3f(y),minHorizontal3f(z),v);
}

_INLINE sse_f setHorizontalMax3f(const sse_f& x,const sse_f& y,const sse_f& z, const float v=0.0f)
{
  return _mm_setr_ps(maxHorizontal3f(x),maxHorizontal3f(y),maxHorizontal3f(z),v);
}

//#include "RTSSEAliases.hxx"

template<int i>
sse_f splat4(const sse_f& v)
{
  return _mm_shuffle_ps(v,v,_MM_SHUFFLE(i,i,i,i));
}

template<int i>
sse_i splat4i(const sse_i& v)
{
  return _mm_shuffle_epi32(v,_MM_SHUFFLE(i,i,i,i));
}

_INLINE sse_f choose4(const sse_f& mask, const sse_f& iftrue, const sse_f& iffalse)
{
  return  _mm_blendv_ps(iftrue,iffalse,mask);
//   return _mm_or_ps(_mm_and_ps(mask,iftrue),_mm_andnot_ps(mask,iffalse));
}


/*! to avoid numerical issues with values that we will divide by,
  lateron: if value is zero, make it at least (positive) epsilon, so
  we can then divide w/o getting nans etc) */
_INLINE void forceZeroToEpsilon(sse_f &val)
{ val = choose4(_mm_cmpeq_ps(val,_mm_setzero_ps()),_mm_set_ps1(1e-6f),val); }
#endif
