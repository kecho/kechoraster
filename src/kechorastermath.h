#ifndef __KECHO_RASTER_MATH_H_
#define __KECHO_RASTER_MATH_H_
#include "stdio.h"
#include "math.h"
namespace kechorender
{
namespace math
{
    template<class T> T abs(T v)
    {
        return v < 0.0 ?  -v : v;
    }
    template <class T> class IDataArray
    {
    public:
        IDataArray(){}
        virtual ~IDataArray(){}
        virtual T Get(int i) const  = 0;
        virtual T* GetRaw() = 0;
        virtual const T* GetRaw() const = 0;
        virtual void Set(int i, const T& v) = 0;
        virtual int Size() const = 0;
    };
    template <class T>
    class DataCell
    {
    public:
        DataCell(T * data, int idx)
        {
            val = data + idx;
        }
        virtual T operator= (const T& v)
        {
            *val = v;
        }

        virtual operator T() const
        {
            return *val;
        }
        
    private:
        T * val; 
    };
    
    template <class T>
    class ConstDataCell
    {
    public:
        ConstDataCell(const T * data, int idx)
        :
        val(data + idx)
        {
        }

        virtual operator T() const
        {
            return *val;
        }
        
    private:
        const T * val; 
    };

    template <unsigned N, class T>
    class Vector : public IDataArray<T>
    {
    public:
        Vector(float * data){ for (int i = 0; i < N; ++i) mData[i] = data[i]; }
        Vector(){}
        virtual ~Vector(){}
        virtual T * GetRaw(){return mData;}
        const T * GetRaw() const {return mData;}
        virtual Vector<N,T>& operator = (const Vector<N,T>& other) {for (int i = 0; i < N; ++i) mData[i] = other.mData[i]; return *this;}
        virtual Vector<N,T>& operator = (const float * other) {for (int i = 0; i < N; ++i) mData[i] = other[i]; return *this;}
        virtual Vector<N,T> operator + (const Vector<N,T>& other) const
        {
            Vector<N,T> result;
            for (int i = 0; i < N; ++i) result[i] = mData[i] + other.mData[i];
            return result;
        }
        virtual Vector<N,T> operator - (const Vector<N,T>& other) const
        {
            Vector<N,T> result;
            for (int i = 0; i < N; ++i) result[i] = mData[i] - other.mData[i];
            return result;
        }
        virtual Vector<N,T> operator - (T f) const
        {
            Vector<N,T> result;
            for (int i = 0; i < N; ++i) result[i] = mData[i] / f;
            return result;
        }
        virtual DataCell<T> operator[] (int idx) { return DataCell<T>(mData, idx);}        
        virtual const ConstDataCell<T> operator[] (int idx) const { return ConstDataCell<T>(mData, idx);}        
        //virtual Vector<N,T> operator * (const Vector<N,T>& v) const;/* cross product, not implementd until needed*/
        T dot(const Vector<N,T>&) const;
        T norm() const { return sqrtf(kechorender::math::abs(dot(*this))); }
        Vector<N,T> Normalize() const {
            Vector<N,T> result;
            T n = norm();
            for (int i = 0; i < N; ++i)
            {
                result.mData[i] = mData[i] / n; 
            }
            return result;
        }
        T x() const {return mData[0];}
        T y() const {return mData[1];}
        T z() const {return mData[2];}
        T w() const {return mData[3];}
        T r() const {return mData[0];}
        T g() const {return mData[1];}
        T b() const {return mData[2];}
        T a() const {return mData[3];}
        virtual T Get(int i ) const { return mData[i];}
        virtual void Set(int i, const T& v ) { mData[i] = v;}
        virtual int Size() const {return N;}
    private:
        T mData[N];
    };

    template <unsigned N, class T>
    class ConstMatrixRow
    {
    public:
        ConstMatrixRow(const T * valueList, int idx)
         :   row(valueList + (idx * N))
        {
        }
        const T * GetRaw() const {return row;}
        virtual ConstDataCell<T> operator[](int idx){return ConstDataCell<T>(row, idx);}
        virtual operator const T*() const{return GetRaw();}
    private:
        const T * row;

    };

    template <unsigned N, class T>
    class MatrixRow
    {
    public:
        MatrixRow(T * valueList, int idx)
         :
         row ( valueList + (idx * N) )
        {
        }
        T * GetRaw(){return row;}
        const T * GetRaw() const {return row;}
        virtual DataCell<T> operator[](int idx){return DataCell<T>(row, idx);}
        virtual operator T*(){return GetRaw();}
        virtual operator const T*() const{return GetRaw();}
    private:
        T * row;

    };

    template <unsigned R, unsigned C, class T = float>
    class Matrix : public IDataArray<T>
    {
    public:
    	Matrix(T * buff)
    	{
            for (int i = 0; i < R*C; ++i)
            {
                mData[i] = buff[i]; 
            } 
    	}
        Matrix(){}
        virtual ~Matrix(){}
        virtual MatrixRow<C,T> operator[](int idx) { return MatrixRow<C,T>(mData, idx);} 
        virtual const ConstMatrixRow<C,T> operator[](int idx) const { return ConstMatrixRow<C,T>(mData, idx);} 
        virtual T* GetRaw(){return mData;}
        virtual const T* GetRaw() const {return mData;}
        virtual Vector<R,T> operator * (const Vector<R,T>&);
        Matrix<R,C,T> inv () const;
        virtual Matrix<R,C,T>& operator= (const Matrix<R,C,T>& other)
        {
            for (int i = 0; i < R*C; ++i) mData[i] = other.mData[i];
        }

