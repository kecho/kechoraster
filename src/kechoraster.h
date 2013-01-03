#ifndef __KECHO_RASTER_H__
#define __KECHO_RASTER_H__

#include "kechorastermath.h"
#include <vector>

namespace kechorender
{
namespace core
{
    class FrameBuffer;
}
namespace render
{
    class PixelIn
    {
    public:
        kechorender::math::Vector4 position; 
        PixelIn();
        virtual ~PixelIn(){}
        int GetNumOfAttributes() const {return mScalarReferences.size() ;}
        float * GetAttribute(int i) const { return mScalarReferences[i]; }
    protected:
        int RegisterAttribute(kechorender::math::IDataArray<float>& a);
        int RegisterAttribute(float* a);
    private:
        std::vector<float*> mScalarReferences;
    };

    class Interpolation
    {
    public:
        Interpolation(const PixelIn& a, const PixelIn& b, const PixelIn& c);
        ~Interpolation();
        void Interpolate(const kechorender::math::Vector2& position, PixelIn& p);
    private:
        int RegisterScalar(
            float a, float b, float c
        );
        float InterpolateScalar(int id, const kechorender::math::Vector2& positon);
        std::vector<kechorender::math::Vector3> mAttributes;
        kechorender::math::Matrix33 mInterpolationMat;
        int mCurrentAttribute;

    };

    class VertexIn
    {
    public:
        VertexIn(){}
        virtual ~VertexIn(){}
        kechorender::math::Vector4 position; 
    };


    class PixelOut
    {
    public:
        float r, g, b, a;
    };

    class IRenderShader
    {
    public:

        virtual void OnVertex(const VertexIn&  in, PixelIn&  out) = 0;
        virtual void OnPixel(const PixelIn& in, PixelOut& out) = 0;
        virtual PixelIn * CreatePixelInput() {return new PixelIn();}
    };
    class KechoDevice
    {
    public:
        KechoDevice();
        ~KechoDevice();
        void SetRenderTarget(kechorender::core::FrameBuffer* target) { mTarget = target; }
        void SetRenderShader(IRenderShader * sh) {mShader = sh;}
        kechorender::core::FrameBuffer * GetRenderTarget() { return mTarget; }
        void SetVertexList(std::vector<VertexIn*>& list ) { mList = &list;}
        void DrawTriangleIndexes(int number, int * indexes);
        void Clear(kechorender::math::Vector3 color);
        void CreateZBuffer();
        void ClearZBuffer();
        void DestroyZBuffer();
        
    private:
        void Triangulate(VertexIn& a, VertexIn& b, VertexIn& c);
        kechorender::core::FrameBuffer * mTarget;
        IRenderShader * mShader;
        std::vector<VertexIn*> * mList;
        float * mZBuffer;
    };
}
}

#endif
