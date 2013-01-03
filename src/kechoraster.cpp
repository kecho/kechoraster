#include "kechoraster.h"
#include "framebuffer.h"
#include "stdio.h"

#define NORMALIZE_DEV_COORDS(x) ((x + 1.0f) / 2.0f)
#define TO_DEV_COORDS(p,P) static_cast<float>(static_cast<float>(p)/static_cast<float>(P) * 2.0f -1.0f)

using namespace kechorender::math;
using namespace kechorender::render;
using namespace kechorender::core;

void printMat(Matrix33& m)
{
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            printf("%f ", (float)m[i][j]);
        }
        printf("\n");
    }
   printf("\n");
    
}
Interpolation::Interpolation(const PixelIn& a, const PixelIn& b, const PixelIn& c)
{

    Vector2 A = (float[]){a.position.x() / a.position.w(), a.position.y() / a.position.w()};
    Vector2 B = (float[]){b.position.x() / b.position.w(), b.position.y() / b.position.w()};
    Vector2 C = (float[]){c.position.x() / c.position.w(), c.position.y() / c.position.w()};
    Matrix33 equations = (float[])
    {
         A.x(), A.y(), 1.0f,
         B.x(), B.y(), 1.0f,
         C.x(), C.y(), 1.0f
    };
    mInterpolationMat = equations.inv();
    for (int i = 0; i < a.GetNumOfAttributes(); ++i)
    {
       RegisterScalar(*a.GetAttribute(i), *b.GetAttribute(i), *c.GetAttribute(i)); 
    }
    
}

Interpolation::~Interpolation()
{
}

void Interpolation::Interpolate(const Vector2& position, PixelIn& p)
{
    for (int a = 0; a < p.GetNumOfAttributes(); ++a)
    {
        float *att = p.GetAttribute(a);
        *att = InterpolateScalar(a,position); 
    }
}


int Interpolation::RegisterScalar(float a, float b, float c)
{
    int id = mAttributes.size();  
    Vector3 equation = (float[]){a,b,c};
    Vector3 result = mInterpolationMat * equation;
    mAttributes.push_back(result);
    return id;
}
float Interpolation::InterpolateScalar(int id, const Vector2& pos)
{
    Vector3 attr = mAttributes[id]; 
    Vector3 newPos = (float[]){pos.x(), pos.y(), 1.0f };
    return attr.dot(newPos);
}


PixelIn::PixelIn()
{
    float * zReference = position.GetRaw() + 2;  //last value, z
    RegisterAttribute(zReference);
    float * wReference = position.GetRaw() + 3;  //last value, w
    RegisterAttribute(wReference);
}

int PixelIn::RegisterAttribute(float * scalarPointer) 
{
    int id = mScalarReferences.size(); 
    mScalarReferences.push_back(scalarPointer);
    return id;
}

int PixelIn::RegisterAttribute(IDataArray<float>& a)
{
    int id = mScalarReferences.size(); 
    for (int i = 0; i < a.Size(); ++i)
    {
        RegisterAttribute(a.GetRaw() + i);
    }
    return id;
}

KechoDevice::KechoDevice() :
mTarget(0), mShader(0), mList(0), mZBuffer(0)
{
    
}

KechoDevice::~KechoDevice()
{
    DestroyZBuffer();
}

void printV(PixelIn * v)
{
    printf("%f %f %f %f\n",
        v->position.x() ,
        v->position.y() ,
        v->position.z() ,
        v->position.w() );
}

void KechoDevice::DrawTriangleIndexes(int size, int * indexes)
{
    if (mShader && mTarget && mList && indexes && size >= 3)
    {
        for (int i = 0; i < size; i+=3)
        {
            VertexIn * A = (*mList)[indexes[i]]; 
            VertexIn * B = (*mList)[indexes[i+1]]; 
            VertexIn * C = (*mList)[indexes[i+2]]; 
            Triangulate(*A,*B,*C);
        }
    }
}

struct Point
{
    int x;
    int y;
};

void get_pixel_pos(PixelIn& pixelCoords, Point& deviceCoords, FrameBuffer * target)
{
    Vector2 pos = (float[]){
        pixelCoords.position.x() / pixelCoords.position.w(),
        pixelCoords.position.y() / pixelCoords.position.w(),
    };
    
    deviceCoords.x = static_cast<int>(NORMALIZE_DEV_COORDS(pos.x()) * static_cast<float>(target->GetW()));
    deviceCoords.y = static_cast<int>(NORMALIZE_DEV_COORDS(pos.y()) * static_cast<float>(target->GetH()));
}

struct Edge
{
    Point * A;
    Point * B;
    int incX;
    float incError;
    float dX;
    float dY;
    float w;//inverse of slope dx / dy
};

struct EdgeState
{
    int x;
    float errorCumulation;    
};

void add_x_to_edge_state(EdgeState& s, Edge& e)
{
    s.x += e.incX;
    s.errorCumulation += e.incError;
    if (s.errorCumulation >= 1.0)
    {
        s.errorCumulation -= 1;
        s.x += e.w > 0 ? 1 : -1;
    }
    
}

void swap_edge_points(Edge& e)
{
    Point * tmp = e.A;
    e.A = e.B;
    e.B = tmp;
}

void init_edge(Edge& e, Point& a, Point& b)
{
    e.A = &a;
    e.B = &b;
    //sort the edge points
    if (e.A->y > e.B->y)
    {
        swap_edge_points(e);
    }
    else if (e.A->y == e.B->y)
    {
        if (e.A->x > e.B->x)
        {
            swap_edge_points(e);
        }
    }
    e.w = static_cast<float>(e.A->x - e.B->x) / static_cast<float>(e.A->y - e.B->y);
    e.incX = static_cast<int>(e.w);
    e.incError = (e.w - static_cast<float>(e.incX));
    e.incError *= e.incError < 0 ? -1.0f : 1.0f; //abs
    e.dX = abs(e.A->x - e.B->x);
    e.dY = abs(e.A->y - e.B->y);
}

