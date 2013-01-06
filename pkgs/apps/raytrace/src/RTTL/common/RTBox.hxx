#ifndef RTTL_BOX_HXX
#define RTTL_BOX_HXX

#include "RTVec.hxx"

namespace RTTL {
    template<int N, typename DataType, int align = 0>
    class RTBox_t {
    public:
        typedef RTVec_t<N, DataType, align> RTVec;

        RTBox_t() {
            // _NO_ default initialization!
        }

        RTBox_t(const RTVec& cmin, const RTVec& cmax): m_min(cmin), m_max(cmax) {}

        RTBox_t(const RTVec* pts, int npts) {
            set(pts[0]);
            for (int i = 1; i < npts; i++)
                extend(pts[i]);
        }

        RTBox_t(const RTVec& pt1, const RTVec& pt2, const RTVec& pt3) {
            // Triangle's BB.
            set(pt1);
            extend(pt2);
            extend(pt3);
        }

        RTBox_t(const RTBox_t& b) : m_min(b.m_min), m_max(b.m_max) {}

        _INLINE void reset();
        _INLINE void setEmpty() { reset(); }

        /// Return index of the longest/shortest side
        _INLINE DataType maxSide() const { return (m_max - m_min).maximum(); }
        _INLINE DataType minSide() const { return (m_max - m_min).minimum(); }

        /// Return index of the longest/sortest side
        _INLINE int maxIndex() const { return (m_max - m_min).maxIndex(); }
        _INLINE int minIndex() const { return (m_max - m_min).minIndex(); }

        //// Return dimension in which box has zero width or -1 otherwise
        _INLINE int flat() const {
            for (int i = 0; i < N; i++)
                if (m_min[i] == m_max[i])
                    return i;

            return -1;
        }

        /// Return true if this box is valid (that is, min <= max).
        _INLINE bool isValid() const {
            // Zero-width sides are allowed.
            return m_max >= m_min;
        }

        /// Volume, valid for any dimension.
        _INLINE DataType volume() const {
            DataType v = 1;
            for (int i = 0; i < N; i++)
                v *= (m_max[i] - m_min[i]);

            return v;
        }

        /// Box area. Valid for 2D and 3D, for other dimensions first 3 components will be used.
        _INLINE DataType area() const {
            DataType a = (m_max[0]-m_min[0]) * (m_max[1]-m_min[1]);
            if (N >= 3) {
                a = 2 * (a +
                         (m_max[0]-m_min[0]) * (m_max[2]-m_min[2]) +
                         (m_max[1]-m_min[1]) * (m_max[2]-m_min[2]));
            }
            return a;
        }

        _INLINE void set(const RTVec& v) {
            m_min = v;
            m_max = v;
        }

        _INLINE void extend(const RTVec* pts, int npts) {
            for (int i = 0; i < npts; i++)
                extend(pts[i]);
        }

        _INLINE void around(const RTVec* pts, int npts) {
            assert(npts >= 1);
            set(pts[0]);
            extend(pts+1, npts-1);
        }

        _INLINE void extend(const RTVec& v) {
            m_min.setMin(v);
            m_max.setMax(v);
        }
        _INLINE void extend(const RTBox_t& b) {
            m_min.setMin(b.m_min);
            m_max.setMax(b.m_max);
        }
        _INLINE void extend(const DataType& v) {
            m_min.setMin(v);
            m_max.setMax(v);
        }

        _INLINE RTBox_t intersection(const RTBox_t& b) const {
            return *this - b;
        }
        _INLINE RTBox_t clip(const RTBox_t& b) const {
            return *this - b;
        }

        _INLINE bool intersect(const RTBox_t& b) const {
            return intersection(b).isValid();
        }

        /// point n is inside this
        _INLINE bool enclose(const RTVec& v) const {
            return (v >= m_min) && (v <= m_max);
        }

        _INLINE bool encloseAny(const RTVec* pts, int npts) const {
            for (int i = 0; i < npts; i++)
                if (enclose(pts[i])) return true;
            return false;
        }

        _INLINE bool encloseAll(const RTVec* pts, int npts) const {
            for (int i = 0; i < npts; i++)
                if (!enclose(pts[i])) return false;
            return true;
        }

