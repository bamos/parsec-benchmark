#include <time.h>

#include "RTTL/common/RTBox.hxx"
#include "RTTL/common/Timer.hxx"
using namespace RTTL;

template<int N, typename DataType>
void randomize(RTVec_t<N, DataType>& v, float scale = 1) {
    for (int i = 0; i < v.nElements(); i++)
        v[i] = DataType(scale * rand()/float(RAND_MAX));
}

template<int N>
void randomize(RTVec_t<N, sse_f>& v, float scale = 1) {
    float* f = (float*)&v;
    for (int i = 0; i < 4*v.nElements(); i++)
        f[i] = float(scale * rand()/float(RAND_MAX));
}

template<int N>
void randomize(RTVec_t<N, sse_i>& v, float scale = 1) {
    int* f = (int*)&v;
    for (int i = 0; i < 4*v.nElements(); i++)
        f[i] = int(scale * rand()/float(RAND_MAX));
}

// Elementary types
template<int N, typename DataType>
void test() {
    typedef RTVec_t<N, DataType> rtvec;
    DataType eps = epsilon<DataType>();

    rtvec a(1);
    rtvec b(2);
    rtvec c(3);
    rtvec d(4);

    DataType q = 14;
    rtvec dif = q*a - b - c*d; // should be ~0
    if (dif.absMaximum() != 0) {
        cout << "err101 = " << dif << ";" << endl;
        //exit(101);
    }

    c += 1; // operator+=(const DataType q)
    if (d != c) {
        cout << "err102 = " << c << ";" << endl;
        //exit(102);
    }

    const float scale = 100;
    randomize(a, scale);
    randomize(b, scale);
    randomize(c, scale);

    d = (a+b)*c - (a*c + b*c);
    if (d.absMaximum() > 30 * N * scale * eps) {
        // Could happen (rather infrequently but still)
        cout << "err103 = " << d << ";" << endl;
        //exit(103);
    }

    DataType e[] = {8,1,2,3,4,5,6,7,8,9,10};
    a = 2;
    a *= 2;
    a -= e;
rtvec tt(e);
    a += rtvec(5);
    if (a.maximum() != ((rtvec&)e).maximum() || a.minimum() != ((rtvec&)e).minimum()) {
        cout << "err104 = " << a << ";" << endl;
        //exit(104);
    }

}

// SSE types (also works for int/floats!)
template<int N, typename DataType>
void test4() {
    typedef RTVec_t<N, DataType> rtvec;
    DataType eps = epsilon<DataType>();

    rtvec a(convert<DataType>(1));
    rtvec b(convert<DataType>(2));
    rtvec c(convert<DataType>(3));
    rtvec d(convert<DataType>(4));

    DataType q = convert<DataType>(14);
    rtvec dif = q*a - b - c*d; // should be ~0
    if (dif.absMaximum() != convert<DataType>(0)) {
        cout << "err201 = " << dif << ";" << endl;
        //exit(201);
    }

    c += convert<DataType>(1); // operator+=(const DataType q)
    if (d != c) {
        cout << "err202 = " << c << ";" << endl;
        //exit(202);
    }

    #define scale 100
    randomize(a, scale);
    randomize(b, scale);
    randomize(c, scale);

    d = (a+b)*c - (a*c + b*c);
    DataType dam = d.absMaximum();
    if (!(30 * N * scale * eps >= dam)) {
        cout << "err203 = " << d << ";" << endl;
        //exit(203);
    }

}

int pow(int a, int n) {
    // versions for float/double are defined in stdlib.
    int r = a;
    for (int i = 1; i < n; i++) r *= a;
    return r;
}
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
int pow(double a, int n) {
    return pow(a, (double)n);
}
#endif

