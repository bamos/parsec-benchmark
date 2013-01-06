//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                           
//	 © 2006, Intel Corporation, licensed under Apache 2.0 
//
//  file :	 Dmatrix.h
//  author : Scott Ettinger - scott.m.ettinger@intel.com
//  description : Displacement matrix and vector math 
//				  operations.
//  modified : 
//--------------------------------------------------------------


#ifndef DMATRIX_H
#define DMATRIX_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <cstring>
#include <iostream>
#include "SmallVectors.h"

typedef enum {X, Y, Z} DMAxis;
typedef enum {XYZ, ZYX, YXZ, ZXY} DMOrder;

//simple 3D Displacement Matrix class
template<class T>
class DMatrix{
protected:
	T data[3][4];
public:

	DMatrix() {};
	
	//constructor to initialize diagonal elements 
	DMatrix(T d1, T d2, T d3);

	//constructor to initialize diagonal and translation elements
	DMatrix(T d1, T d2, T d3, T t1, T t2, T t3);

	//constructor given a pointer to data (copies the data)
	DMatrix(const T *p) { memcpy(data, p, 12 * sizeof(T)); };

	//constructor given an orthogonal axis and an angle in radians
	DMatrix(DMAxis axis, T angle);

	//constructor given a set of Euler angles in radians
	DMatrix(float theta, float phi, float tau, DMOrder order);

	//destructor
	~DMatrix() {};

	///set to zero
	void Clear() {memset(data, 0, sizeof(T) * 12); };
	
	///set to identity
	void SetIdentity() {Clear(); data[0][0] = (T)1; data[1][1] = (T)1; data[2][2] = (T)1; };
	
	///element access
	inline T &operator()(int r, int c) {return data[r][c]; };
	
	///set to given data
	void Set(const T *p) { memcpy(data, p, 12 * sizeof(T)); };
	
	///set translation vector of Displacement matrix
	void SetTranslation(const T t1, const T t2, const T t3) { data[0][3] = t1; data[1][3] = t2; data[2][3] = t3; };
	
	///display matrix
	void Print(std::ostream &s) {s << data[0][0] << " " << data[0][1] << " " << data[0][2] << " " << data[0][3] << std::endl;
				  s << data[1][0] << " " << data[1][1] << " " << data[1][2] << " " << data[1][3] << std::endl;
				  s << data[2][0] << " " << data[2][1] << " " << data[2][2] << " " << data[2][3] << std::endl; };
};

//------------- Operators Supported ---------------

//between Dmatrices				:	+   -   *   += 
//between Dmatrix and scalar	:	*   *=
//between Dmatrix and Vector3	:	*



//-------------- Related functions ----------------------------------------------------------

//Displacement matrix inverse
template<class T>
DMatrix<T> Inverse(const DMatrix<T> &m);

//Frobenius norm squared of matrix
template<class T>
T FrobNorm2(const DMatrix<T> &m);

//Create a rotation matrix around a given axis
template<class T>
void AxisRotation(DMatrix<T> &m, DMAxis axis, T angle);

//Create a rotation matrix given Euler angles and axis order
template<class T>
void EulerRotation(DMatrix<T> &m, float theta, float phi, float tau, int order);




//--------------------------------- Implementation -------------------------------------------

//Constructor initializing diagonal elements
template<class T>
DMatrix<T>::DMatrix(T d1, T d2, T d3)
{	Clear();
	data[0][0] = d1; data[1][1] = d2; data[2][2] = d3;
}

//Constructor initializing diagonal elements and translation
template<class T>
DMatrix<T>::DMatrix(T d1, T d2, T d3, T t1, T t2, T t3)
{	Clear();
	data[0][0] = d1; data[1][1] = d2; data[2][2] = d3;
	data[0][3] = t1; data[1][3] = t2; data[2][3] = t3;
}

//Constructor given an axis and an angle
template<class T>
DMatrix<T>::DMatrix(DMAxis axis, T angle)
{	AxisRotation(*this, axis, angle);
}

//Constructor given a set of Euler angles and rotation order
template<class T>
DMatrix<T>::DMatrix(float theta, float phi, float tau, DMOrder order)
{	EulerRotation(*this, theta, phi, tau, order);
}

//--------------------------------- Arithmetic Operators --------------------------------------

//Matrix-vector product with normalized coordinates (3x4 Matrix * 3x1 vector)
template<class T1, class T2>
inline Vector3<T1> operator*(const DMatrix<T2> &dm, const Vector3<T1> &v)
{
	Vector3<T1> r;
	DMatrix<T2> &m = *((DMatrix<T2> *)&dm);
	r.x = m(0,0) * v.x + m(0,1) * v.y + m(0,2) * v.z + m(0,3);
	r.y = m(1,0) * v.x + m(1,1) * v.y + m(1,2) * v.z + m(1,3);
	r.z = m(2,0) * v.x + m(2,1) * v.y + m(2,2) * v.z + m(2,3);
	return r;
}

//Matrix Subtraction
template<class T> 
inline DMatrix<T> operator-(const DMatrix<T> &m1, const DMatrix<T> &m2)
{
	DMatrix<T> r;
	T *pr = (T *)r.data, *p1 = (T *)m1.data, *p2 = (T *)m2.data;
	for(int i = 0; i < 12; i++)
		*(pr++) = *(p1++) - *(p2++);
	return(r); 
}

