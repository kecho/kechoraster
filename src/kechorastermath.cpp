#include "kechorastermath.h"


void kechorender::math::RotateMatrix(const kechorender::math::Vector3& axis, float ammount, kechorender::math::Matrix44& result)
{
    float p = axis.x() * ammount; 
    float t = axis.y() * ammount; 
    float r = axis.z() * ammount; 
    float cosR = cosf(r);
    float sinR = sinf(r);
    float cosP = cosf(p);
    float sinP = sinf(p);
    float cosT = cosf(t);
    float sinT = sinf(t);
    result = (float[]){
        cosT*cosR, -cosP*sinR + sinP*sinT*cosR, sinP*sinR + cosP*sinT*cosR, 0,
        cosT*sinR, cosP*cosR + sinP*sinT*sinR, -sinP*cosR + cosP*sinT*sinR, 0,
        -sinT    , sinP*cosT,                  cosP*cosT,                   0,
        0        ,         0,                   0,                          1};
}

void kechorender::math::TranslateMatrix(const kechorender::math::Vector3& translation, kechorender::math::Matrix44& result)
{
    result[0][3] = translation.x();
    result[1][3] = translation.y();
    result[2][3] = translation.z();
    result[3][3] = 1.0f;
}

void kechorender::math::ScaleMatrix(const kechorender::math::Vector3& scale, kechorender::math::Matrix44& result)
{
    result[0][0] = (float)result[0][0] * scale.x();
    result[1][1] = (float)result[1][1] * scale.y();
    result[2][2] = (float)result[2][2] * scale.z();
}

void kechorender::math::ViewMatrix(
    const kechorender::math::Vector3& eye,
    const kechorender::math::Vector3& target,
    const kechorender::math::Vector3& up,
    kechorender::math::Matrix44& result
)
{
    kechorender::math::Vector3 t, nx, ny, nz;
    nz = (target - eye).Normalize();
    t = Cross(nz, up);
    nx = t.Normalize();
    ny = Cross(nx, nz);
    result = (float[])
    {
        nx.x(), nx.y(), nx.z(), -eye.x(),
        ny.x(), ny.y(), ny.z(), -eye.y(),
        nz.x(), nz.y(), nz.z(), -eye.z(),
        0, 0, 0, 1
    };
}

kechorender::math::Vector3 kechorender::math::Cross(const kechorender::math::Vector3& a, const kechorender::math::Vector3& b)
{
    return (float[])
    {
        a.y()*b.z() - a.z()*b.y(),
        a.x()*b.z() - a.z()*b.x(),
        a.x()*b.y() - a.y()*b.x()
    };
}

void kechorender::math::Frustum(float l, float r, float b, float t, float n, float f, kechorender::math::Matrix44& result)
{
    result = (float[])
    {
        ((2*n)/(r - l)),0            , (r+l)/(r-l)   ,0,
        0              ,((2*n)/(t-b)), (t+b)/(t-b)   ,0,
        0              ,0            , -((f+n)/(f-n)), -((2*f*n)/(f-n)),
        0              ,0            , -1            ,0
    };
}

