#ifndef __RTSOFTSSE_H__
#define __RTSOFTSSE_H__

#ifdef _MSC_VER
#pragma warning(disable: 4244)
#pragma warning(disable: 4311)
#endif
#ifdef __INTEL_COMPILER
#pragma warning(disable:1684)
#ifndef _WIN32
#pragma warning(disable:810)
#endif
#endif

#ifdef VECTOR_SIZE
#error macro conflict found in __FILE__ << ":" << __LINE__
#endif
#define VECTOR_SIZE 4

typedef struct sse_f {
	sse_f()	{}

	float f[VECTOR_SIZE];
} sse_f;

typedef struct sse_i64 {
	union {
	  char b[2*VECTOR_SIZE];
	  signed short wd[VECTOR_SIZE];
          signed int dw[VECTOR_SIZE>>1];
	}__sse_i64;
} sse_i64;

typedef struct sse_i {
	sse_i() {}
	union {
	  signed char b[4*VECTOR_SIZE];
	  signed int i[VECTOR_SIZE];
	  unsigned int ui[VECTOR_SIZE];
          sse_i64 l[VECTOR_SIZE>>1];
	} __sse_i;
} sse_i;



#define _MM_ZERO 0.0f
#define _MM_ONE  1.0f

#define _MM_FP(x,n) (x.f[n])
#define _MM_INT(x,n) (*((int *)(&(x.f[n]))))

#define _MM_BITONE  (0xffffffff)
#define _MM_BITZERO (0x00000000)

#define _MM_MIN(a,b) (((a) < (b)) ? (a) : (b))
#define _MM_MAX(a,b) (((a) > (b)) ? (a) : (b))


#define APPROXIMATE_ROUNDUP_MODES
#ifdef  APPROXIMATE_ROUNDUP_MODES

// Approximate roundup functions
// (will not provide exact SSE functionality, but works faster).

_INLINE static int _emm_round_nearest(double d) {
    return (int)d;
}

_INLINE static int _emm_round_trunc(double d) {
    return (int)d;
}

_INLINE static int _mm_round_trunc(float f) {
    return (int)f;
}

_INLINE static double _mminternal_sqrt(double src) {
    return (float)sqrt(src);
}

#else

// Exact roundup functions.

_INLINE static int _emm_round_nearest(double d) {
	int result;
	__asm	{
		fld d
            fistp result
    }
	return result;
}

_INLINE static int _emm_round_trunc(double d) {
	int result;
	int saved_cw;
	int new_cw;
	__asm	{
		push      eax
            fld       d
            fstcw     saved_cw
            mov       eax, saved_cw
            or        eax, 3072
            mov       new_cw, eax
            fldcw     new_cw
            fistp     result
            fldcw     saved_cw
            pop       eax
    }
	return result;
}

_INLINE static int _mm_round_trunc(float f) {
	int result;
	int saved_cw;
	int new_cw;
	__asm	{
		push      eax
            fld       f
            fstcw     saved_cw
            mov       eax, saved_cw
            or        eax, 3072
            mov       new_cw, eax
            fldcw     new_cw
            fistp     result
            fldcw     saved_cw
            pop       eax
    }
	return result;
}

// This is helper function for getting sqrt - Port from ICC9.1
_INLINE static double _mminternal_sqrt(double src) {
    double result;
	_asm	{
		fld QWORD PTR src
            fsqrt
            fstp result;
	}

    return result;
}
#endif

#ifndef _MM_NO_ABORT
#define _mminternal_abort(str)                                          \
    { fprintf(stderr, "*** Functionality intrinsics error: %s ***\n", str); \
        exit(1); }
#else
#define _mminternal_abort(str)                                          \
    { fprintf(stderr, "*** Functionality intrinsics warning: %s ***\n", str); }
#endif

#ifndef _MM_NO_ALIGN_CHECK
#define _mminternal_assert_16B(addr)                            \
    if ((unsigned long long int)addr % 16 != 0) {                             \
        _mminternal_abort("address must be 16-byte aligned");   \
    }
#else
#define _mminternal_assert_16B(addr) ;
#endif