//Matrix Addition
template<class T> 
inline DMatrix<T> operator+(const DMatrix<T> &m1, const DMatrix<T> &m2)
{
	DMatrix<T> r;
	T *pr = (T *)r.data, *p1 = (T *)m1.data, *p2 = (T *)m2.data;
	for(int i = 0; i < 12; i++)
		*(pr++) = *(p1++) + *(p2++);
	return(r);
}

//Matrix Addition in place +=
template<class T> 
inline void operator+=(DMatrix<T> &m1, const DMatrix<T> &m2)
{
	T *p1 = (T *)m1.data, *p2 = (T *)m2.data;
	for(int i = 0; i < 12; i++)
		*(p1++) += *(p2++);
}

//Matrix Subtraction in place -=
template<class T> 
inline void operator-=(DMatrix<T> &m1, const DMatrix<T> &m2)
{
	T *p1 = (T *)m1.data, *p2 = (T *)m2.data;
	for(int i = 0; i < 12; i++)
		*(p1++) -= *(p2++);
}

//Scalar multiplication in place 
template<class T>
inline void operator*=(DMatrix<T> &m, T s)
{	for(int i = 0; i < 12; i++) 
		((T *)m.data)[i] *= s;
}

//Scalar multiplication
template<class T>
inline DMatrix<T> operator*(DMatrix<T> m, T s)
{	m *= s;
	return m;
}

//Matrix-Matrix product 
template<class T>
inline DMatrix<T> operator*(const DMatrix<T> &dm1, const DMatrix<T> &dm2)
{
	DMatrix<T> m3, &m1 = *((DMatrix<T> *)&dm1), &m2 = *((DMatrix<T> *)&dm2);	//get non-const references to dm1 and dm2 to allow () operator
	for(int r = 0; r < 3; r++)
		for(int c = 0; c < 4; c++)
			m3(r,c) = m1(r,0) * m2(0,c) + m1(r,1) * m2(1,c) + m1(r,2) * m2(2,c);
	m3(0,3) += m1(0,3); m3(1,3) += m1(1,3); m3(2,3) += m1(2,3);
	return m3;
}

//---------------------------- Mathematical Functions ------------------------------------------

//Frobenius norm squared of matrix
template<class T>
T FrobNorm2(DMatrix<T> &m)
{	T *p = (T *)m.data, r = 0;
	for(int i = 0; i < 12; i++)
	{	r += *p * *p;
		p++;
	}
	return r;
}

//Displacement matrix inverse
template<class T>
DMatrix<T> Inverse(const DMatrix<T> &dm)
{
	DMatrix<T> r, &m = *((DMatrix<T> *)&dm);
	r.Clear();
	T k1 = m(1,1) * m(2,2) - m(1,2) * m(2,1);						//calculate rotation matrix determinant
	T k2 = m(1,0) * m(2,2) - m(1,2) * m(2,0);
	T k3 = m(1,0) * m(2,1) - m(1,1) * m(2,0);
	T c = (T)1.0 / (m(0,0) * k1 - m(0,1) * k2 +	m(0,2) * k3);
	r(0,0) = c * k1;												//calculate inverse of rotation matrix
	r(0,1) = c * (m(0,2) * m(2,1) - m(0,1) * m(2,2));
	r(0,2) = c * (m(0,1) * m(1,2) - m(0,2) * m(1,1));
	r(1,0) = c * -k2;
	r(1,1) = c * (m(0,0) * m(2,2) - m(0,2) * m(2,0));
	r(1,2) = c * (m(0,2) * m(1,0) - m(0,0) * m(1,2));
	r(2,0) = c * k3;
	r(2,1) = c * (m(0,1) * m(2,0) - m(0,0) * m(2,1));
	r(2,2) = c * (m(0,0) * m(1,1) - m(0,1) * m(1,0));
	Vector3<T> v(-m(0,3), -m(1,3), -m(2,3));						//calculate new translation vector
	Vector3<T> t = r * v;
	r(0,3) = t.x;  r(1,3) = t.y; r(2,3) = t.z;
	return r;
}

//--------------------------------- Rotation Matrix functions -----------------------------------

//generate a rotation matrix about a given axis
template<class T>
inline void AxisRotation(DMatrix<T> &m, DMAxis axis, T angle)
{	m.Clear();
	float c = cos(angle), s = sin(angle);
	switch(axis)
	{	case X : m(0,0) = 1; m(1,1) = c; m(1,2) = -s; m(2,1) = s; m(2,2) = c;
				 break;
		case Y : m(0,0) = c; m(0,2) = s; m(1,1) = 1; m(2,0) = -s; m(2,2) = c;
				 break;
		case Z : m(0,0) = c; m(0,1) = -s; m(1,0) = s; m(1,1) = c; m(2,2) = 1;
				 break;
	}
}

//Constructor given a set of Euler angles
template<class T>
void EulerRotation(DMatrix<T> &m, float theta, float phi, float tau, DMOrder order)
{	DMatrix<T> Rx(X, theta), Ry(Y, phi), Rz(Z, tau);
	switch (order)
	{	case XYZ:
			m = (Rx * Ry) * Rz;	break;
		case ZYX:
			m = (Rz * Ry) * Rx;	break;
		case YXZ:
			m = (Ry * Rx) * Rz;	break;
		case ZXY:
			m = (Rz * Rx) * Ry;	break;
		default:
			m = (Rx * Ry) * Rz;	break;
	}
}



#endif
