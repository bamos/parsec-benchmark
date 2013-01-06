/// \file RTVecBody.h
/// Defines implementation of RTVec_t class for general case (N is not defined)
/// and its specializations for N = 2,3,4.
/// \note This file is not to be used directly; it is included by RTVec.hxx.
{
#if   N == 4
#define _C4(t) ,t
#define _M4(t)  t
#define _C3(t) ,t
#define _M3(t)  t
#elif N == 3
#define _C4(t)
#define _M4(t)
#define _C3(t) ,t
#define _M3(t)  t
#elif N == 2
#define _C4(t)
#define _M4(t)
#define _C3(t)
#define _M3(t)
#endif

 public:

    /// DataArray - type describing array of size N of DataType with alignment align.
    typedef RTData_t<N, DataType, align> DataArray;

    /// "User-defined" default constructor.
    RTVec_t() {
        // _NO_ default initialization!
    }

    /// Explicit ctor (set everthing to v).
    explicit RTVec_t(const DataType& v) {
        DataArray& t = *this;
        t.assignDataTypeValue(v);
    }

    /// Ctor from vector of different type is allowed, conditionally.
    /// This freedom is restricted to assignments and ctors only
    /// to facilitate reasonable data conversions
    /// (which are made deliberately and in sound mind).
    template<int AnotherN, typename AnotherDataType, int AnotherAlign>
    RTVec_t(const RTVec_t<AnotherN, AnotherDataType, AnotherAlign>& x) {
        DataArray& t = *this;
        const RTData_t<AnotherN, AnotherDataType, AnotherAlign>& xt = x;
        t.assign(xt);
    }

    RTVec_t(const DataType x[]) {
        DataArray& t = *this;
        t.assignDataTypeArray(x);
    }

    /// Just for convenience...
    static int nElements() { return N; }
    static int entrySize() { return sizeof(DataType); }
    static int totalSize() { return N*sizeof(DataType); }

    /// One-operand operators.
    /// Assignment from vectors of different types is allowed, conditionally.
    /// This freedom is restricted to assignments and ctors only
    /// to facilitate reasonable data conversions
    /// (which are made deliberately and in sound mind).
    template<int AnotherN, typename AnotherDataType, int AnotherAlign>
    _INLINE const RTVec_t& operator=(const RTVec_t<AnotherN, AnotherDataType, AnotherAlign>& x) {
        assert(nElements() <= x.nElements());
        DataArray& t = *this;
        const RTData_t<AnotherN, AnotherDataType, AnotherAlign>& xt = x;
        t.assign(xt);
        return *this;
    }
    /// Non-templated version as well (for assigning the same type).
    _INLINE const RTVec_t& operator=(const RTVec_t& x) {
        DataArray& t = *this;
        const DataArray& xt = x;
        t.assign(xt);
        return *this;
    }

    _INLINE const RTVec_t& operator=(const DataType& x) {
        DataArray& t = *this;
        t.assignDataTypeValue(x);
        return *this;
    }

    _INLINE const RTVec_t& operator+=(const RTVec_t& x) {
        DataArray& t = *this;
        t += x;
        return *this;
    }

    _INLINE const RTVec_t& operator-=(const RTVec_t& x) {
        DataArray& t = *this;
        t -= x;
        return *this;
    }

    _INLINE const RTVec_t& operator*=(const RTVec_t& x) {
        DataArray& t = *this;
        t *= x;
        return *this;
    }

    _INLINE const RTVec_t& operator/=(const RTVec_t& x) {
        DataArray& t = *this;
        t /= x;
        return *this;
    }

    _INLINE const RTVec_t& operator+=(const DataType& q) {
        DataArray& t = *this;
        t += q;
        return *this;
    }

    _INLINE const RTVec_t& operator-=(const DataType& q) {
        DataArray& t = *this;
        t -= q;
        return *this;
    }

    _INLINE const RTVec_t& operator*=(const DataType& q) {
        DataArray& t = *this;
        t *= q;
        return *this;
    }

    _INLINE const RTVec_t& operator/=(const DataType& q) {
        DataArray& t = *this;
        t /= q;
        return *this;
    }

    _INLINE DataType dot(const RTVec_t& x) const {
        DataType v;
        DataArray& t = *this;
        const DataArray& xt = x;
        for (int i = 0; i < N; i++)
            v += t[i] * xt[i];
        return v;
    }

    /// Length^2 and length.
    _INLINE DataType lengthSquared() const {
        const DataArray& t = *this;
        DataType v = t[0] * t[0];
        for (int i = 1; i < N; i++)
            v += t[i] * t[i];
        return v;
    }

    _INLINE DataType length() const {
        return sqrt(lengthSquared());
    }

    _INLINE void normalize() {
        DataType q = 1/length();
        DataArray& t = *this;
        for (int i = 0; i < N; i++)
            t[i] *= q;
    }

    _INLINE void setMin(const RTVec_t<N, DataType>& other) {
        DataArray& t = *this;
        for (int i = 0; i < N; i++)
            t[i] = min(t[i], other[i]);
    }
    _INLINE void setMin(const DataType& other) {
        DataArray& t = *this;
        for (int i = 0; i < N; i++)
            t[i] = min(t[i], other);
    }
    _INLINE void setMax(const RTVec_t<N, DataType>& other) {
        DataArray& t = *this;
        for (int i = 0; i < N; i++)
            t[i] = max(t[i], other[i]);
    }
    _INLINE void setMax(const DataType& other) {
        DataArray& t = *this;
        for (int i = 0; i < N; i++)
            t[i] = max(t[i], other);
    }

    /// These functions are not defined for all DataTypes.
    static _INLINE DataType epsilon()  { return numeric_limits<DataType>::epsilon(); }
    static _INLINE DataType minValue() { return numeric_limits<DataType>::min(); }
    static _INLINE DataType maxValue() { return numeric_limits<DataType>::max(); }
    static _INLINE DataType infinity() { return numeric_limits<DataType>::infinity(); }

    // ===============================================
    // Define size-specific functions and data members
    // ===============================================

#ifdef N

    /// Component-wise ctor.
    RTVec_t(const DataType& a, const DataType& b _C3(const DataType& c) _C4(const DataType& d)): x(a), y(b) _C3(z(c)) _C4(w(d)) {}

    /// Access the data as an array.
    DataType* data()             { return &x; }
    const DataType* data() const { return &x; }

    /// Indices.
    DataType& operator[](int index)       { return data()[index]; }
    DataType  operator[](int index) const { return data()[index]; }

    /// Casts to pointer to DataType (equivalent to using data()).
    operator       DataType*(void)        { return &x; }
    operator const DataType*(void)  const { return &x; }

    /// Casts to reference to DataArray.
    operator DataArray&(void)             { return (DataArray&)x; }
    operator const DataArray&(void) const { return (DataArray&)x; }
    DataArray& array(void)                { return (DataArray&)x; }
    const DataArray& array(void)    const { return (DataArray&)x; }

    /// Templated and recasted access functions (some DataType/CastType combinations are meaningless).
    template<typename CastType>
    const CastType& entry(int i = 0) const {
      return *((const CastType*)data() + i);
    }

    /// Templated recasting
    /// (some DataType/CastType combinations are meaningless).
    template<typename CastType>
    CastType& entry(int i = 0) {
      return *((CastType*)data() + i);
    }

    template<typename CastType>
    const CastType* pointer() const {
      return (const CastType*)data();
    }

    template<typename CastType>
    CastType* pointer() {
      return (CastType*)data();
    }

    /// Default casts (to DataType).
    const DataType& entry(int i = 0) const {
      return *((const DataType*)data() + i);
    }

    DataType& entry(int i = 0) {
        return *((DataType*)data() + i);
    }

    const DataType* pointer() const {
      return (const DataType*)data();
    }

    DataType* pointer() {
      return (DataType*)data();
    }

#if   N == 4
    _INLINE DataType minimum()    const { return min(min(x,y), min(z,w)); }
    _INLINE DataType maximum()    const { return max(max(x,y), max(z,w)); }
    _INLINE DataType absMinimum() const { return min(min(::abs(x),::abs(y)), min(::abs(z),::abs(w))); }
    _INLINE DataType absMaximum() const { return max(max(::abs(x),::abs(y)), max(::abs(z),::abs(w))); }
    _INLINE int minIndex() const {
      int i;
      DataType extr = infinity();
      if (x < extr) { extr = x; i = 0; }
      if (y < extr) { extr = y; i = 1; }
      if (z < extr) { extr = z; i = 2; }
      if (w < extr) { extr = w; i = 3; }
      return i;
    }
    _INLINE int maxIndex() const {
      int i;
      DataType extr = -infinity();
      if (x > extr) { extr = x; i = 0; }
      if (y > extr) { extr = y; i = 1; }
      if (z > extr) { extr = z; i = 2; }
      if (w > extr) { extr = w; i = 3; }
      return i;
    }
    _INLINE int minAbsIndex() const {
      int i;
      DataType extr = infinity();
      if (::abs(x) < extr) { extr = ::abs(x); i = 0; }
      if (::abs(y) < extr) { extr = ::abs(y); i = 1; }
      if (::abs(z) < extr) { extr = ::abs(z); i = 2; }
      if (::abs(w) < extr) { extr = ::abs(w); i = 3; }
      return i;
    }
    _INLINE int maxAbsIndex() const {
      int i;
      DataType extr = -infinity();
      if (::abs(x) > extr) { extr = ::abs(x); i = 0; }
      if (::abs(y) > extr) { extr = ::abs(y); i = 1; }
      if (::abs(z) > extr) { extr = ::abs(z); i = 2; }
      if (::abs(w) > extr) { extr = ::abs(w); i = 3; }
      return i;
    }
#elif N == 3

    /// cross product of 2 3D vectors.
    _INLINE RTVec_t cross(const RTVec_t& r) const {
      return RTVec_t(y*r.z - z*r.y, z*r.x - x*r.z, x*r.y - y*r.x);
    }
    /// The same as cross.
    _INLINE RTVec_t operator^(const RTVec_t& r) const {
      return RTVec_t(y*r.z - z*r.y, z*r.x - x*r.z, x*r.y - y*r.x);
    }

    /// Normalize this.
    _INLINE RTVec_t normalize() const {
      const DataType oneOverLength = rsqrt(x*x + y*y + z*z);
      return RTVec_t(x * oneOverLength, y * oneOverLength, z * oneOverLength);
    }

    _INLINE DataType minimum()     const { return min(min(x, y), z); }
    _INLINE DataType maximum()     const { return max(max(x, y), z); }
    _INLINE DataType absMinimum()  const { return min(min(::abs(x), ::abs(y)), ::abs(z)); }
    _INLINE DataType absMaximum()  const { return max(max(::abs(x), ::abs(y)), ::abs(z)); }
    _INLINE int      minIndex()    const { return (y < x)? ((z < y)? 2:1) : ((z < x)? 2:0); }
    _INLINE int      maxIndex()    const { return (y > x)? ((z > y)? 2:1) : ((z > x)? 2:0); }
    _INLINE int      minAbsIndex() const { return (::abs(y) < ::abs(x))? ((::abs(z) < ::abs(y))? 2:1) : ((::abs(z) < ::abs(x))? 2:0); }
    _INLINE int      maxAbsIndex() const { return (::abs(y) > ::abs(x))? ((::abs(z) > ::abs(y))? 2:1) : ((::abs(z) > ::abs(x))? 2:0); }
#elif N == 2
    _INLINE DataType minimum()     const { return min(x, y); }
    _INLINE DataType maximum()     const { return max(x, y); }
    _INLINE DataType absMinimum()  const { return min(::abs(x), ::abs(y)); }
    _INLINE DataType absMaximum()  const { return max(::abs(x), ::abs(y)); }
    _INLINE int      minIndex()    const { return (y < x)? 1:0; }
    _INLINE int      maxIndex()    const { return (y > x)? 1:0; }
    _INLINE int      minAbsIndex() const { return (::abs(y) < ::abs(x))? 1:0; }
    _INLINE int      maxAbsIndex() const { return (::abs(y) > ::abs(x))? 1:0; }
#endif

    /// for vectors of size 2,3,4 data members are public
    /// using traditional x/y/z/w names.
    typename DataArray::AlignedDataType x;
    DataType y _C3(z) _C4(w);

#undef  N
#undef _C4
#undef _M4
#undef _C3
#undef _M3

#else

    /// Indices.
    DataType& operator[](int index)       { return t[index]; }
    DataType operator[](int index)  const { return t[index]; }

    /// Casts to pointer to DataType.
    operator       DataType*(void)        { return (DataType*)&t; }
    operator const DataType*(void)  const { return (DataType*)&t; }

    /// Casts to reference to DataArray.
    operator DataArray&(void)             { return t; }
    operator const DataArray&(void) const { return t; }
    DataArray& array(void)                { return t; }
    const DataArray& array(void)    const { return t; }

    /// Templated recasting
    /// (some DataType/CastType combinations are meaningless).
    template<typename CastType>
    const CastType& entry(int i = 0) const {
        return *((const CastType*)&t + i);
    }

    template<typename CastType>
    CastType& entry(int i = 0) {
        return *((CastType*)&t + i);
    }

    template<typename CastType>
    const CastType* pointer() const {
        return (const CastType*)&t;
    }

    template<typename CastType>
    CastType* pointer() {
        return (CastType*)&t;
    }

    /// Default casts (to DataType).
    const DataType& entry(int i = 0) const {
        return *((const DataType*)&t + i);
    }

    DataType& entry(int i = 0) {
        return *((DataType*)&t + i);
    }

    const DataType* pointer() const {
        return (const DataType*)&t;
    }

    DataType* pointer() {
        return (DataType*)&t;
    }

    /// min/max/index functions may not be defined for all DataTypes.
    _INLINE DataType minimum() const {
        DataType v = t[0];
        for (int i = 1; i < N; i++)
            v = min(v, t[i]);
        return v;
    }
    _INLINE DataType maximum() const {
        DataType v = t[0];
        for (int i = 1; i < N; i++)
            v = max(v, t[i]);
        return v;
    }
    _INLINE DataType absMinimum() const {
        DataType v = ::abs(t[0]);
        for (int i = 1; i < N; i++)
            v = ::abs(min(v, t[i]));
        return v;
    }
    _INLINE DataType absMaximum() const {
        DataType v = ::abs(t[0]);
        for (int i = 1; i < N; i++)
            v = ::abs(max(v, t[i]));
        return v;
    }
    _INLINE int minIndex() const {
        DataType v = t[0];
        int mi = 0;
        for (int i = 1; i < N; i++)
            if (t[i] < v) { mi = i; v = t[i]; }
        return mi;
    }
    _INLINE int maxIndex() const {
        DataType v = t[0];
        int mi = 0;
        for (int i = 1; i < N; i++)
            if (t[i] > v) { mi = i; v = t[i]; }
        return mi;
    }
    _INLINE int minAbsIndex() const {
        DataType v = ::abs(t[0]);
        int mi = 0;
        for (int i = 1; i < N; i++)
            if (::abs(t[i]) < v) { mi = i; v = ::abs(t[i]); }
        return mi;
    }
    _INLINE int maxAbsIndex() const {
        DataType v = ::abs(t[0]);
        int mi = 0;
        for (int i = 1; i < N; i++)
            if (::abs(t[i]) > v) { mi = i; v = ::abs(t[i]); }
        return mi;
    }

 protected:
    /// Protected data member (array of DataTypes) for all vectors with size > 4.
    DataArray t;
#endif
};