_INLINE static sse_i _mm_add_epi32(sse_i a, sse_i b) {
	sse_i result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.__sse_i.i[i] = a.__sse_i.i[i] +  b.__sse_i.i[i];
	return result;
}

_INLINE static sse_f _mm_add_ps(sse_f a, sse_f b) {
	sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.f[i] = a.f[i] + b.f[i];

	return result;
}

_INLINE static sse_f _mm_add_ss(sse_f a, sse_f b) {
	sse_f result;

	result.f[0] = a.f[0] + b.f[0];
    //#pragma unroll(3)
    for (int i = 1; i < VECTOR_SIZE; i++)
        result.f[i] = a.f[i]; 

	return result;
}

_INLINE static sse_f _mm_and_ps(sse_f a, sse_f b) {
	sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_INT(result,i) = _MM_INT(a,i) & _MM_INT(b,i);

	return result;
}

_INLINE static sse_i _mm_and_si128(sse_i a, sse_i b) {
	sse_i result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.__sse_i.i[i] = a.__sse_i.i[i] & b.__sse_i.i[i];

	return result;
}

_INLINE static sse_f _mm_andnot_ps(sse_f a, sse_f b) {
	sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_INT(result,i) = (~_MM_INT(a,i)) & _MM_INT(b,i);

	return result;
}

_INLINE static sse_i _mm_andnot_si128(sse_i a, sse_i b) {
	sse_i result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.__sse_i.i[i] = (~a.__sse_i.i[i]) & b.__sse_i.i[i];

	return result;
}

_INLINE static sse_i _mm_cmpeq_epi32(sse_i a, sse_i b) {
	sse_i result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.__sse_i.i[i] = (a.__sse_i.i[i] == b.__sse_i.i[i]) ? 0xffffffff: 0x00000000;

	return result;
}


_INLINE static sse_f _mm_cmpeq_ps(sse_f a, sse_f b) {
	sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_INT(result,i) = (_MM_INT(a,i) == _MM_INT(b,i)) ? _MM_BITONE : _MM_BITZERO;

	return result;
}

_INLINE static sse_f _mm_cmpge_ps(sse_f a, sse_f b) {
	sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_INT(result,i) = (_MM_FP(a,i) >= _MM_FP(b,i)) ? _MM_BITONE : _MM_BITZERO;

	return result;
}

_INLINE static sse_f _mm_cmpgt_ps(sse_f a, sse_f b) {
	sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_INT(result,i) = (_MM_FP(a,i) > _MM_FP(b,i)) ? _MM_BITONE : _MM_BITZERO;

	return result;
}

_INLINE static sse_f _mm_cmple_ps(sse_f a, sse_f b) {
	sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_INT(result,i) = (_MM_FP(a,i) <= _MM_FP(b,i)) ? _MM_BITONE : _MM_BITZERO;

	return result;
}

_INLINE static sse_i _mm_cmplt_epi32(sse_i a, sse_i b) {
    sse_i result;
    int i;

    //#pragma unroll(4)
    for (i=0;i<VECTOR_SIZE;i++)
        result.__sse_i.i[i] = (a.__sse_i.i[i] < b.__sse_i.i[i]) ? 0xffffffff: 0x0;

    return result;
}

_INLINE static sse_i _mm_cmpgt_epi32(sse_i a, sse_i b) {
    sse_i result;
    int i;

    //#pragma unroll(4)
    for (i=0;i<VECTOR_SIZE;i++)
        result.__sse_i.i[i] = (a.__sse_i.i[i] > b.__sse_i.i[i]) ? 0xffffffff: 0x0;

    return result;
}

_INLINE static sse_f _mm_cmplt_ps(sse_f a, sse_f b) {
	sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_INT(result,i) = (_MM_FP(a,i) < _MM_FP(b,i)) ? _MM_BITONE : _MM_BITZERO;

	return result;
}

_INLINE static sse_f _mm_cmple_ss(sse_f a, sse_f b) {
	sse_f result;

	_MM_INT(result,0) = (_MM_FP(a,0) <= _MM_FP(b,0)) ? _MM_BITONE : _MM_BITZERO;
    //#pragma unroll(3)
    for (int i = 1; i < VECTOR_SIZE; i++)
        _MM_FP(result,i) = _MM_FP(a,i);

	return result;
}