void swap_int(int * list, int a, int b)
{
    int tmp = list[a];
    list[a] = list[b];
    list[b] = tmp;
}

void sort_edge_list(Edge * list, int * order)
{
    //sort by y first
    for (int i = 0; i < 3; ++i)
    {
        for (int j = i + 1; j < 3; ++j)
        {
            if (list[order[j]].A->y < list[order[i]].A->y)
            {
                swap_int(order, i, j);
            }
        }
    }
    if (list[order[0]].w > list[order[1]].w)
    {
        swap_int(order, 0, 1);
    }
}

bool passZTest(float * zbuffer, int x, int y, float z, int W)
{
    if (zbuffer == 0) return true;
    if ((z - zbuffer[y*W + x] ) > 0)
    {
        zbuffer[y*W + x] = z;
        return true;
    } 
    return false;

}
void drawHorizontalLine(FrameBuffer& fb, int y, int cxl, int cxr, PixelIn&  current, Interpolation& interpolator, IRenderShader * shader, float * zbuffer)
{
    if (y < 0 || y >= fb.GetH())
    {
        return;
    }
    int xl = cxl;
    int xr = cxr;
    if (xl > xr)
    {
        int t = xl;
        xl = xr;
        xr = t;
    }
    for (int x = xl; x <= xr; ++x) 
    {
        if (x < 0 || x > fb.GetW() )
        {
            continue;
        }
        Vector2 pos = (float[]){
            TO_DEV_COORDS(x,fb.GetW()),
            TO_DEV_COORDS(y, fb.GetH())
        };
        
        interpolator.Interpolate(pos, current);
        PixelOut out;
        if (passZTest(zbuffer,x,y,current.position.z() / current.position.w(),fb.GetW()))
        {
            shader->OnPixel(current,out);
            Pixel * p = fb.GetPixel(x,y);
            p->r = static_cast<float>(255) * out.r;
            p->g = static_cast<float>(255) * out.g;
            p->b = static_cast<float>(255) * out.b;
            p->a = static_cast<float>(255) * out.a;
        }
    }
}

void printPixelPos(Point& p)
{
    printf("%d %d\n", p.x, p.y);
}

void KechoDevice::Triangulate(VertexIn& va, VertexIn& vb, VertexIn& vc)
{
    PixelIn * pcurrent = mShader->CreatePixelInput(); 
    PixelIn * pa = mShader->CreatePixelInput(); 
    PixelIn * pb = mShader->CreatePixelInput(); 
    PixelIn * pc = mShader->CreatePixelInput(); 

    mShader->OnVertex(va, *pa);
    mShader->OnVertex(vb, *pb);
    mShader->OnVertex(vc, *pc);

    Interpolation interpolator(*pa, *pb, *pc);
    
    Point a, b, c; 
    get_pixel_pos(*pa, a, mTarget);
    get_pixel_pos(*pb, b, mTarget);
    get_pixel_pos(*pc, c, mTarget);

    Edge e[3];
    init_edge(e[0], a, b);
    init_edge(e[1], b, c);
    init_edge(e[2], c, a);
    int order[3] = {0, 1, 2};
    sort_edge_list(e, order);
    Edge * left = &e[order[0]];
    Edge * right = &e[order[1]];
    Edge * final = &e[order[2]];
    bool finished = false;
    EdgeState stateLeft = {left->A->x, 0.0f};
    EdgeState stateRight = {right->A->x, 0.0f};
    while (!finished)
    {
        int limitY = left->B->y < right->B->y ? left->B->y : right->B->y ;
        int currentY = left->A->y > right->A->y ? left->A->y : right->A->y;
        for (; currentY < limitY; ++currentY)
        {
            drawHorizontalLine(*mTarget, currentY, stateLeft.x, stateRight.x, *pcurrent, interpolator, mShader, mZBuffer);
            add_x_to_edge_state(stateLeft, *left);
            add_x_to_edge_state(stateRight, *right);
        } 
        
        if (final != 0)
        { 
            if (limitY < left->B->y)
            { 
                right = final;
                stateRight.x = right->A->x;
                stateRight.errorCumulation = 0.0;
                final = 0;
            }
            else
            {
                left = final;
                stateLeft.x = left->A->x;
                stateLeft.errorCumulation = 0.0f;
                final = 0;
            }
        } 
        else
        {
            finished = true;
        }
    } 
    delete pcurrent;    
    delete pa;
    delete pb;
    delete pc;

}

void KechoDevice::Clear(Vector3 color)
{
    for (int x = 0; x < mTarget->GetW(); ++x)
    {
        for (int y = 0; y < mTarget->GetH(); ++y)
        {
            Pixel * p = mTarget->GetPixel(x,y);
            p->r = static_cast<float>(255) * color.r();
            p->g = static_cast<float>(255) * color.g();
            p->b = static_cast<float>(255) * color.b();
            p->a = 255;
        }
    }
}

void KechoDevice::CreateZBuffer()
{
    if (mTarget)
    {
        mZBuffer = new float[mTarget->GetW() * mTarget->GetH()];
    }
}

void KechoDevice::DestroyZBuffer()
{
    if (mZBuffer)
    {
        delete [] mZBuffer;
        mZBuffer = 0;
    }
}


void KechoDevice::ClearZBuffer()
{
    for (int i = 0; i < mTarget->GetH()*mTarget->GetW(); ++i)
    {
        mZBuffer[i] = -100.0f;
    }
}



