/// \file RTVecBody.h
/// Defines basic data array
/// and its specializations for differen alignment types.
#ifndef RTTL_VEC_H
#error RTData.hxx has to be included into RTVec.hxx.
#endif

#ifndef RTTL_DATA_H
#define RTTL_DATA_H

namespace RTTL {
    
    template<int N, typename DataType, int align = 0>
    struct RTData_t {
    private:
        /// Disallow generic cases.
        RTData_t();
    };

    /// Default (non-aligned instantiation).
    template<int N, typename DataType>
    struct RTData_t<N, DataType, 0> {

        typedef DataType AlignedDataType;
        int alignment() const { return 0; }

        /// We do not overload assign ops to help compiler disambiguate them more easily.
        _INLINE void assignDataTypeValue(const DataType& v) {
            for (int i = 0; i < N; i++)
                t[i] = v;
        }
        _INLINE void assignDataTypeArray(const DataType v[]) {
            for (int i = 0; i < N; i++)
                t[i] = v[i];
        }
        /// Assignment from entities of different types is allowed, conditionally.
        /// This freedom is restricted to assignments and ctors only
        /// to facilitate reasonable data conversions
        /// (which are made deliberately and in sound mind).
        /// AnotherDataType must have operator[].
        /// This is strictly prohibited for RTData_t though.
        template<int AnotherN, typename AnotherDataType, int AnotherAlign>
        _INLINE void assign(const RTData_t<AnotherN, AnotherDataType, AnotherAlign>& v) {
            assert(nElements() <= v.nElements());
            // aka operator=
            for (int i = 0; i < N; i++)
                t[i] = v[i];
        }
        _INLINE DataType& operator[](int index) {
            return t[index];
        }
        _INLINE DataType operator[](int index) const {
            return t[index];
        }
        
        _INLINE void operator+=(const RTData_t& x) {
            for (int i = 0; i < N; i++)
                t[i] += x.t[i];
        }
        
        _INLINE void operator-=(const RTData_t& x) {
            for (int i = 0; i < N; i++)
                t[i] -= x.t[i];
        }
        
        _INLINE void operator*=(const RTData_t& x) {
            for (int i = 0; i < N; i++)
                t[i] *= x.t[i];
        }
        
        _INLINE void operator/=(const RTData_t& x) {
            for (int i = 0; i < N; i++)
                t[i] /= x.t[i];
        }
        
        _INLINE void operator+=(const DataType& q) {
            for (int i = 0; i < N; i++)
                t[i] += q;
        }
        
        _INLINE void operator-=(const DataType& q) {
            for (int i = 0; i < N; i++)
                t[i] -= q;
        }
        
        _INLINE void operator*=(const DataType& q) {
            for (int i = 0; i < N; i++)
                t[i] *= q;
        }
        
        _INLINE void operator/=(const DataType& q) {
            for (int i = 0; i < N; i++)
                t[i] /= q;
        }
        
        _INLINE void add(const RTData_t& v1, const RTData_t& v2) {
            for (int i = 0; i < N; i++)
                t[i] = v1.t[i] + v2.t[i];
        }
        _INLINE void subtract(const RTData_t& v1, const RTData_t& v2) {
            for (int i = 0; i < N; i++)
                t[i] = v1.t[i] - v2.t[i];
        }
        _INLINE void multiply(const RTData_t& v1, const RTData_t& v2) {
            for (int i = 0; i < N; i++)
                t[i] = v1.t[i] * v2.t[i];
        }
        _INLINE void divide(const RTData_t& v1, const RTData_t& v2) {
            for (int i = 0; i < N; i++)
                t[i] = v1.t[i] / v2.t[i];
        }
        