template<int N, typename DataType>
void testbox() {
    typedef RTVec_t<N, DataType> rtvec;
    typedef RTBox_t<N, DataType> rtbox;
    rtbox a;
    a[0] = rtvec(2);
    a[1] = rtvec(12);
    rtbox b(rtvec(1), rtvec(10));
    rtbox d = a + b;
    rtbox e = a - b;
    if (d.sides().minimum() != 11 || e.center().maximum() != 6) {
        cout << "err301 = " << a << ";" << endl;
        //exit(301);
    }
    DataType dv = d.volume();
    d[1][0] = 20;
    rtvec v = d.sides();
    if (v.minIndex() != 1 || dv != pow(convert<DataType>(11), N)) {
        cout << "err302 = " << d << ";" << endl;
        //exit(302);
    }
}

template<int N, typename DataType>
void testbox4() {
    typedef RTVec_t<N, DataType> rtvec;
    typedef RTBox_t<N, DataType> rtbox;
    rtbox a;
    a[0] = rtvec(convert<DataType>(2));
    a[1] = rtvec(convert<DataType>(12));
    rtbox b(rtvec(convert<DataType>(1)), rtvec(convert<DataType>(10)));
    rtbox d = a + b;
    rtbox e = a - b;
    if (d.sides().minimum() != 11 || e.center().maximum() != 6) {
        cout << "err303 = " << a << ";" << endl;
        //exit(303);
    }
}

#include "RTTL/common/RTcoutRedirect.hxx"

