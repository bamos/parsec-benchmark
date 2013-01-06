#ifndef RTVIEW_OBJ_PARSER_HXX
#define RTVIEW_OBJ_PARSER_HXX

#include "RTTL/common/RTInclude.hxx"
#include "RTTL/common/RTVec.hxx"
#include "RTTL/common/RTShader.hxx"
#include "ImagePPM.hxx"
#include <map>
#include <set>
#include <algorithm>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

#define NOT_SUPPORTED(x) cout << x << " currently not supported" << endl


class ObjParser {
private:
  RTBox3f sceneAABB;
  std::vector<RTVec3f> vertex;
  std::vector<RTVec3f> normal;
  std::vector<RTVec2f> textureCoord; 

  /* temporary vertex data arrays, required for vertex mapping */

  std::vector<RTVec3f> tmpVtx;
  std::vector<RTVec3f> tmpNor;       
  std::vector<RTVec2f> tmpTxt;
  map<pair<int, pair<int,int> >,int> vertexMap;

  /* ---------------------------- */


  std::vector<RTVec3i> triangle;
  std::vector<int> triangleShaderId;
  std::vector<RTVec4i> quad;
  std::vector<int> quadShaderId;


  std::vector<RTMaterial> material;
  map<string,int> material_map;
  std::vector<ImagePPM*> texture;


  void ParseMTL(const string base, const string fname)
  {
    DBG_PRINT(base);
    DBG_PRINT(fname);

    string name;
    const string filename = base+"/"+fname;
    fstream file;
    file.open(filename.c_str());
    
    cout << "parsing material file " << filename << endl;

    if(!file.is_open())
      {
        FATAL("Error: cannot open %s for reading!\n" << fname.c_str());
      }

    while(!file.eof())
      {
        string s;
        file >> s;

        if (s[0] == '#'){
          getline(file, s, '\n'); // skip an entire line ...
          continue;
        }
        else if (s == "newmtl")
          {
            file >> name;
            material_map[name] = material.size();
            //cout << "New material with shader ID " << material.size() << endl;
            material.push_back(RTMaterial());
          }
        else if (s == "Ns")
          {
            RTMaterial &m = material[material_map[name]];
            file >> m.m_shininess;
          }
        else if (s == "Tr")
          {
            RTMaterial &m = material[material_map[name]];
            file >> m.m_transparency;
          }
        else if (s == "Ka")
          {
            RTMaterial &m = material[material_map[name]];
            file >> m.m_ambient.x;
            file >> m.m_ambient.y;
            file >> m.m_ambient.z;
          }
        else if (s == "Kd")
          {
            RTMaterial &m = material[material_map[name]];
            file >> m.m_diffuse.x;
            file >> m.m_diffuse.y;
            file >> m.m_diffuse.z;
          }
        else if (s == "Ks")
          {
            RTMaterial &m = material[material_map[name]];
            file >> m.m_specular.x;
            file >> m.m_specular.y;
            file >> m.m_specular.z;
          }
        else if (s == "map_Ka")
          {
            string textureName;
            file >> textureName;
            texture.push_back(new ImagePPM((base+"/"+textureName).c_str()));
          }
        else if (s == "map_Kd")
          {
            string textureName;
            file >> textureName;
            texture.push_back(new ImagePPM((base+"/"+textureName).c_str()));
            RTMaterial &m = material[material_map[name]];
            m.m_textureId = 0;
          }
        else if (s == "map_Ks")
          {
            string textureName;
            file >> textureName;
            texture.push_back(new ImagePPM((base+"/"+textureName).c_str()));
          }
        else {
          getline(file, s, '\n');
        }
      }
    cout << "finished parsing material file." << endl;
  }

  _INLINE int getVertexID(int vtxID,int norID, int txtID)
  {
    pair<int, pair < int, int > > v(vtxID, pair<int, int>(txtID, norID));
    if (vertexMap.find(v) == vertexMap.end())
      {
        vertexMap[v] = vertex.size();
        vertex.push_back(tmpVtx[vtxID]);
        if (norID >= 0) normal.push_back(tmpNor[norID]); else normal.push_back(RTVec3f(0.0f,0.0f,0.0f));
        if (txtID >= 0) textureCoord.push_back(tmpTxt[txtID]); else textureCoord.push_back(RTVec2f(0.0f,0.0f));
      }
    return vertexMap[v];

  }

public:

  _INLINE int quads() { return quad.size(); }
  _INLINE int vertices() { return vertex.size(); }
  _INLINE int triangles() { return triangle.size(); }
  _INLINE int normals() { return normal.size(); }
  _INLINE int textureCoordinates() { return textureCoord.size(); }
  _INLINE int materials() { return material.size(); }
  _INLINE int textures() { return texture.size(); }
  _INLINE int triangleShaderIDs() { return triangleShaderId.size(); }
  _INLINE int quadShaderIDs() { return quadShaderId.size(); }

