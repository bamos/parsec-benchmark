#include <time.h>

#include "RTTL/common/RTRay.hxx"
using namespace RTTL;

template <int N, int MULTIPLE_ORIGINS, int SHADOW_RAYS>
void test1() {
    const int LAYOUT = USE_CORNER_RAYS;
    int i, j;
    RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> packet0;
    packet0.reset();
    RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> packet;
    packet = packet0; // implicit operator=

    // Check various ways of setting/accessing data members.

    if (!SHADOW_RAYS) {
        // id is defined, so we cast to class in which it is "visible".
        RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, 0>& packetid = (RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, 0>&)packet;
        packetid.intId(0) = 1;
        packetid.intId(1) = 2;
        packetid.intId(2) = 3;
        packetid.intId(3) = 4;
        packetid.id(1) = _mm_setr_epi32(5,6,7,8);
        for (i = 2; i < N; i++) {
            int k = 4*i;
            packetid.id(i) = _mm_set_epi32(k+4, k+3, k+2, k+1);
        }
        for (i = 0; i < N; i++) {
            int k = 4*i;
            if (packetid.id(i) != _mm_setr_epi32(k+1, k+2, k+3, k+4)) {
                cout << "err11 = " << packetid.id(0) << ";" << endl;
                exit(11);
            }
        }

        for (i = 0; i < 4*N; i++) {
            if (packetid.intId(i) != i+1) {
                cout << "err12 = " << packetid.id(0) << ";" << endl;
                exit(12);
            }
        }
    }

    packet.originX(0) = _mm_setr_ps(101,102,103,104);
    packet.originY(0) = _mm_setr_ps(201,202,203,204);
    packet.originZ(0) = _mm_setr_ps(301,302,303,304);

    // Access origins as sse_f values.
    for (j = 0; j < 3; j++) {
        for (i = 1; i < N; i++) {
            int k = 100*(j+1)+4*i;
            packet.origin(j,i) = MULTIPLE_ORIGINS? _mm_setr_ps(k+1, k+2, k+3, k+4) : _mm_set_ps1(k);
        }
    }

    // Per-component access to data members.
    if (MULTIPLE_ORIGINS) {
        for (j = 0; j < 3; j++) {
            for (i = 0; i < 4*N; i++)
                if (packet.floatOrigin(j,i) != 100*(j+1)+i+1) {
                    cout << "err14 = " << packet.origin(0,0) << ";" << endl;
                    exit(14);
                }
        }
    } else {
        // common origin
        for (j = 0; j < 3; j++) {
            for (i = 0; i < 4*N; i++)
                if (packet.floatOrigin(j,i) != 100*(j+1)+4*(N-1)) {
                    cout << "err14 = " << packet.origin(0,0) << ";" << endl;
                    exit(14);
                }
        }
    }

    RayPacket<N, STORE_NEAR_FAR_DISTANCE, MULTIPLE_ORIGINS, SHADOW_RAYS> packet2;
    packet2.reset();
    for (i = 0; i < 4*N; i++) {
        if (packet2.floatDistance(i) != (SHADOW_RAYS?1:numeric_limits<float>::infinity()) || packet2.floatMinDistance(i) != 0) {
            cout << "err15 = " << packet2.distance(0) << ";" << endl;
            exit(15);
        }
    }

}

int main() {
    unsigned int seed = (unsigned int)(CLOCKS_PER_SEC*unsigned(time(NULL)));
    srand(seed);
    cout << "seed = " << seed << ";" << endl;

    test1<4,1,1>();
    test1<8,1,1>();
    test1<16,1,1>();
    test1<4,1,0>();
    test1<8,1,0>();
    test1<16,1,0>();

    test1<8,0,1>();
    test1<8,0,0>();

    //RayPacket<4, 1, 1, 2> syntax_error;

    // Assignment of vectors of different types and sizes.
    RTTL::RTVec_t<6, float> v0(33.333f);
    RTTL::RTVec_t<6, long long> vl;
    RTTL::RTVec_t<6, float, 16> vf;
    vl = v0;
    vf = vl;  // generic, data conversion
    RTTL::RTVec_t<5, float, 16> vf2;
    vf2 = vf; // generic, no data conversion
    cout << vf2;

    RTTL::RTVec_t<5, float, 16> vf3;
    vf3 = vf2; // SSE
    cout << vf3;

    RTTL::RTVec_t<5, float, 16> vc;
    vc = vf3;
    cout << vc;

    cout << "success" << endl;
    return 0;
}