_INLINE static sse_f  _mm_cmpneq_ps(sse_f a, sse_f b) {
	sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_INT(result,i) = (!(_MM_FP(a,i) == _MM_FP(b,i))) ? _MM_BITONE : _MM_BITZERO;

	return result;
}

_INLINE static sse_f _mm_cvtepi32_ps(sse_i a) {
	sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_FP(result,i) = (float)(a.__sse_i.i[i]);

	return result;
}

_INLINE static sse_i _mm_cvtps_epi32(sse_f a) {
	sse_i result;
	sse_f temp;
	temp = a;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.__sse_i.i[i] =  _emm_round_nearest(_MM_FP(temp,i));

	return result;
}

_INLINE static sse_f _mm_cvtsi32_ss(sse_f a, int b) {
	sse_f result;

	_MM_FP(result,0) = (float) b;
    //#pragma unroll(3)
    for (int i = 1; i < VECTOR_SIZE; i++)
        _MM_INT(result,i) = _MM_INT(a,i);

	return result;
}

_INLINE static sse_i _mm_cvttps_epi32(sse_f a) {
	sse_i result;
	sse_f temp;
	temp = a;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.__sse_i.i[i] = _emm_round_trunc(_MM_FP(temp,i));

	return result;
}


_INLINE static sse_i64 _mm_cvtps_pi32(sse_f a) {
    sse_i64 result;

    result.__sse_i64.dw[0] = _emm_round_nearest(_MM_FP(a,0) );
    result.__sse_i64.dw[1] = _emm_round_nearest(_MM_FP(a,1) );

    return result;
}

_INLINE static int _mm_cvttss_si32(sse_f a) {
	int result;

	result = _mm_round_trunc ( _MM_FP(a,0) );

	return result;
}

_INLINE static sse_f _mm_div_ps(sse_f a, sse_f b) {
    sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_FP(result,i) = _MM_FP(a,i) / _MM_FP(b,i);

    return result;
}


_INLINE static sse_f _mm_load_ps(float const *a) {
	sse_f result;

	_mminternal_assert_16B(a);
    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_FP(result,i) = a[i];

	return result;
}




_INLINE static sse_i _mm_load_si128(sse_i *p) {
	sse_i result;

	_mminternal_assert_16B(p);
	result = *p;

	return result;
}

_INLINE static void _mm_maskmoveu_si128(sse_i d, sse_i n, char *p) {
	int i;
	for(i=0;i<16;i++) {
		if(n.__sse_i.b[i] & 0x80) 
			p[i] = d.__sse_i.b[i];
	}

}

_INLINE static sse_f _mm_max_ps(sse_f a, sse_f b) {
	sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_FP(result,i) = _MM_MAX(_MM_FP(a,i), _MM_FP(b,i));

	return result;
}

_INLINE static sse_f _mm_min_ps(sse_f a, sse_f b) {
	sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_FP(result,i) = _MM_MIN(_MM_FP(a,i), _MM_FP(b,i));

	return result;
}

_INLINE static sse_f _mm_movehl_ps(sse_f a, sse_f b) {
    sse_f result;

    _MM_FP(result,0) = _MM_FP(b,2);
    _MM_FP(result,1) = _MM_FP(b,3);
    _MM_FP(result,2) = _MM_FP(a,2);
    _MM_FP(result,3) = _MM_FP(a,3);

    return result;
}



_INLINE static int _mm_movemask_epi8(sse_i a) {
	int result;

	result = ((((a.__sse_i.b[0])>>7)&0x1) | (((a.__sse_i.b[1])>>6)&0x2) |
              (((a.__sse_i.b[2])>>5)&0x4) | (((a.__sse_i.b[3])>>4)&0x8) |
              (((a.__sse_i.b[4])>>3)&0x10) | (((a.__sse_i.b[5])>>2)&0x20) |
              (((a.__sse_i.b[6])>>1)&0x40) | (((a.__sse_i.b[7])>>4)&0x80) |
              (((a.__sse_i.b[8])<<1)&0x100) | (((a.__sse_i.b[9])<<2)&0x200) |
              (((a.__sse_i.b[10])<<3)&0x400) | (((a.__sse_i.b[11])<<4)&0x800) |
              (((a.__sse_i.b[12])<<5)&0x1000) | (((a.__sse_i.b[13])<<6)&0x2000) |
              (((a.__sse_i.b[14])<<7)&0x4000) | (((a.__sse_i.b[15])<<8)&0x8000));

	return result;
}