  _INLINE RTBox3f &getSceneAABB() { return sceneAABB; }
  _INLINE RTVec3f *getVertexPtr() { return vertex.size() ? &*vertex.begin() : NULL; }
  _INLINE RTVec3i *getTrianglePtr() { return triangle.size() ? &*triangle.begin() : NULL; }
  _INLINE RTVec4i *getQuadPtr() { return quad.size() ? &*quad.begin() : NULL; }
  _INLINE RTVec3f *getNormalPtr() { return normal.size() ? &*normal.begin() : NULL; }
  _INLINE RTVec2f *getTextureCoordinatePtr() { return textureCoord.size() ? &*textureCoord.begin() : NULL; }
  _INLINE RTMaterial *getMaterialPtr() { return material.size() ? &*material.begin() : NULL; }
  _INLINE int *getTriangleShaderPtr() { return triangleShaderId.size() ? &*triangleShaderId.begin() : NULL; }
  _INLINE int *getQuadShaderPtr() { return quadShaderId.size() ? &*quadShaderId.begin() : NULL ; }

  // might replace this with a more general texture interface (not restricted to PPM)
  _INLINE ImagePPM *getTexture(const int i) { return texture[i]; }

  ObjParser()
  {
    sceneAABB.reset();
  }

  _INLINE void Free()
  {
    tmpVtx.clear();
    tmpNor.clear();
    tmpTxt.clear();
    vertexMap.clear();
    vertex.clear();
    triangle.clear();
    normal.clear();
    textureCoord.clear();
    triangleShaderId.clear();
    quadShaderId.clear();
    for (unsigned int i=0;i<texture.size();i++)
      delete texture[i];
    texture.clear();
    material_map.clear();
  }