        /// point v is strictly inside this box
        _INLINE bool isStrictlyEnclose(const RTVec& v) const {
            return (v > m_min) && (v < m_max);
        }
        /// point v is strictly inside this extended box
        _INLINE bool isStrictlyEnclose(const RTVec& v, DataType off) const {
            RTVec shift(off);
            return (v > m_min + shift) && (v < m_max - shift);
        }

        /// this box is occluded by b
        _INLINE bool isInside(const RTBox_t& b) const {
            return (m_min >= b.m_min) && (m_max <= b.m_max);
        }

        /// this box is completely occluded by b
        _INLINE bool isStrictlyInside(const RTBox_t& b) const {
            return (m_min > b.m_min) && (m_max < b.m_max);
        }
        /// this box is completely occluded by extended b
        _INLINE bool isStrictlyInside(const RTBox_t& b, DataType off) const {
            RTVec shift(off);
            return (m_min > b.m_min - shift) && (m_max < b.m_max + shift);
        }

        _INLINE void enlarge(DataType f) {
            m_max += f;
            m_min -= f;
        }

        _INLINE RTVec sides() const {
            return m_max - m_min;
        }
        _INLINE RTVec diameter() const {
            return m_max - m_min;
        }

        _INLINE RTVec center() const {
            return 0.5f*(m_max + m_min);
        }

        // Operators.

        _INLINE bool operator==(const RTBox_t& b) const {
            return m_min == b.m_min && m_max == b.m_max;
        }

        _INLINE bool operator!=(const RTBox_t& b) const    {
            return m_min != b.m_min || m_max != b.m_max;
        }

        _INLINE const RTVec& operator[](const int i) const {
            assert(i >= 0 && i < 2);
            return *(&m_min + i);
        }

        _INLINE RTVec& operator[](const int i) {
            assert(i >= 0 && i < 2);
            return *(&m_min + i);
        }

        _INLINE const RTBox_t& operator=(const RTBox_t& b) {
            m_min = b.m_min;
            m_max = b.m_max;
            return *this;
        }

        /// Return union.
        _INLINE RTBox_t operator+(const RTBox_t& b) const {
            RTBox_t r = *this;
            r.m_min.setMin(b.m_min);
            r.m_max.setMax(b.m_max);
            return r;
        }

        /// return intersection -- could result in invalid box
        _INLINE RTBox_t operator-(const RTBox_t& b) const {
            RTBox_t r = *this;
            r.m_min.setMax(b.m_min);
            r.m_max.setMin(b.m_max);
            return r;
        }

        _INLINE RTBox_t operator*(DataType f) const {
            return RTBox_t(m_min*f, m_max*f);
        }

        /// set *this to union
        _INLINE void operator+=(const RTBox_t& b) {
            extend(b);
        }

        /// set *this to intersection -- could result in invalid box
        _INLINE void operator-=(const RTBox_t& b) {
            m_min.setMax(b.m_min);
            m_max.setMin(b.m_max);
        }

        _INLINE const RTBox_t& operator*=(DataType f) {
            m_min *= f;
            m_max *= f;
            return *this;
        }

        _INLINE const RTBox_t& operator/=(DataType f) {
            m_min /= f;
            m_max /= f;
            return *this;
        }

        /// not valid for floats
        _INLINE const RTBox_t& operator%=(DataType f) {
            m_min %= f;
            m_max %= f;
            return *this;
        }

        RTVec m_min;
        RTVec m_max;
    };

    /// Generic reset and its specialization.
    template<int N, typename DataType, int align>
    void RTBox_t<N, DataType, align>::reset() {
        // Invalidate the current box.
        m_min = RTVec_t<N, DataType, align>( RTVec_t<N, DataType, align>::infinity());
        m_max = RTVec_t<N, DataType, align>(-RTVec_t<N, DataType, align>::infinity());
    }

    template<>
    _INLINE void RTBox_t<1,sse_f>::reset() {
        // Invalidate the current box.
        m_min = _mm_set_ps1(+FLT_MAX);
        m_max = _mm_set_ps1(-FLT_MAX);
    }

    template<int N, typename DataType, int align>
    _INLINE RTBox_t<N, DataType, align> operator*(DataType f, const RTBox_t<N, DataType, align>& q) {
        return q * f;
    }