        _INLINE void add(const RTData_t& v1, const DataType v2[]) {
            for (int i = 0; i < N; i++)
                t[i] = v1.t[i] + v2[i];
        }
        _INLINE void subtract(const RTData_t& v1, const DataType v2[]) {
            for (int i = 0; i < N; i++)
                t[i] = v1.t[i] - v2[i];
        }
        _INLINE void multiply(const RTData_t& v1, const DataType v2[]) {
            for (int i = 0; i < N; i++)
                t[i] = v1.t[i] * v2[i];
        }
        _INLINE void divide(const RTData_t& v1, const DataType v2[]) {
            for (int i = 0; i < N; i++)
                t[i] = v1.t[i] / v2[i];
        }
        
        _INLINE void add(const DataType v1[], const RTData_t& v2) {
            add(v2, v1);
        }
        _INLINE void subtract(const DataType v1[], const RTData_t& v2) {
            subtract(v2, v1);
        }
        _INLINE void multiply(const DataType v1[], const RTData_t& v2) {
            multiply(v2, v1);
        }
        _INLINE void divide(const DataType v1[], const RTData_t& v2) {
            for (int i = 0; i < N; i++)
                t[i] = v1[i] / v2.t[i];
        }
        
        template<typename ScalarType>
        _INLINE void addScalar(const RTData_t& a, const ScalarType& b) {
            for (int i = 0; i < N; i++)
                t[i] = a.t[i] + b;
        }
        template<typename ScalarType>
        _INLINE void subtractScalar(const RTData_t& a, const ScalarType& b) {
            for (int i = 0; i < N; i++)
                t[i] = a.t[i] - b;
        }
        template<typename ScalarType>
        _INLINE void subtractScalar(const ScalarType& b, const RTData_t& a) {
            for (int i = 0; i < N; i++)
                t[i] = b - a.t[i];
        }
        template<typename ScalarType>
        _INLINE void multiplyScalar(const RTData_t& a, const ScalarType& b) {
            for (int i = 0; i < N; i++)
                t[i] = a.t[i] * b;
        }
        template<typename ScalarType>
        _INLINE void divideScalar(const RTData_t& a, const ScalarType& b) {
            for (int i = 0; i < N; i++)
                t[i] = a.t[i] / b;
        }
        template<typename ScalarType>
        _INLINE void divideScalar(const ScalarType& b, const RTData_t& a) {
            for (int i = 0; i < N; i++)
                t[i] = b / a.t[i];
        }

        static int nElements() { return N; }
        
    protected:
        DataType t[N];
    };
    
    template<int N, typename DataType>
    _INLINE bool operator==(const RTData_t<N, DataType, 0>& v1, const RTData_t<N, DataType, 0>& v2) {
        for (int i = 0; i < N; i++)
            if (v1[i] != v2[i])
                return false;
        
        return true;
    }
    template<int N, typename DataType>
    _INLINE bool operator==(const RTData_t<N, DataType, 0>& v1, const DataType v2[]) {
        for (int i = 0; i < N; i++)
            if (v1[i] != v2[i])
                return false;
        
        return true;
    }
    template<int N, typename DataType>
    _INLINE bool operator==(const DataType v1[], const RTData_t<N, DataType, 0>& v2) {
        for (int i = 0; i < N; i++)
            if (v1[i] != v2[i])
                return false;
        
        return true;
    }
    template<int N, typename DataType>
    _INLINE bool operator==(const RTData_t<N, DataType, 0>& v1, DataType v2) {
        for (int i = 0; i < N; i++)
            if (v1[i] != v2)
                return false;
        
        return true;
    }
    template<int N, typename DataType>
    _INLINE bool operator==(DataType v1, const RTData_t<N, DataType, 0>& v2) {
        for (int i = 0; i < N; i++)
            if (v1 != v2[i])
                return false;
        
        return true;
    }
    
    /// define one (or more) aligned data types.
    #define ALIGNMENT 16
    template<int N, typename DataType> struct RTData_t<N, DataType, ALIGNMENT>
    #include "RTDataAligned.hxx"
};

#endif