_INLINE static int _mm_movemask_ps(sse_f a) {
	int result;

	result = (((_MM_INT(a,0)>>31)&0x1) | ((_MM_INT(a,1)>>30)&0x2) |
              ((_MM_INT(a,2)>>29)&0x4) | ((_MM_INT(a,3)>>28)&0x8));

	return result;
}

_INLINE static sse_f _mm_mul_ss(sse_f a, sse_f b) {
    sse_f result;

    _MM_FP(result,0) = _MM_FP(a,0) * _MM_FP(b,0);
    //#pragma unroll(3)
    for (int i = 1; i < VECTOR_SIZE; i++)
        _MM_FP(result,i) = _MM_FP(a,i);

    return result;
}

_INLINE static sse_f _mm_mul_ps(sse_f a, sse_f b) {
    sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_FP(result,i) = _MM_FP(a,i) * _MM_FP(b,i);

    return result;
}

/////////////////////////////////////////////////////////////////////////////
// This is helper function for getting approx value - Port from ICC9.1
_INLINE static float _mminternal_approx(float x) {
	unsigned int *p = (unsigned int*)&x;
	*p = *p & 0xfffff800;
	return x;
}

_INLINE static sse_f _mm_or_ps(sse_f a, sse_f b) {
    sse_f result;
  
    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_INT(result,i) = _MM_INT(a,i) | _MM_INT(b,i);

    return result;
}

_INLINE static sse_i _mm_or_si128(sse_i a, sse_i b) {
    sse_i result;
  
    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.__sse_i.i[i] = a.__sse_i.i[i] | b.__sse_i.i[i];
  
    return result;
}

// TODO
// _mm_packs_pi32

_INLINE static sse_f _mm_rcp_ps(sse_f a) {
    sse_f result;
  
    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        _MM_FP(result,i) = _mminternal_approx(_MM_ONE / _MM_FP(a,i));
    return result;
}

_INLINE static sse_f _mm_rcp_ss(sse_f a) {
    sse_f result;

    result.f[0] = _mminternal_approx( _MM_ONE / (a.f[0]) );
    //#pragma unroll(3)
    for (int i = 1; i < VECTOR_SIZE; i++)
        result.f[i] = a.f[i];

    return result;
}
//FIXME - How to get sqrt efficient
_INLINE static sse_f _mm_rsqrt_ss(sse_f a) {
    sse_f result;

    result.f[0] = _mminternal_approx(1.0f / _mminternal_sqrt(a.f[0]));
    //#pragma unroll(3)
    for (int i = 1; i < VECTOR_SIZE; i++)
        result.f[i] = a.f[i];

    return result;
}

_INLINE static sse_f _mm_rsqrt_ps(sse_f a) {
    sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.f[i] = _mminternal_approx(1.0f / _mminternal_sqrt(a.f[i]));

    return result;
}

_INLINE static sse_i _mm_set1_epi32(int i) {
    sse_i result;
    result.__sse_i.i[3] = result.__sse_i.i[2] = result.__sse_i.i[1] = result.__sse_i.i[0] = i;

    return result;
}

_INLINE static sse_i _mm_set_epi32(int i3, int i2, int i1, int i0) {
    sse_i result;
    result.__sse_i.i[0] = i0;
    result.__sse_i.i[1] = i1;
    result.__sse_i.i[2] = i2;
    result.__sse_i.i[3] = i3;

    return result;
}


_INLINE static sse_f _mm_set_ps(float a, float b, float c, float d) {
    sse_f result;

    result.f[0] = a;
    result.f[1] = b;
    result.f[2] = c;
    result.f[3] = d;

    return result;
}

_INLINE static sse_f _mm_set_ps1(float a) {
    sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.f[i] = a;

    return result;
}

