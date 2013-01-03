#include "framebuffer.h"

using namespace kechorender::core;

FrameBuffer::FrameBuffer(int w, int h)
:
mW(w), mH(h)
{
   mData = new Pixel[w * h]; 
}

FrameBuffer::~FrameBuffer()
{
    delete [] mData;
}

Pixel * FrameBuffer::GetPixel(int x, int y)
{
    return mData + y*mW + x;
}