        virtual Matrix<R,C,T>& operator= (const T buffer[R*C])
        {
            for (int i = 0; i < R*C; ++i) mData[i] = buffer[i];
        }
        virtual T Get(int i ) const { return mData[i];}
        virtual void Set(int i, const T& v ) { mData[i] = v;}
        virtual int Size() const {return R*C;}
    
    private:
        T mData[R*C];
    };

    
    template<unsigned N, class T> T Vector<N,T>::dot(const Vector<N,T>& other) const
    {
        T result = 0;
        for (int i = 0; i < N; ++i) result += (*this)[i] * other[i];
        return result;
    }

    template<unsigned R, unsigned C, class T> Vector<R,T> Matrix<R,C,T>::operator * (const Vector<R,T>& v)
    {
        const T * vect  = v.GetRaw();
        Vector<R,T> res;  
        for (int i = 0; i < R; ++i)
        {
            const T * r = (*this)[i];        
            T result = 0;
            for (int j = 0; j < C; ++j) result += r[j] * vect[j];
            res[i] = result;
        }
        return res;
    }

    template<unsigned R, unsigned C, unsigned Z, class T> void matmul(const Matrix<R,C,T>& A, const Matrix<C,Z,T>&B, Matrix<R,Z,T>& res)
    {
        for (int i = 0; i < R; ++i)
        {
            for (int j = 0; j < Z; ++j)
            {
                T val = 0;
                for (int k = 0; k < C; ++k)
                {
                    val += A[i][k]*B[k][j];
                }
                res[i][j] = val;
            }
        }
    }
    
    template<unsigned N, class T> void _matinv(T ** M, T ** result );

    template<unsigned R, unsigned C, class T> Matrix<R,C,T> Matrix<R,C,T>::inv() const
    {
    	Matrix<R,C,T> resultCpy;
    	Matrix<R,C,T> result;
    	Matrix<R,C,T> mCpy = *this;
    	T* resultMat[R];
    	T* mCpyMat[R];

    	for (int i = 0; i < R; ++i)
        {
	        resultMat[i] = resultCpy[i];	
	        mCpyMat[i] = mCpy[i];	
    	}
        _matinv<R,T>(mCpyMat, resultMat);
        for (int i = 0; i < R; ++i)
        {
            for (int j = 0; j < C; ++j)
            {
                result[i][j] = resultMat[i][j];
            }
        }
        return result;

    }

template<unsigned N, class T> void mulrow(T ** r1, T ** r2, int r, T f)
{
    for (int i = 0; i < N; ++i)
    {
        r1[r][i] *= f;
        r2[r][i] *= f;
    }
}

template <unsigned N, class T> void addrow(int row, T ** r1, T ** r2, int resultRows)
{
    for (int i = 0 ; i < N; ++i)
    {
        r1[resultRows][i] += r1[row][i]; 
        r2[resultRows][i] += r2[row][i]; 
    }
}

template<class T> void swaprow(T ** M ,int i, int j)
{
    T * tmp = M[i];
    M[i] = M[j];
    M[j] = tmp;
}

template<unsigned N, class T> void pmat(T ** M)
{
    for (int i = 0; i < 3; ++i)
    {
        printf("r(%p) ", M[i]);
        for (int j = 0; j < 3; ++j)
        {
            printf("%f : ", (float)M[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
template <unsigned N, class T> void _matinv(T ** M, T ** result)
{
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j) result[i][j] = i == j ? 1 : 0;
    }
    for (int i = 0; i < N; ++i)
    {
        //find max row
        for (int k = i+1; k < N; ++k)
        {
            if (abs(M[i][i]) < abs(M[k][i]))
            {
                swaprow<T>(M,i,k);
                swaprow<T>(result,i,k);
            } 
        }
        //zero out columns
        for (int j = i + 1; j < N; ++j)
        {
            T d = M[i][i];
            if (M[j][i] == 0.0f)continue;
            T factor = -d / M[j][i];
            mulrow<N,T>(M,result,j,factor);
            addrow<N,T>(i,M,result,j);
        }
    }

    for (int i = 0; i < N; ++i)
    {
        for (int j = i + 1; j < N; ++j)
        {
            if (M[i][j] == 0.0f) continue;
            T factor = -M[i][j] / M[j][j];
            mulrow<N,T>(M,result,j,factor);
            addrow<N,T>(j,M,result,i);
        } 
        mulrow<N,T>(M,result,i,1.0f/M[i][i]);
    }
}


    typedef Matrix<3,3,float> Matrix33;
    typedef Matrix<4,4,float> Matrix44;
    typedef Vector<4,float> Vector4;
    typedef Vector<3,float> Vector3;
    typedef Vector<2,float> Vector2;
    Vector3 Cross(const Vector3& a, const Vector3& b);
    void RotateMatrix(const Vector3& axis, float ammount, Matrix44& result);
    void ScaleMatrix(const Vector3& scale, Matrix44& result);
    void TranslateMatrix(const Vector3& translation, Matrix44& result);
    void ViewMatrix(const Vector3& eye, const Vector3& target, const Vector3& up, Matrix44& result);
    void Frustum(float l, float r, float b, float t, float n, float f, Matrix44& result);
}
}
#endif