_INLINE static sse_f _mm_set_ss(float a) {
    sse_f result;

    result.f[0] = a;
    //#pragma unroll(3)
    for (int i = 1; i < VECTOR_SIZE; i++)
        result.f[i] = 0.0f;

    return result;
}

_INLINE static sse_i _mm_setr_epi32(int i0, int i1, int i2, int i3) {
    sse_i result;
    result.__sse_i.i[0] = i0;
    result.__sse_i.i[1] = i1;
    result.__sse_i.i[2] = i2;
    result.__sse_i.i[3] = i3;

    return result;
}

_INLINE static sse_f _mm_setr_ps(float a, float b, float c, float d) {
    sse_f result;

    result.f[0] = a;
    result.f[1] = b;
    result.f[2] = c;
    result.f[3] = d;

    return result;
}

_INLINE static sse_f _mm_setzero_ps(void) {
    sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.f[i] = 0.0;
    return result;
}

_INLINE static sse_i _mm_slli_epi32(sse_i a, int count) {
	sse_i result;
	int i;

	if (count > 31){
    //#pragma unroll(4)
        for (int i = 0; i < VECTOR_SIZE; i++)
            result.__sse_i.i[i]=0;
	} else {
    //#pragma unroll(4)
		for (i=0;i<VECTOR_SIZE;i++)
			result.__sse_i.i[i] = a.__sse_i.i[i] << count;
	}
	return result;
}

_INLINE static sse_i _mm_shuffle_epi32(sse_i a, int imm8) {
    sse_i result;

    int t;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++) {
        t =  ((imm8 >> 2*i) & 0x3) ;
        result.__sse_i.i[i] = (t==0 ? a.__sse_i.i[0] :
                              (t==1 ? a.__sse_i.i[1] :
                              (t==2 ? a.__sse_i.i[2] : a.__sse_i.i[3])));
    }

    return result;
}


_INLINE static sse_f _mm_shuffle_ps(sse_f a, sse_f b, unsigned int imm8) {
    sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.f[i] = b.f[ ((imm8 >> 2*i) & 0x3) ];
	
    return result;
}

_INLINE static void _mm_stream_ps(float *a, sse_f b) {
	_mminternal_assert_16B(a);
	*(sse_f*)a = b;
}

_INLINE static sse_i _mm_srli_epi32(sse_i a, int count) {
	sse_i result;
	int i;

	if (count > 31) {
    //#pragma unroll(4)
        for (int i = 0; i < VECTOR_SIZE; i++)
            result.__sse_i.ui[i] = 0;
	} else {
    //#pragma unroll(4)
		for (i=0;i<VECTOR_SIZE;i++)
			result.__sse_i.ui[i] = a.__sse_i.ui[i] >> count;
	}

	return result;
}


_INLINE static sse_f _mm_sqrt_ps(sse_f a) {
    sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.f[i] = _mminternal_sqrt(a.f[i]);

    return result;
}

_INLINE static void _mm_store_ps(float *v, sse_f a) {
    //FIXME address must be 16-byte aligned
    //TODO add assert ( v % 16 != 0 )
    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        v[i] = a.f[i];
}

_INLINE static sse_i _mm_sub_epi32(sse_i a, sse_i b) {
    sse_i result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.__sse_i.i[i] = a.__sse_i.i[i] - b.__sse_i.i[i];

    return result;
}


_INLINE static sse_f _mm_sub_ss(sse_f a, sse_f b) {
    sse_f result;

    result.f[0] = a.f[0] - b.f[0];
    //#pragma unroll(3)
    for (int i = 1; i < VECTOR_SIZE; i++)
        result.f[i] = a.f[i];

    return result;
}

_INLINE static sse_f _mm_sub_ps(sse_f a, sse_f b) {
    sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.f[i] = a.f[i] - b.f[i];

    return result;
}

_INLINE static sse_f _mm_unpackhi_ps(sse_f a, sse_f b) {
    sse_f result;

    result.f[0] = a.f[2];
    result.f[1] = b.f[2];
    result.f[2] = a.f[3];
    result.f[3] = b.f[3];

    return result;
}

_INLINE static sse_f _mm_unpacklo_ps(sse_f a, sse_f b) {
    sse_f result;

    result.f[0] = a.f[0];
    result.f[1] = b.f[0];
    result.f[2] = a.f[1];
    result.f[3] = b.f[1];

    return result;
}

