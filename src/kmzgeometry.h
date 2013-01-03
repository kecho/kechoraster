#ifndef _KMZ_GEOMETRY_H_
#define _KMZ_GEOMETRY_H_

#include <vector>
#include <string>

namespace kmz
{
    struct Vertex
    {
        float x, y, z, nx, ny, nz, u, v;
    };
    class Mesh 
    {
    public:
        explicit Mesh (std::string name);
        ~Mesh();
        void AllocateVertex(int size);
        void AllocateIndexes(int size);
        Vertex& GetVertex(int i) { return mData[i];}
        int& GetIndex(int i) { return mIndexes[i];}
        int * GetIndexBuffer() { return mIndexes; }
        int GetVertexSize() { return mVSize; }
        int GetIndexSize() { return mISize; }
        std::string GetName() { return mName; }
    private:
        Vertex * mData;
        int * mIndexes;
        std::string mName;
        int mVSize;
        int mISize;
    };
    class Geometry
    {
    public:
        Geometry(const char * file);
        ~Geometry();
        int GetMeshSize(){return mMeshes.size();}
        Mesh* GetMesh(int i) { return mMeshes[i];}
    private:
        std::vector<Mesh*> mMeshes;
    };
}

#endif
