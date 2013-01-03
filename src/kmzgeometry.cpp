#include "kmzgeometry.h"
#include  <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "string.h"

using namespace std;

kmz::Mesh::Mesh(std::string name)
:
    mVSize(0), mISize(0), mData(0), mIndexes(0) 
{
    mName = name;
}

kmz::Mesh::~Mesh()
{
    if (mIndexes)
    {
        delete [] mIndexes;
    }
    if (mData)
    {
        delete [] mData;
    }
}

void kmz::Mesh::AllocateVertex(int size)
{
    mData = new kmz::Vertex[size];
    mVSize = size;
}

void kmz::Mesh::AllocateIndexes(int size)
{
    mIndexes = new int[size];
    mISize = size;
}

const char * consume(const char * match, const char * src)
{
    int i = 0;
    while (match[i] != 0)
    {
        if (src[i] == 0 || match[i] != src[i])
        {
            return 0;
        }
        ++i;
    }
    return src + i;
}


void fill_mesh(kmz::Mesh * m, ifstream& infile)
{
    string line;
    getline(infile, line);
    while (strcmp(line.c_str(), "mesh-end"))
    {
        const char * cline = line.c_str();
        const char * vertice_n = consume("vertices ", cline);
        if (vertice_n != 0)
        {
            int num = 0; 
            sscanf(vertice_n, "%d", &num);
            m->AllocateVertex(num);  
            for (int i = 0; i < num; ++i)
            {
                getline(infile, line);
                const char * vinfo = consume("v: ", line.c_str());
                kmz::Vertex& v = m->GetVertex(i);
                sscanf(vinfo, "%f %f %f %f %f %f %f %f",
                    &v.x,
                    &v.y,
                    &v.z,
                    &v.nx,
                    &v.ny,
                    &v.nz,
                    &v.u,
                    &v.v
                );
            }
        }
        const char * triangles = consume("triangles ", cline);
        if (triangles != 0)
        {
            int num = 0;
            sscanf(triangles, "%d", &num);
            m->AllocateIndexes(3*num);
            for (int i = 0; i < num; ++i)
            {
                getline(infile, line);
                const char * triangle = consume("t: ", line.c_str());
                int& a = m->GetIndex(3*i);
                int& b = m->GetIndex(3*i + 1);
                int& c = m->GetIndex(3*i + 2); 
                sscanf(triangle, "%d %d %d",&a,&b,&c);
            }
        }
        
        getline(infile, line);
    }
}

void parseFile(std::vector<kmz::Mesh*>& list, ifstream& infile)
{
    string line;
    while(getline(infile, line))
    {
        const char * cline = line.c_str();
        stringstream ss(cline);

        const char * begin_token = consume("mesh-begin ", cline);
        if (begin_token)
        {
            std::string meshName = begin_token;
            kmz::Mesh * m = new kmz::Mesh(meshName);
            list.push_back(m);
            fill_mesh(m, infile);
        }
    } 
}

kmz::Geometry::Geometry(const char * file)
{
    ifstream infile;
    infile.open(file);
    parseFile(mMeshes, infile);
}

kmz::Geometry::~Geometry()
{
    for (int i = 0; i < mMeshes.size(); ++i)
    {
        kmz::Mesh * m = mMeshes[i];
        delete m;
    }
}
