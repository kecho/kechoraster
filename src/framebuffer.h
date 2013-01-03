#ifndef __FRAME_BUFFER_H__
#define __FRAME_BUFFER_H__

namespace kechorender
{
namespace core
{
    struct Pixel
    {
        unsigned char b;
        unsigned char g;
        unsigned char r;
        unsigned char a;  
    };

    class FrameBuffer
    {
    public:
        FrameBuffer(int w, int h);
        ~FrameBuffer();
        Pixel * GetBuffer() { return mData;}
        int GetW(){return mW;}
        int GetH(){return mH;}
        Pixel * GetPixel(int x, int y); 
    private:
        Pixel * mData;
        int mW;
        int mH;
    };
}
}

#endif
