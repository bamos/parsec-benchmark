#ifndef RTTL_VEC_H
#define RTTL_VEC_H

#include "RTInclude.hxx"

#include <fstream>
using namespace std;

/// Explicit conversions from int/floats ...
template<typename DataType> _INLINE DataType convert(int n)   { return DataType(n); }
template<typename DataType> _INLINE DataType convert(float n) { return DataType(n); }
/// ...and their specializations for SSE values.
template<> _INLINE sse_f convert<sse_f> (int n)   { return _mm_set_ps1((float)n); }
template<> _INLINE sse_f convert<sse_f> (float n) { return _mm_set_ps1(n); }
template<> _INLINE sse_i convert<sse_i>(int n)    { return _mm_set1_epi32(n); }
template<> _INLINE sse_i convert<sse_i>(float n)  { return _mm_set1_epi32((int)n); }

// Don't need templates for these conversions (overloaded function is deduced from arguments).
_INLINE sse_f convert(float n) { return _mm_set_ps1(n); }
_INLINE sse_i convert(int n)   { return _mm_set1_epi32(n); }
_INLINE sse_f convert(float n0, float n1, float n2, float n3) { return _mm_set_ps(n0,n1,n2,n3); }
_INLINE sse_i convert(int n0,   int n1,   int n2,   int n3)   { return _mm_set_epi32(n0,n1,n2,n3); }

/// SSE ops (outside rttl).
#include "RTSSE.hxx"

/// Define ops for arrays of basic data types.
#include "RTData.hxx"

namespace RTTL {
    /// Template arguments:
    /// N         - vector size
    /// DataType  - basic data type
    /// align     - alignment (0 for none, 16 for SSE, other values could also be defined).

    /// Generic vector for sizes 1, 5, 6, etc.
    template<int N, typename DataType, int align = 0> class RTVec_t
    #include "RTVecBody.h"

    /// Specialized vectors of size 2.
    #define N 2
    template<typename DataType, int align> class RTVec_t<N, DataType, align>
    #include "RTVecBody.h"

    /// Specialized vectors of size 3. Among other things,
    /// have support for cross product.
    #define N 3
    template<typename DataType, int align> class RTVec_t<N, DataType, align>
    #include "RTVecBody.h"

    /// Specialized vectors of size 4.
    #define N 4
    template<typename DataType, int align> class RTVec_t<N, DataType, align>
    #include "RTVecBody.h"

    /// Generic numerics and their SSE specializations.
    template<typename DataType>
    _INLINE DataType nan()  { return numeric_limits<DataType>::quiet_NaN(); }
    template<typename DataType>
    _INLINE DataType epsilon()  { return numeric_limits<DataType>::epsilon(); }
    template<typename DataType>
    _INLINE DataType minValue() { return numeric_limits<DataType>::min(); }
    template<typename DataType>
    _INLINE DataType maxValue() { return numeric_limits<DataType>::max(); }
    template<typename DataType>
    _INLINE DataType infinity() { return numeric_limits<DataType>::infinity(); }
    template<>
    _INLINE sse_f  nan<sse_f>()       { return _mm_set_ps1(numeric_limits<float>::quiet_NaN()); }
    template<>
    _INLINE sse_f  epsilon<sse_f>()   { return _mm_set_ps1(FLT_EPSILON); }
    template<>
    _INLINE sse_f  minValue<sse_f>()  { return _mm_set_ps1(FLT_MIN); }
    template<>
    _INLINE sse_f  maxValue<sse_f>()  { return _mm_set_ps1(FLT_MAX); }
    template<>
    _INLINE sse_f  infinity<sse_f>()  { return _mm_set_ps1(numeric_limits<float>::infinity()); }
    template<>
    _INLINE sse_i epsilon<sse_i>()    { return _mm_set1_epi32(0); }
    template<>                        
    _INLINE sse_i minValue<sse_i>()   { return _mm_set1_epi32(INT_MIN); }
    template<>                        
    _INLINE sse_i maxValue<sse_i>()   { return _mm_set1_epi32(INT_MAX); }
    template<>                        
    _INLINE sse_i infinity<sse_i>()   { return _mm_set1_epi32(numeric_limits<int>::infinity()); }

