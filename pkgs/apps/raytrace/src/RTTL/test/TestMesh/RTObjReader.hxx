#ifndef RTOBJREADER_CXX
#define RTOBJREADER_CXX

#include "RTTL/common/MapOptions.hxx"

// For extended specs see
// http://www.martinreddy.net/gfx/3d/OBJ.spec
// http://www.fileformat.info/format/wavefrontobj/
namespace RTTL {
    
    inline static
    bool getInt3(char*& token, int& in, int& tn, int& nn) {
        // Parse differently formated triplets like
        // n0
        // n0/n1/n2
        // n0//n2
        // Missing entries are assigned -1.
        token += strcspn(token, " \t\n");
        token += strspn (token, " \t\n");
        if (token[0] == 0) {
        err:
            in = tn = nn = -1;
            return false;
        }
        in = atoi(token);
        token += strcspn(token, "/ \t\n");
        if (token[0] == '/') {
            // Either i/t/n or i//n
            if (token[1] == '/') {
                // it is i//n (no texture index)
                tn = -1;
                nn = atoi(token + 2);
                return true;
            }
            // it is i/t/n
            tn = atoi(++token);
            token = strchr(token, '/');
            if (token == 0) goto err;
            nn = atoi(++token);
        } else {
            tn = nn = -1;
        }
        return true;
    }
    
    template<int N>
    inline static
    RTVec_t<N, float> getVec(char* token) {
        // Read a vector form a string
        RTVec_t<N, float> v;
        #pragma unroll(3)
        for (int i = 0; i < N; i++) {
            token += strcspn(token, " \t\n");
            token += strspn (token, " \t\n");
            v[i] = atof(token);
        }
        return v;
    }
    
    template<long long mesh_descriptor>
    inline static
    bool readObj(string fileName, RTTriangleMesh<mesh_descriptor>& mesh) {

        bool merge_triangles_to_quads = options.defined("merge_quads");
        bool store_quads = merge_triangles_to_quads? true : options.defined("quads");

        const int LINE_SIZE = 1000;
        char line[LINE_SIZE];
        FILE* file = fopen(fileName.c_str(), "rt");
        if (!file) {
            perror(fileName.c_str());
            return false;
        }

        // First  phase : only scan the file to compute sizes (false).
        // Second phase : actually parse the file (true).
        bool parse = false;
        do {
            //  Counters of processed
            //  vertices, normals,  textures, triangles
            int nvv = -1, nvn = -1, nvt = -1, nt = -1;
            // Read lines one by one.
            while (fgets(line, LINE_SIZE, file)) {
                int ll;
                // check for continuation
                while (line[ll = (strlen(line)-2)] == '\\') {
                    if (ll >= LINE_SIZE-8) {
                        printf("The following line in %s is too long:\n%s\n", fileName.c_str(), line);
                        return false;
                    }
                    fgets(line+ll, LINE_SIZE - ll, file);
                }

                char* token = line + strspn(line, " \t\n");
                if (!token) continue;

                if (token[0] == 'v') {
                    if		  (token[1] == ' ') {
                        // Vertex
                        nvv++;
                        if (!parse) continue;
                        mesh.addVertexPosition(getVec<3>(token));
                    } else if (token[1] == 'n') {
                        // Normal
                        nvn++;
                        if (!parse) continue;
                        mesh.addVertexNormal(getVec<3>(token));
                    } else if (token[1] == 't') {
                        // Texture
                        nvt++;
                        if (!parse) continue;
                        mesh.addVertexTexture1(getVec<2>(token));
                    }
                } else if (token[0] == 'f' && strchr(" \t", token[1])) {
                    // Face: tri.v[i]/texture/normal
                    int t0, n0;
                    int t1, n1;
                    int t2, n2;
                    RTVec3i tri;
                    getInt3(token, tri.x, t0, n0);
                    if (getInt3(token, tri.y, t1, n1) == false) goto err;
                    if (getInt3(token, tri.z, t2, n2) == false) goto err;
                    if (store_quads) {
                    } else {
                        int ntriangles = 1;
                    extract_triangles_from_polygon:
                        ntriangles++;
                        nt++;
                        if (parse) {
                            // Store triangle 0/1/2
                            int ti = mesh.addTriangle(tri);
                            // Assign the 2nd vertex to the 1st (and then read new 2nd  one).
                            tri.y = tri.z; t1 = t2; n1 = n2;
                        }
                        if (getInt3(token, tri.z, t2, n2)) {
                            goto extract_triangles_from_polygon;
                        }
                    }
                } else if (!parse) {
                    // Read the next line while counting # of entities (first phase).
                    continue;
                } else if (token[0] == '#') {
                    // Comment
                    continue;
                } else if (token[0] == 'g' && strchr(" \t", token[1])) {
                    // Named group -- ignore it for now.
                    continue;
                } else if (!strncmp(token, "mtllib", 6)) {
                    // Mtllib
                } else if (!strncmp(token, "usemtl", 6)) {
                    // Usemtl
                } else {
                    // Unsupported -- treat it as a comment.
                    continue;
                }
            }

            // Go for second scan and actually read data (if parse was false)
            // or break out of the loop.
            rewind(file);
            if (!parse) {
                // Prepare for the second phase (actually parsing the file).
                nvv++; nvn++; nvt++; nt++;
                int nvv0 = mesh.numberOfVertices()           ; if (nvv) mesh.setNumberOfVertices           (nvv0 + nvv);
                int nvn0 = mesh.numberOfVertexNormals()      ; if (nvn) mesh.setNumberOfVertexNormals      (nvn0 + nvn);
                int nvt0 = mesh.numberOfTextureCoordinates1(); if (nvt) mesh.setNumberOfTextureCoordinates1(nvt0 + nvt);
                int nt0  = mesh.numberOfTriangles()          ; if (nt ) mesh.setNumberOfTriangles          (nt0  + nt);
                nvv = nvv0 - 1;
                nvn = nvn0 - 1;
                nvt = nvt0 - 1;
                nt  = nt0  - 1;
            }
        } while (parse = !parse);

        fclose(file);
        return true;
    err:
        printf("Errors parsing file %s\n", fileName.c_str());
        return false;
    }
};

#endif
