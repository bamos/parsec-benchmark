/// \file RTDataAligned.hxx
/// Defines aligned data array (defined by macro ALIGNMENT).
/// \note This file is not to be used directly; it is included by RTData.hxx.
{
#ifndef ALIGNMENT
#error wrong use of __FILE__
#endif

    /// Define aligned data type (will be used in aligned vectors).
    typedef /*_ALIGN(ALIGNMENT)*/ DataType AlignedDataType;
    /// Return alignment as an integer value.
    int alignment() const { return ALIGNMENT; }

    _INLINE void assignDataTypeValue(DataType v) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = _mm_set_ps1(v);
    }
    _INLINE void assignDataTypeArray(const DataType v[]) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = ((sse_f*)v)[i];
    }
    
    template<int AnotherN, typename AnotherDataType, int AnotherAlign>
    _INLINE void assign(const RTData_t<AnotherN, AnotherDataType, AnotherAlign>& v) {
        // Use unaligned RTData_t to assign entries one by one (and possibly convert them).
        ((RTData_t<N, DataType, 0>*)this)->assign(v);
    }
    /// Non-templated version as well (for assigning the same type).
    _INLINE void assign(const RTData_t& v) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = ((sse_f*)v.t)[i];
    }
    _INLINE DataType& operator[](int index) {
        return t[index];
    }
    _INLINE DataType operator[](int index) const {
        return t[index];
    }
    
    _INLINE void operator+=(const RTData_t& x) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] += ((sse_f*)x.t)[i];
    }
    
    _INLINE void operator-=(const RTData_t& x) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] -= ((sse_f*)x.t)[i];
    }
    
    _INLINE void operator*=(const RTData_t& x) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] *= ((sse_f*)x.t)[i];
    }
    
    _INLINE void operator/=(const RTData_t& x) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] /= ((sse_f*)x.t)[i];
    }
    
    _INLINE void operator+=(const DataType& q) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] += q;
    }
    
    _INLINE void operator-=(const DataType& q) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] -= q;
    }
    
    _INLINE void operator*=(const DataType& q) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] *= q;
    }
    
    _INLINE void operator/=(const DataType& q) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] /= q;
    }
    
    _INLINE void add(const RTData_t& v1, const RTData_t& v2) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = ((sse_f*)v1.t)[i] + ((sse_f*)v2.t)[i];
    }
    _INLINE void subtract(const RTData_t& v1, const RTData_t& v2) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = ((sse_f*)v1.t)[i] - ((sse_f*)v2.t)[i];
    }
    _INLINE void multiply(const RTData_t& v1, const RTData_t& v2) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = ((sse_f*)v1.t)[i] * ((sse_f*)v2.t)[i];
    }
    _INLINE void divide(const RTData_t& v1, const RTData_t& v2) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = ((sse_f*)v1.t)[i] / ((sse_f*)v2.t)[i];
    }
    
    _INLINE void add(const RTData_t& v1, const DataType v2[]) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = ((sse_f*)v1.t)[i] + ((sse_f*)v2)[i];
    }
    _INLINE void subtract(const RTData_t& v1, const DataType v2[]) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = ((sse_f*)v1.t)[i] - ((sse_f*)v2)[i];
    }
    _INLINE void multiply(const RTData_t& v1, const DataType v2[]) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = ((sse_f*)v1.t)[i] * ((sse_f*)v2)[i];
    }
    _INLINE void divide(const RTData_t& v1, const DataType v2[]) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = ((sse_f*)v1.t)[i] / ((sse_f*)v2)[i];
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
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = v1[i] / ((sse_f*)v2.t)[i];
    }
    
    template<typename ScalarType>
        _INLINE void addScalar(const RTData_t& a, const ScalarType& b) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = ((sse_f*)a.t)[i] + b;
    }
    template<typename ScalarType>
        _INLINE void subtractScalar(const RTData_t& a, const ScalarType& b) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = ((sse_f*)a.t)[i] - b;
    }
    template<typename ScalarType>
        _INLINE void subtractScalar(const ScalarType& b, const RTData_t& a) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = b - ((sse_f*)a.t)[i];
    }
    template<typename ScalarType>
        _INLINE void multiplyScalar(const RTData_t& a, const ScalarType& b) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = ((sse_f*)a.t)[i] * b;
    }
    template<typename ScalarType>
        _INLINE void divideScalar(const RTData_t& a, const ScalarType& b) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = ((sse_f*)a.t)[i] / b;
    }
    template<typename ScalarType>
        _INLINE void divideScalar(const ScalarType& b, const RTData_t& a) {
        for (int i = 0; i < 1+(N-1)/4; i++)
            ((sse_f*)t)[i] = b / ((sse_f*)a.t)[i];
    }
    
    static int nElements() { return N; }

 protected:
    /// Data with padding.
    AlignedDataType t[(N%ALIGNMENT)? (N - (N%ALIGNMENT) + ALIGNMENT) : N];
};

template<int N, typename DataType>
_INLINE bool operator==(const RTData_t<N, DataType, ALIGNMENT>& v1, const RTData_t<N, DataType, ALIGNMENT>& v2) {
    for (int i = 0; i < 1+(N-1)/4; i++)
        if (((sse_f*)&v1)[i] != ((sse_f*)&v2)[i])
            return false;
    
    return true;
}
template<int N, typename DataType>
_INLINE bool operator==(const RTData_t<N, DataType, ALIGNMENT>& v1, const DataType v2[]) {
    for (int i = 0; i < 1+(N-1)/4; i++)
        if (((sse_f*)&v1)[i] != ((sse_f*)v2)[i])
            return false;
    
    return true;
}
template<int N, typename DataType>
_INLINE bool operator==(const DataType v1[], const RTData_t<N, DataType, ALIGNMENT>& v2) {
    for (int i = 0; i < 1+(N-1)/4; i++)
        if (((sse_f*)v1)[i] != ((sse_f*)&v2)[i])
            return false;
    
    return true;
}
template<int N, typename DataType>
_INLINE bool operator==(const RTData_t<N, DataType, ALIGNMENT>& v1, DataType v2) {
    for (int i = 0; i < 1+(N-1)/4; i++)
        if (((sse_f*)&v1)[i] != _mm_set_ps1(v2))
            return false;
    
    return true;
}
template<int N, typename DataType>
_INLINE bool operator==(DataType v1, const RTData_t<N, DataType, ALIGNMENT>& v2) {
    return v2 == v1;
}

#undef ALIGNMENT