    /// Two-operand operators. All operations are performed through DataArray class.
    /// Comparison ops are overloaded wrt constantness (with the same semantic).
    template<int N, typename DataType, int align>
    _INLINE bool operator==(const RTVec_t<N, DataType, align>& v1, const RTVec_t<N, DataType, align>& v2) {
        return v1.array() == v2.array();
    }
    template<int N, typename DataType, int align>
    _INLINE bool operator==(const RTVec_t<N, DataType, align>& v1, const DataType v2[]) {
        return v1.array() == v2;
    }
    template<int N, typename DataType, int align>
    _INLINE bool operator==(const DataType v1[], const RTVec_t<N, DataType, align>& v2) {
        return v1 == v2.array();
    }
    template<int N, typename DataType, int align>
    _INLINE bool operator==(const RTVec_t<N, DataType, align>& v1, DataType v2[]) {
        return v1.array() == v2;
    }
    template<int N, typename DataType, int align>
    _INLINE bool operator==(DataType v1[], const RTVec_t<N, DataType, align>& v2) {
        return v1 == v2.array();
    }

    template<int N, typename DataType, int align>
    _INLINE bool operator!=(const RTVec_t<N, DataType, align>& v1, const RTVec_t<N, DataType, align>& v2) {
        return !(v1 == v2);
    }
    template<int N, typename DataType, int align>
    _INLINE bool operator!=(const RTVec_t<N, DataType, align>& v1, const DataType v2[]) {
        return !(v1 == v2);
    }
    template<int N, typename DataType, int align>
    _INLINE bool operator!=(const DataType v1[], const RTVec_t<N, DataType, align>& v2) {
        return !(v1 == v2);
    }
    template<int N, typename DataType, int align>
    _INLINE bool operator!=(const RTVec_t<N, DataType, align>& v1, DataType v2[]) {
        return !(v1 == v2);
    }
    template<int N, typename DataType, int align>
    _INLINE bool operator!=(DataType v1[], const RTVec_t<N, DataType, align>& v2) {
        return !(v1 == v2);
    }

    /// Arithmetic +.
    template<int N, typename DataType, int align>
    _INLINE RTVec_t<N, DataType, align> operator+(const RTVec_t<N, DataType, align>& a, const RTVec_t<N, DataType, align>& b) {
        RTVec_t<N, DataType, align> result;
        (result.array()).add(a.array(), b.array());
        return result;
    }

    template<int N, typename DataType, int align>
    _INLINE RTVec_t<N, DataType, align> operator-(const RTVec_t<N, DataType, align>& a, const RTVec_t<N, DataType, align>& b) {
        RTVec_t<N, DataType, align> result;
        (result.array()).subtract(a.array(), b.array());
        return result;
    }

    template<int N, typename DataType, int align>
    _INLINE RTVec_t<N, DataType, align> operator*(const RTVec_t<N, DataType, align>& a, const RTVec_t<N, DataType, align>& b) {
        RTVec_t<N, DataType, align> result;
        (result.array()).multiply(a.array(), b.array());
        return result;
    }

    template<int N, typename DataType, int align>
    _INLINE RTVec_t<N, DataType, align> operator/(const RTVec_t<N, DataType, align>& a, const RTVec_t<N, DataType, align>& b) {
        RTVec_t<N, DataType, align> result;
        (result.array()).divide(a.array(), b.array());
        return result;
    }

    template<int N, typename DataType, int align>
    _INLINE DataType dot(const RTVec_t<N, DataType, align>& x, const RTVec_t<N, DataType, align>& y) {
        DataType v;
        for (int i = 0; i < N; i++)
            v += x[i] * y[i];
        return v;
    }

    /// Unary operator.
    template<int N, typename DataType, int align>
    _INLINE RTVec_t<N, DataType, align> operator-(const RTVec_t<N, DataType, align>& a) {
        RTVec_t<N, DataType, align> result;
        for (int i = 0; i < N; i++)
            result[i] = -a[i];
        return result;
    }

    /// Mixed operands.
    template<int N, typename DataType, int align>
    _INLINE bool operator==(const RTVec_t<N, DataType, align>& v1, const DataType& v2) {
        return v1.array() == v2;
    }