    template<int N, typename DataType, int align>
    _INLINE ostream& operator<<(ostream& out, const RTBox_t<N, DataType, align>& b) {
        out << "[" << b.m_min << "," << b.m_max << "]";
        return out;
    }

#if 1
    /// SSE implementation of 3D box of floats.
    class RTBox3a : public RTBox_t<1, sse_f> {
    public:
        _INLINE sse_f center()   const { return *RTBox_t<1, sse_f>::center(); }
        _INLINE sse_f diameter() const { return *RTBox_t<1, sse_f>::diameter(); }

        #ifdef  RT_EMULATE_SSE
        // No alignment in emulation mode -- has to create 3D boxes to compute area/volume.
        _INLINE float volume() const {
            return RTBox_t<3, float, 0>(min3f(), max3f()).volume();
        }
        _INLINE float area() const {
            return RTBox_t<3, float, 0>(min3f(), max3f()).area();
        }
        #else
        /// Treat this as aligned box of 3D vectors (which it is really is).
        _INLINE float volume() const {
            return ((RTBox_t<3, float, 16>*)this)->volume();
        }
        _INLINE float area() const {
            float a3 = ((RTBox_t<3, float, 16>*)this)->area();
            //float a4 = ((RTBox_t<4, float, 16>*)this)->area();
            //cout << a3 << "\t" << a4 << endl;
            return a3;
        }
        #endif
      _INLINE RTVec3f& min3f() const {
            return (RTVec3f&)m_min;
      }
      _INLINE RTVec3f& max3f() const {
            return (RTVec3f&)m_max;
      }

      _INLINE RTVec3f& operator[](const int i) {
          assert(i >= 0 && i < 2);
          return (i == 0) ? *(RTVec3f*)&m_min[0] : *(RTVec3f*)&m_max[0];
      }

      _INLINE sse_f &min_f() { return m_min[0]; }
      _INLINE sse_f &max_f() { return m_max[0]; }
      _INLINE sse_f min_f() const { return m_min[0]; }
      _INLINE sse_f max_f() const { return m_max[0]; }

    };
#else
    class RTBox3a : public RTBox_t<1, RTVec3f, RTData_t<1, RTVec3f, 16> > {
  public:

    RTBox3a() {}

    RTBox3a(const sse_f& _min, const sse_f& _max) {
      min_f() = _min;
      max_f() = _max;
    }

    _INLINE RTVec3f& operator[](const int i) {
      assert(i >= 0 && i < 2);
      return (i == 0) ? m_min[0] : m_max[0];
    }

    _INLINE void extend(const sse_f &v) {
      min_f() = min(min_f(),v);
      max_f() = max(max_f(),v);
    }

    _INLINE void extend(const RTBox3a &box) {
      min_f() = min(min_f(),box.min_f());
      max_f() = max(max_f(),box.max_f());
    }

    _INLINE sse_f center() const {
      return 0.5f*(min_f() + max_f());
    }

    _INLINE sse_f diameter() const {
      return max_f() - min_f();
    }

    _INLINE void setEmpty() {
      min_f() = _mm_set_ps1(+FLT_MAX);
      max_f() = _mm_set_ps1(-FLT_MAX);
    }

    _INLINE sse_f &min_f() { return *(sse_f*)&m_min; }
    _INLINE sse_f &max_f() { return *(sse_f*)&m_max; }
    _INLINE sse_f &min_f() const { return *(sse_f*)&m_min; }
    _INLINE sse_f &max_f() const { return *(sse_f*)&m_max; }

    /// Treat this as aligned box of 3D vectors (which it is really is).
    _INLINE float volume() const {
        return ((RTBox_t<3, float, RTData_t<3, float, 16> >*)this)->volume();
    }

    _INLINE float area() const {
        return ((RTBox_t<3, float, RTData_t<3, float, 16> >*)this)->area();
    }

  };

#endif

};

typedef RTTL::RTBox_t<2, float> RTBox2f;
typedef RTTL::RTBox_t<3, float> RTBox3f;
typedef RTTL::RTBox_t<4, float> RTBox4f;
typedef RTTL::RTBox_t<2, int>   RTBox2i;
typedef RTTL::RTBox_t<3, int>   RTBox3i;
typedef RTTL::RTBox_t<4, int>   RTBox4i;
typedef RTTL::RTBox3a RTBoxSSE;


#endif
