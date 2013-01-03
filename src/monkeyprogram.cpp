#include "program.h"
#include "kechoraster.h"
#include "kmzgeometry.h"
#include <vector>
#include <math.h>
#include <stdio.h>

using namespace kechorender::core;
using namespace kechorender::render;
using namespace kechorender::math;

static const int WIDTH = 600;
static const int HEIGHT = 400;


class MyPixelIn : public PixelIn
{
public:
    MyPixelIn() 
    {
        RegisterAttribute(&color);
    }
    virtual ~MyPixelIn(){}
    float color;
    Vector4 w;
};

class MyVertexIn : public VertexIn
{
public:
    Vector3 normal;
};

class MyShader : public IRenderShader
{
public:
    virtual ~MyShader(){}
    virtual void OnVertex(const VertexIn& in, PixelIn& out)
    {
        const MyVertexIn& min = (const MyVertexIn&)in;
        MyPixelIn& mout = (MyPixelIn&)out;
        mout.position = mViewProj * (mLocalToWorld * min.position);
        Vector3 lightDir = (float[]){-1, 1, 0};
        Vector3 wvNormal = toWorld(min.normal);

        Vector3 normalizedLight = lightDir.Normalize();
        Vector3 normalizedNormal = wvNormal.Normalize();
        float intensity = saturate(normalizedLight.dot(normalizedNormal));
        mout.color = saturate( intensity * 2.0 + 0.2);
    }

    float saturate(float v)
    {
        return v < 0 ? 0 : (v > 1 ? 1 : v);
    }


    Vector3 transform(const Matrix44& m, const Vector3&v)
    {
        Vector4 t = (float[]) {v.x(), v.y(), v.z(), 1.0f};
        Vector4 tr = m * t;
        Vector3 result = (float[]){tr.x() / tr.w(), tr.y() / tr.w(), tr.z() / tr.w()};
        return result;
    }

    Vector3 toView(const Vector3& v)
    {
        return transform(mView, v);
    }
    Vector3 toWorld(const Vector3& v)
    {
        return transform(mLocalToWorld, v);
    }

    virtual void OnPixel(const PixelIn& in, PixelOut& out)
    {
        const MyPixelIn& min = (const MyPixelIn&)in;
        out.r = min.color ;
        out.g = 0;
        out.b = 0;
        out.a = 1.0f;
    }

    virtual PixelIn * CreatePixelInput() { return new MyPixelIn(); }
    
    Matrix44 mLocalToWorld;
    Matrix44 mView;
    Matrix44 mProj;
    Matrix44 mViewProj;
    
};

class MonkeyProgram : public ProgramInterface
{
public:
    MonkeyProgram()
    {
        fb = 0;
        mRAmmount = 0;
    }
    virtual ~MonkeyProgram()
    {
    }

    void initList()
    {
        for (int i = 0; i < mesh->GetVertexSize(); ++i)
        {
            kmz::Vertex& v = mesh->GetVertex(i);   
            MyVertexIn * vin = new MyVertexIn();
            vin->position = (float[]){v.x, v.y, v.z, 1.0f};
            vin->normal = (float[]){v.nx, v.ny, v.nz};
            mList.push_back(vin);
        }
    }
    
    void destroyList()
    {
        for (int i = 0; i < mList.size(); ++i)
        {
            delete mList[i];
        } 
    }

    virtual void OnStart()
    {
        g = new kmz::Geometry("geometry/monkey.kmz");
        mesh = g->GetMesh(0);
        fb = new FrameBuffer(WIDTH, HEIGHT);
        mDevice.SetRenderTarget(fb);
        mDevice.CreateZBuffer();
        mDevice.SetRenderShader(&mShader);
        initList();
        mDevice.SetVertexList(mList);

        mShader.mViewProj = (float[])
        {
           1, 0, 0, 0,
           0, 1, 0, 0,
           0, 0, 1, 0,
           0, 0, 0, 1,
        };

        PrepareView();
        PrepareProj();
        PrepareViewProj();
        
    }

    virtual void OnEnd()
    {
        delete g;
        if (fb) { delete fb; }
        destroyList();
    }

    virtual const char * GetName()
    {
        return "Sample Kecho Rasterizer";
    }
    
    void PrepareView()
    {
        Vector3 eye = (float[]) {0,0,0.2};
        Vector3 target = (float[]) {0,0,1};
        Vector3 up = (float[]) {0,1,0};
        ViewMatrix(eye, target, up, mShader.mView);
    }
    
    void PrepareProj()
    {
        float ratio = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
        Frustum(-ratio, ratio, 1, -1, 3, 7, mShader.mProj);
    }
    
    void PrepareViewProj()
    {
         matmul(mShader.mProj, mShader.mView,  mShader.mViewProj);
    }

    void PrepareWorld()
    {
        Vector3 axis = (float[]){0, 1, 0};
        RotateMatrix(axis, mRAmmount, mShader.mLocalToWorld);
        mRAmmount += 0.1;
        Vector3 translate = (float[]){0,0.9,7};
        TranslateMatrix(translate, mShader.mLocalToWorld);
    };

    virtual void Render()
    {
        PrepareWorld();
        Vector3 clearcolor = (float[]){0, 0, 0};
        mDevice.Clear(clearcolor);
        mDevice.ClearZBuffer();
        mDevice.DrawTriangleIndexes(mesh->GetIndexSize(), mesh->GetIndexBuffer());
    }
    virtual FrameBuffer * GetFrame() {return fb;}
private:
    float mRAmmount;
    FrameBuffer * fb;
    KechoDevice mDevice;
    MyShader mShader;
    std::vector<VertexIn*> mList;
    kmz::Geometry * g;
    kmz::Mesh * mesh;
};


ProgramInterface * ProgramInterface::create()
{
    return new MonkeyProgram();
}
