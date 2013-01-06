// Synonyms for SSE intrinsics with
// some defines to make SSE code more readable... Everything that ends
// with a '4' is supposed to work on SSE 4-floats ...

#define or4 _mm_or_ps
#define or4i _mm_or_si128
#define and4 _mm_and_ps
#define and4i _mm_and_si128
#define andnot4 _mm_andnot_ps
#define andnot4i _mm_andnot_si128
#define xor4 _mm_xor_ps
#define xor4i _mm_xor_si128
#define mul4 _mm_mul_ps
#define add4 _mm_add_ps
#define add4i _mm_add_epi32
#define sub4 _mm_sub_ps
#define min4 _mm_min_ps
#define max4 _mm_max_ps
#define set4 _mm_set_ps1
#define set44 _mm_set_ps
#define set4i _mm_set1_epi32
#define set44i _mm_set_epi32
#define set4l _mm_set1_epi64x // See comments below
#define unpacklo _mm_unpacklo_ps // (a,b) => [a0, b0, a1, b1]
#define unpackhi _mm_unpackhi_ps // (a,b) => [a2, b2, a3, b3]
#define zero4 _mm_setzero_ps
#define zero4i _mm_setzero_si128
#define getmask4 _mm_movemask_ps
#define maskmove4i _mm_maskmoveu_si128 // destination need not be aligned
#define cmp4_ge _mm_cmpge_ps
#define cmp4_le _mm_cmple_ps
#define cmp4_gt _mm_cmpgt_ps
#define cmp4_lt _mm_cmplt_ps
#define cmp4_eq _mm_cmpeq_ps
#define cmp4_eq_i _mm_cmpeq_epi32
#define load44 _mm_load_ps
#define load44i _mm_load_si128
#define store44 _mm_store_ps
#define store44i _mm_store_si128
#define shuffle4 _mm_shuffle_ps
#define shuffle4i _mm_shuffle_epi32

// The cast_x2y are more like reinterpret casts.  For convertions of
// types use the convert4 functions below.
#define convert4_f2i _mm_cvttps_epi32
#define convert4_i2f _mm_cvtepi32_ps
#define sqrt4 _mm_sqrt_ps
#define shift_right4int _mm_srli_epi32 // >>
#define shift_left4int _mm_slli_epi32  // <<

#define blend4  _mm_blendv_ps
#define blend4i _mm_blendv_si128
