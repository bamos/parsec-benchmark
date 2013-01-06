#include <time.h>

//#include "RTTL/common/RTMesh.hxx"
#include "RTMesh.hxx"

#include "RTTL/common/Timer.hxx"
using namespace RTTL;

#include "RTTL/common/RTcoutRedirect.hxx"

int main(int argc, char* argv[]) {
    options.parse(argc-1, argv+1);

    int nfiles = options.vector_size("files");
    RTTriangleMesh<RT_INDEXED_MESH> mesh;
    for (int fi = 0; fi < nfiles; fi++) {
        const string& fn = (*options["files"])[fi];
        cout << "Adding obj file: " << fn << endl;
        readObj(fn, mesh);
    }
    
    int N = 100;

    Timer timer; timer.start();
    unsigned int seed = (unsigned int)(CLOCKS_PER_SEC*unsigned(time(NULL)));
    srand(seed);
    cout << "seed = " << seed << ";" << endl;

    RTTriangleMesh<RT_FAT_VERTEX_MESH>::Vertex v1;
    v1.m_vertex_position = RTVec3f(1,2,3);
    v1.m_vertex_normal   = RTVec3f(11,22,33);
    v1.m_vertex_texture1 = RTVec2f(100,200);
    RTTriangleMesh<RT_FAT_VERTEX_MESH>::Vertex v2;
    v2.m_vertex_position = RTVec3f(5,6,7);
    v2.m_vertex_normal   = RTVec3f(33,44,55);
    v2.m_vertex_texture1 = RTVec2f(200,400);
    RTTriangleMesh<RT_FAT_VERTEX_MESH>::Vertex v3 = (*v1 + *v2)/2;

    cout << v3 << endl;

    RTTriangleMesh<RT_FAT_VERTEX_MESH> tm;
    tm.setNumberOfVertices(N);
    tm.setNumberOfTriangles(N);
    tm.setNumberOfFaceNormals(N);
    tm.setNumberOfMaterials(N);

    tm.addTriangle(RTVec3i(1,2,3));
    tm.vertexPosition(0, 0) = RTVec3f(1,1,10);
    tm.vertexPosition(0, 1) = RTVec3f(2,2,20);
    tm.vertexPosition(0, 2) = RTVec3f(3,3,30);
    tm.vertexNormal(0, 0)   = RTVec3f(1,1,1);
    tm.vertexNormal(0, 1)   = RTVec3f(2,2,2);
    tm.vertexNormal(0, 2)   = RTVec3f(3,3,3);

    tm.triangle(0).m_face_normal_index = 0;
    tm.faceNormal(0) = RTVec3f(5,5,5);

    tm.addTriangle();
    tm.triangle(1) = tm.triangle(0);

    cout << endl << tm.triangle(1);

    const RTTriangleMesh<RT_FAT_VERTEX_MESH>& tmc = tm;
    RTVec3f fn = tmc.vertexNormal(0, 0);

    RTTriangleMesh<RT_INDEXED_MESH> tmi;
    tmi.setNumberOfVertices(N);
    tmi.setNumberOfTriangles(N);
    tmi.setNumberOfFaceNormals(N);
    tmi.setNumberOfMaterials(N);

    tmi.addTriangle(RTVec3i(1,2,3));
    tmi.triangle(0).m_face_normal_index = 0;
    tmi.faceNormal(0) = RTVec3f(5,5,5);

    RTTriangleMesh<RT_FAT_TRIANGLE_MESH> tmf;
    tmf.setNumberOfTriangles(N);

    tmf.addTriangle();
    tmf.vertexPosition(0,0) = RTVec3f(1,1,1);
    tmf.vertexPosition(0,1) = RTVec3f(1,1,1);
    tmf.vertexPosition(0,2) = RTVec3f(1,1,1);
    tmf.faceNormal(0) = RTVec3f(3,3,3);

    cout << endl << tmf.triangle(0);

    cout << "success (" << timer.stop() << " seconds)" << endl;
    return 0;
}