  void Parse(string fileName)
  {

    fstream mFile;
    const char* fn = fileName.c_str();
    mFile.open(fn, ios_base::in);

    /* extrace base directory */
    int p = -1;
    for (unsigned int i=0; i<fileName.size(); i++)
#if !defined(_WIN32)
      if (fileName[i] == '/')
#else
        if (fileName[i] == '\\')
#endif
          p = i;
    string base;
    if (p != -1)
      base = fileName.substr(0,p);
    else
      {
	//FIXME: Implement proper base extraction
        base = ".";
      }

    if(!mFile.is_open())
      {
        cout << "Error: cannot open " << fileName << " for reading!" << endl;
        perror("Error code");
        exit(-1);
      }

    int shaderId = 0;

    int count = 0;
    while(!mFile.eof())
      {
        count++;
        string s;
        mFile >> s;

        if (s[0] == '#'){
          continue;
        }
        else if (s == "mtllib")
          {
            string mtllib;
            mFile >> mtllib;
            ParseMTL(base,mtllib);
          }
        else if (s == "g")
          {
            string temp;
            mFile >> temp;
          }
        else if (s == "usemtl")
          {
            string material;
            mFile >> material;
            shaderId = material_map[material];
            //cout << "using material " << material << " -> active shaderId " << shaderId << endl;
          }
        else if (s == "v")
          {
            RTVec3f v;
            mFile >> v[0];
            mFile >> v[1];
            mFile >> v[2];
            sceneAABB.extend(v);
            tmpVtx.push_back(v);
          }
        else if (s == "vt")
          {
            RTVec2f c;
            mFile >> c[0];
            mFile >> c[1];
            tmpTxt.push_back(c);
          }
        else if (s == "vn")
          {
            RTVec3f n;
            mFile >> n[0];
            mFile >> n[1];
            mFile >> n[2];
            tmpNor.push_back(n);
          }
        else if (s == "f" || s == "fo")
          {
            string dash;
            int ta = -1,tta=0,tna=0;
            int tb = -1,ttb=0,tnb=0;
            int tc = -1,ttc=0,tnc=0;
            int td = -1,ttd=0,tnd=0;

            mFile >> dash;

            if (sscanf(dash.c_str(),"%d/%d/%d", &ta ,&tta, &tna) != 3 &&
                sscanf(dash.c_str(),"%d/%d", &ta ,&tta) != 2 &&
                sscanf(dash.c_str(),"%d//%d", &ta ,&tna) != 2 &&
                sscanf(dash.c_str(),"%d", &ta) != 1
                )
              {
                printf("Parsing error v0: %i\n",count);
                exit(-1);
              }

            mFile >> dash;

            if (sscanf(dash.c_str(),"%d/%d/%d", &tb, &ttb, &tnb) != 3 &&
                sscanf(dash.c_str(),"%d/%d", &tb ,&ttb) != 2 &&
                sscanf(dash.c_str(),"%d//%d", &tb ,&tnb) != 2 &&
                sscanf(dash.c_str(),"%d", &tb) != 1
                )
              {
                printf("Parsing error v1: %i\n",count);
                exit(-1);
              };

            mFile >> dash;

            if (sscanf(dash.c_str(),"%d/%d/%d", &tc, &ttc, &tnc) != 3 &&
                sscanf(dash.c_str(),"%d/%d", &tc ,&ttc) != 2 &&
                sscanf(dash.c_str(),"%d//%d", &tc ,&tnc) != 2 &&
                sscanf(dash.c_str(),"%d", &tc) != 1
                )
              {
                printf("Parsing error v2: %i\n",count);
                exit(-1);
              };


            bool isQuad = true;

            char c;
            while (mFile.peek() == ' ') mFile.get(c);

            if (mFile.peek() == 0x0D || mFile.peek() == 0x0A) {
              while (mFile.peek() == 0x0D || mFile.peek() == 0x0A) mFile.get(c);
              isQuad = false;
            }
            else
              {
                mFile >> dash;
                if (sscanf(dash.c_str(),"%d/%d/%d", &td, &ttd, &tnd) != 3 &&
                    sscanf(dash.c_str(),"%d/%d", &td ,&ttd) != 2 &&
                    sscanf(dash.c_str(),"%d//%d", &td ,&tnd) != 2 &&
                    sscanf(dash.c_str(),"%d", &td) != 1
                    )
                  {
                    isQuad = false;
                  }
              }

            bool has_na = tna != 0;
            bool has_nb = tnb != 0;
            bool has_nc = tnc != 0;
            bool has_nd = tnd != 0;

            bool has_ta = tta != 0;
            bool has_tb = ttb != 0;
            bool has_tc = ttc != 0;
            bool has_td = ttd != 0;

            if (ta > 0) ta--;
            if (tb > 0) tb--;
            if (tc > 0) tc--;
            if (td > 0) td--;

            if (tta > 0) tta--;
            if (ttb > 0) ttb--;
            if (ttc > 0) ttc--;
            if (ttd > 0) ttd--;

            if (tna > 0) tna--;
            if (tnb > 0) tnb--;
            if (tnc > 0) tnc--;
            if (tnd > 0) tnd--;

            if (ta < 0) ta += tmpVtx.size();
            if (tb < 0) tb += tmpVtx.size();
            if (tc < 0) tc += tmpVtx.size();
            if (td < 0) td += tmpVtx.size();

            if (tta < 0) tta += tmpTxt.size();
            if (ttb < 0) ttb += tmpTxt.size();
            if (ttc < 0) ttc += tmpTxt.size();
            if (ttd < 0) ttd += tmpTxt.size();

            if (tna < 0) tna += tmpNor.size();
            if (tnb < 0) tnb += tmpNor.size();
            if (tnc < 0) tnc += tmpNor.size();
            if (tnd < 0) tnc += tmpNor.size();

            /*
              DBG_PRINT(ta);
              DBG_PRINT(tb);
              DBG_PRINT(tc);
              DBG_PRINT(td);
              DBG_PRINT(quad);
            */

            if (ta < 0 || ta >= (int)tmpVtx.size() ||
                tb < 0 || tb >= (int)tmpVtx.size() ||
                tc < 0 || tc >= (int)tmpVtx.size() ||
                (isQuad && (td < 0 || td >= (int)tmpVtx.size())))
              /*
                (tta < 0 || tta >= (int)textureCoord.size()) && has_ta ||
                (ttb < 0 || ttb >= (int)textureCoord.size()) && has_tb ||
                (ttc < 0 || ttc >= (int)textureCoord.size()) && has_tc ||
                (ttd < 0 || ttd >= (int)textureCoord.size()) && has_td ||
                (tna < 0 || tna >= (int)normal.size()) && has_na ||
                (tnb < 0 || tnb >= (int)normal.size()) && has_nb ||
                (tnc < 0 || tnc >= (int)normal.size()) && has_nc ||
                (tnd < 0 || tnd >= (int)normal.size()) && has_nd
              */
              {
                DBG_PRINT(td);
                DBG_PRINT(tmpVtx.size());
                printf("Invalid triangle %i: (%i,%i,%i) (%i,%i,%i) (%i,%i,%i) (%i,%i,%i) (%i,%i,%i) %i %i %i\n",
                       count,ta,tb,tc,tta,ttb,ttc,tna,tnb,tnc,has_na,has_nb,has_nc,has_ta,has_tb,has_tc,
                       int(tmpVtx.size()),
                       int(tmpTxt.size()),
                       int(tmpNor.size()));
                exit(-1);
              }

            if (!has_ta) tta = -1;
            if (!has_tb) ttb = -1;
            if (!has_tc) ttc = -1;
            if (!has_td) ttd = -1;

            if (!has_na) tna = -1;
            if (!has_nb) tnb = -1;
            if (!has_nc) tnc = -1;
            if (!has_nd) tnd = -1;

            if (isQuad)
              {
                const int newA = getVertexID(ta,tna,tta);
                const int newB = getVertexID(tb,tnb,ttb);
                const int newC = getVertexID(tc,tnc,ttc);
                const int newD = getVertexID(td,tnd,ttd);
                quad.push_back(RTVec4i(newA,newB,newC,newD));
                quadShaderId.push_back(shaderId);
              }
            else
              {
                const int newA = getVertexID(ta,tna,tta);
                const int newB = getVertexID(tb,tnb,ttb);
                const int newC = getVertexID(tc,tnc,ttc);
                triangle.push_back(RTVec3i(newA,newB,newC));
                triangleShaderId.push_back(shaderId);
              }
          }
        else
          continue;
      }

    assert( textureCoordinates() == vertices() );
    assert( normals() == vertices() );
  }

  
};

#endif