int main() { 
    Timer timer; timer.start();
    unsigned int seed = (unsigned int)(CLOCKS_PER_SEC*unsigned(time(NULL)));
    srand(seed);

    cout << "seed = " << seed << ";" << endl;

	// Check if alignment is working...
	int align0 = __alignof(RTData_t<3, float,  0>::AlignedDataType) ;
	int alignx = __alignof(RTData_t<3, float, 16>::AlignedDataType);
	if (align0 != __alignof(float) || alignx != __alignof(sse_f)) {
        cout << "err00 = " << align0 << " vs " << alignx << endl;
        //exit(1);
	}

    RTVec_t<4, float, 16> a(0.0f);
    int aa = __alignof(a);
    a.entry(1) = 1;            // default cast (to float&)
    a.entry<float>(2) = 2;     // explicit cast
    a.entry<float>(3) = 3;

    // Different overloads for 'const' attribute (same behavior)
    //const
    _ALIGN(16) float a0[] = {0,1,2,3};
    if (a != (RTVec_t<4, float, 16>&)a0) { // using cast
        cout << "err01 = " << a << ";" << endl;
        //exit(1);
    }
    if (a != a0) {
        cout << "err021 = " << a << ";" << endl;
        //exit(21);
    }
    if (a == *a0) { // operator==
        cout << "err022 = " << a << ";" << endl;
        //exit(22);
    }

    RTVec_t<4, float, 16> b;
    b = 3.0f;
    if (b != a0[3]) { // operator!=
        cout << "err023 = " << a << ";" << endl;
        //exit(23);
    }

    // Mixed operands and casts.
    float* bv = b.pointer<float>();
    bv[0] = 1;
    bv[1] = 2;
    bv[2] = 3;
    bv[3] = 4;
    if (a == b) { // comparing RTVec_t<4, float, 16>
        cout << "err03 = " << a << ";" << endl;
        //exit(3);
    }
    if (a != b - 1) {
        cout << "err04 = " << a << ";" << endl;
        //exit(4);
    }
    if (a != b - 1.0f) {
        cout << "err05 = " << a << ";" << endl;
        //exit(5);
    }

    RTVec_t<1, sse_f> asse;
    asse = convert(3.0f, 2.0f, 1.0f, 0.0f); // _mm_set_ps
    if (asse != a.entry<sse_f>()) {
        cout << "err06 = " << asse << ";" << endl;
        //exit(6);
    }

    // Access individual floats in sse_f vector.
    asse.entry<float>(1) = 5;
    if (asse != convert(3.0f, 2.0f, 5.0f, 0.0f)) {
        cout << "err07 = " << asse << ";" << endl;
        //exit(7);
    }

    // Difference between cast and convert.
    RTVec_t<1, sse_i> assei;
    assei = convert(3, 2, 5, 0);
    if (assei[0] != convert(asse[0])) {
        cout << "err081 = " << assei << ";" << endl;
        //exit(81);
    }
    if (assei[0] == cast(asse[0])) {
        cout << "err082 = " << assei << ";" << endl;
        //exit(82);
    }

    // Check different sizes and data types.

    test<5, float>();
//#if 0
    test<6, float>();
    test<8, float>();
    test<5, int>();
    test<6, int>();
    test<8, int>();
    test<5, double>();
    test<6, double>();
    test<8, double>();

    test4<16, int>();
    test4<16, float>();
    test4<16, sse_f>();

    // Specialized instances.
    test<2, int>();
    test<3, float>();
    test<4, char>();

    test4<3, sse_f>();

    #if !defined(__GNUC__) || defined(__INTEL_COMPILER)
    _ALIGN(16) float ef[] = {4,3,2,1, 8,7,6,5, 11,10,9,8};
    sse_f* e = (sse_f*)ef;
    RTVec_t<3, sse_f> a4;
    a4 = RTVec_t<3, sse_f>(convert(2.0f)); // fine without template parameter
    a4 *= convert<sse_f>(4);               // need one
    a4 -= e;
    sse_f ama = a4.maximum();
    sse_f ami = a4.minimum();
    sse_f adi = ama - ami;
    if (adi != 7) {
        cout << "err09 = " << a4 << ";" << endl;
        //exit(9);
    }
    #endif

    RTVec_t<4, float, 16> at0(1.0f);
    RTVec_t<3, float> at1(1.0f);
    RTVec_t<4, float, 16> at2(1.0f);
    // Vectors of different size are always not equal.
    if (at1 == at0) {
        cout << "err10 = " << at0 << ";" << endl;
        //exit(10);
    }
    // The same size.
    if (at2 != at0) {
        cout << "err11 = " << at0 << ";" << endl;
        //exit(11);
    }

    test4<4, float>();
    test4<4, sse_f>();
    test4<4, sse_i>();

    testbox<2, float>();
    testbox<3, int>();
    testbox<4, float>();
    testbox<8, double>();

    testbox4<1, sse_f>();

    RTBox_t<3, float, 16> b3;
    _ALIGN(16) float b31[] = {1,2,3};
    b3.m_min = (RTVec_t<3, float, 16>&)b31;
    _ALIGN(16) float b32[] = {3,6,5};
    b3.m_max = (RTVec_t<3, float, 16>&)b32;
    int b3i = b3.maxIndex();
    float b3ar = b3.area();
    float b3vol = b3.volume();
    if (b3i != 1 || b3ar != 40 || b3vol != 16) {
        cout << "err12 = " << b3 << ";" << endl;
        //exit(12);
    }

    RTBox_t<1, sse_f> b4;
    b4[0] = _mm_set_ps(1,2,3,4);
    b4[1] = _mm_set_ps(5,5,6,6);
    b4[0].entry<float>(3) = 2;
    sse_f bs = b4.sides()[0];
    RTBox_t<4, float>& bf = (RTBox_t<4, float>&)b4;
    float bfv = bf.volume();
    if (bfv != 54) {
        cout << "err13 = " << b4 << ";" << endl;
        //exit(13);
    }

#if 0
    RTBox3a b3a;
    b3a[0] = _mm_set_ps(-1,3,2,1);
    b3a[1] = _mm_set_ps(-1,5,4,3);
    float b3av = b3a.volume();
    float b3aa = b3a.area();
    if (b3av != 8 || b3aa != 24) {
        cout << "err14 = " << b3a << ";" << endl;
        //exit(14);
    }
#else
    cout << "WARNING: as layout if RTBox3a is unclear, code has been disabled" << endl;
#endif
    
//#endif
    cout << "success (" << timer.stop() << " seconds)" << endl;
    return 0;
}