//FIXME - Type cast might be wrong
_INLINE static sse_f _mm_xor_ps(sse_f a, sse_f b) {
    sse_f result;

    //#pragma unroll(4)
    for (int i = 0; i < VECTOR_SIZE; i++)
        result.f[i] = ( ((int)a.f[i]) ^ ((int)b.f[i]) );

    return result;
}

_INLINE static sse_i64 _mm_packs_pi32(sse_i64 a, sse_i64 b) {
    sse_i64 result;
    //FIXME signed ? how to deal with that????
    result.__sse_i64.wd[0] = a.__sse_i64.dw[0];
    result.__sse_i64.wd[1] = a.__sse_i64.dw[1];
  
    result.__sse_i64.wd[2] = b.__sse_i64.dw[0];
    result.__sse_i64.wd[3] = b.__sse_i64.dw[1];

    return result;  
}

_INLINE static sse_i64 _mm_setzero_si64() {
    sse_i64 result;
    result.__sse_i64.dw[1] = result.__sse_i64.dw[0] = 0;

    return result;
}

_INLINE static sse_i64 _mm_unpacklo_pi16(sse_i64 a, sse_i64 b) {
    sse_i64 result;
    result.__sse_i64.wd[0] = a.__sse_i64.wd[0];
    result.__sse_i64.wd[1] = b.__sse_i64.wd[0];
    result.__sse_i64.wd[2] = a.__sse_i64.wd[1];
    result.__sse_i64.wd[3] = b.__sse_i64.wd[1];

    return result;
}

_INLINE static sse_i64 _mm_unpackhi_pi16(sse_i64 a, sse_i64 b) {
    sse_i64 result;
    result.__sse_i64.wd[0] = a.__sse_i64.wd[2];
    result.__sse_i64.wd[1] = b.__sse_i64.wd[2];
    result.__sse_i64.wd[2] = a.__sse_i64.wd[3];
    result.__sse_i64.wd[3] = b.__sse_i64.wd[3];

    return result;
}


_INLINE static sse_i64 _mm_unpacklo_pi32(sse_i64 a, sse_i64 b) {
    sse_i64 result;
    result.__sse_i64.dw[0] = a.__sse_i64.dw[0];
    result.__sse_i64.dw[1] = b.__sse_i64.dw[0];

    return result;
}

_INLINE static sse_i64 _mm_unpackhi_pi32(sse_i64 a, sse_i64 b) {
    sse_i64 result;
    result.__sse_i64.dw[0] = a.__sse_i64.dw[1];
    result.__sse_i64.dw[1] = b.__sse_i64.dw[1];

    return result;
}

_INLINE static sse_i64 _mm_packs_pu16(sse_i64 a, sse_i64 b) {
    sse_i64 result;
    result.__sse_i64.b[0] = a.__sse_i64.wd[0];
    result.__sse_i64.b[1] = a.__sse_i64.wd[1];
    result.__sse_i64.b[2] = a.__sse_i64.wd[2];
    result.__sse_i64.b[3] = a.__sse_i64.wd[3];

    result.__sse_i64.b[4] = b.__sse_i64.wd[0];
    result.__sse_i64.b[5] = b.__sse_i64.wd[1];
    result.__sse_i64.b[6] = b.__sse_i64.wd[2];
    result.__sse_i64.b[7] = b.__sse_i64.wd[3];

    return result;
}

_INLINE static sse_i _mm_setr_epi64(sse_i64 a, sse_i64 b) {
    sse_i result;

    result.__sse_i.l[0] = a;
    result.__sse_i.l[1] = b;

    return result;
}

_INLINE static unsigned int _mm_getcsr() {
    //FIXME
    return 0;
}

_INLINE static void _mm_setcsr(unsigned int v) {
    //FIXME
    return;
}

/////////////////////////////////////////////////////////////////////////////

#define _mm_cvtps_pi16(a)                                   \
    _mm_packs_pi32(_mm_cvtps_pi32(a),                       \
                   _mm_cvtps_pi32(_mm_movehl_ps((a), (a))))


#undef VECTOR_SIZE

#endif
