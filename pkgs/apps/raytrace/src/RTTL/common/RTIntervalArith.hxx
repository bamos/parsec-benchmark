#ifndef RTTL_INTERVAL_ARITH_HXX
#define RTTL_INTERVAL_ARITH_HXX

#include "RTVec.hxx"

namespace RTTL {
    
    class RTIntervalVec {
    public:
        
        sse_f m_min;
        sse_f m_max;
        
        RTIntervalVec() {}
        
        RTIntervalVec(const sse_f& _min, const sse_f& _max)
        {
            m_min = _min;
            m_max = _max;
        }
        
        _INLINE RTIntervalVec &operator=(const RTIntervalVec &v)
        {
            m_min = v.m_min;
            m_max = v.m_max;
            return *this;
        };
        
        _INLINE bool empty() const
        {
            return minHorizontal3f(m_min) > maxHorizontal3f(m_max);
        }
        
    };
    
    _INLINE RTIntervalVec inverse(const RTIntervalVec &v)
    {
        const sse_f m_min = rcp(v.m_min);
        const sse_f m_max = rcp(v.m_min);
        return RTIntervalVec(m_min,m_max);
    }
    
    _INLINE RTIntervalVec operator+(const RTIntervalVec &a,const RTIntervalVec &b)
    {
        return RTIntervalVec(a.m_min+b.m_min,a.m_max+b.m_max);
    }
    
    _INLINE RTIntervalVec operator-(const RTIntervalVec &a,const RTIntervalVec &b)
    {
        return RTIntervalVec(a.m_min+b.m_max,a.m_max+b.m_min);
    }
    
    _INLINE RTIntervalVec operator*(const RTIntervalVec &a,const RTIntervalVec &b)
    {
        const sse_f t0 = a.m_min * b.m_min;
        const sse_f t1 = a.m_min * b.m_max;
        const sse_f t2 = a.m_max * b.m_min;
        const sse_f t3 = a.m_max * b.m_max;
        return RTIntervalVec(min(min(t0,t1),min(t2,t3)),max(max(t0,t1),max(t2,t3)));
    }
    
};

#endif