    template<int N, typename DataType, int align>
    _INLINE bool operator==(const DataType& v1, const RTVec_t<N, DataType, align>& v2) {
        return v1 == v2.array();
    }

    template<int N, typename DataType, int align>
    _INLINE bool operator!=(const RTVec_t<N, DataType, align>& v1, const DataType& v2) {
        return !(v1 == v2);
    }

    template<int N, typename DataType, int align>
    _INLINE bool operator!=(const DataType& v1, const RTVec_t<N, DataType, align>& v2) {
        return !(v1 == v2);
    }

    /// Ops with scalar operands.
    template<int N, typename DataType, int align, typename ScalarType>
    _INLINE RTVec_t<N, DataType, align> operator+(const RTVec_t<N, DataType, align>& a, const ScalarType& b) {
        RTVec_t<N, DataType, align> result;
        (result.array()).addScalar(a.array(), b);
        return result;
    }

    template<int N, typename DataType, int align, typename ScalarType>
    _INLINE RTVec_t<N, DataType, align> operator+(const ScalarType& b, const RTVec_t<N, DataType, align>& a) {
        RTVec_t<N, DataType, align> result;
        (result.array()).addScalar(b.array(), a);
        return result;
    }

    template<int N, typename DataType, int align, typename ScalarType>
    _INLINE RTVec_t<N, DataType, align> operator-(const RTVec_t<N, DataType, align>& a, const ScalarType& b) {
        RTVec_t<N, DataType, align> result;
        (result.array()).subtractScalar(a.array(), b);
        return result;
    }

    template<int N, typename DataType, int align, typename ScalarType>
    _INLINE RTVec_t<N, DataType, align> operator-(const ScalarType& b, const RTVec_t<N, DataType, align>& a) {
        RTVec_t<N, DataType, align> result;
        (result.array()).subtractScalar(b, a.array());
        return result;
    }

    template<int N, typename DataType, int align, typename ScalarType>
    _INLINE RTVec_t<N, DataType, align> operator*(const RTVec_t<N, DataType, align>& a, const ScalarType& b) {
        RTVec_t<N, DataType, align> result;
        (result.array()).multiplyScalar(a.array(), b);
        return result;
    }

    template<int N, typename DataType, int align, typename ScalarType>
    _INLINE RTVec_t<N, DataType, align> operator*(const ScalarType& b, const RTVec_t<N, DataType, align>& a) {
        RTVec_t<N, DataType, align> result;
        (result.array()).multiplyScalar(a.array(), b);
        return result;
    }

    template<int N, typename DataType, int align, typename ScalarType>
    _INLINE RTVec_t<N, DataType, align> operator/(const RTVec_t<N, DataType, align>& a, const ScalarType& b) {
        RTVec_t<N, DataType, align> result;
        (result.array()).divideScalar(a.array(), b);
        return result;
    }

    template<int N, typename DataType, int align, typename ScalarType>
    _INLINE RTVec_t<N, DataType, align> operator/(const ScalarType& b, const RTVec_t<N, DataType, align>& a) {
        RTVec_t<N, DataType, align> result;
        (result.array()).divideScalar(b, a.array());
        return result;
    }

    /// Streaming.
    template<int N, typename DataType, int align>
    _INLINE ostream& operator<<(ostream& out, const RTVec_t<N, DataType, align>& t) {
        out << "[" << t[0];
        for (int i = 1; i < N; i++)
            out << "," << t[i];
        out << "]";

        return out;
    }
};

namespace RTTL {

    typedef RTTL::RTVec_t<2, float> RTVec2f;
    typedef RTTL::RTVec_t<3, float> RTVec3f;
    typedef RTTL::RTVec_t<4, float> RTVec4f;
    typedef RTTL::RTVec_t<2, int  > RTVec2i;
    typedef RTTL::RTVec_t<3, int  > RTVec3i;
    typedef RTTL::RTVec_t<4, int  > RTVec4i;
    
    _INLINE int maxDim3(const sse_f t) {
        return ((RTVec3f*)&t)->maxIndex();
    }

    _INLINE sse_f vec3fToSSE(const RTVec3f &v, const float w = 0.0f) {
        return _mm_setr_ps(v.x,v.y,v.z,w);
    }

};

#endif
