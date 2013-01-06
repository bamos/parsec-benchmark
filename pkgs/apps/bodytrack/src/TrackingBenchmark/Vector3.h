//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//
//	 © 2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : 
//  author : Scott Ettinger - scott.m.ettinger@intel.com
//  description : 
//  modified : 
//--------------------------------------------------------------

#ifndef VECTOR3_H
#define VECTOR3_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <math.h>
#include <iostream>

#define Vector3f Vector3<float>
#define Vector3d Vector3<double>
#define Vector3i Vector3<int>

//3D vector 
template<class T> 
class Vector3 {
public:
	T x, y, z;
	///constructors
	Vector3() {};
	Vector3(const T xv, const T yv, const T zv)	{ x = xv;  y = yv; z = zv; };
	Vector3(const Vector3 &v)	{ x = v.x; y = v.y; z = v.z; };		//copy constructor

	~Vector3() {};

	///basic vector operations

	void Set(const T xv, const T yv, const T zv)  { x = xv;  y = yv; z = zv; };

	inline const Vector3 operator-(const Vector3 &v) 	//subtraction
	{	Vector3 r(x - v.x, y - v.y, z - v.z); 
		return(r);
	};

	inline Vector3 operator+(const Vector3 &v)			//addition
	{	Vector3 r(x + v.x, y + v.y, z + v.z); 
		return(r);
	}

	inline void operator+=(const Vector3 &v)			//in place addition 
	{	x += v.x; y += v.y; z += v.z;
	}

	inline void operator-=(const Vector3 &v)			//in place subtraction
	{	x -= v.x; y -= v.y; z -= v.z;
	}

	inline Vector3 operator*(const T s)					//scalar multiplication
	{	Vector3 r(x * s, y * s, z * s); 
		return(r);
	}

	inline Vector3 operator/(const T s)					//scalar division
	{	Vector3 r(x / s, y / s, z / s); 
		return(r);
	}

	inline void operator*=(const T s)					//in place scalar multiplication 
	{	x *= s; y *= s; z *= s;
	}

	inline void operator/=(const T s)					//in place scalar division
	{	x /= s; y /= s; z /= s;
	}


	inline T Dot(const Vector3 &v)						//dot product with another vector
	{	return(v.x * x + v.y * y + v.z * z);
	}

	inline Vector3 Norm()								//return normalized vector
	{	T mag = sqrt(x*x + y*y + z*z);
		return(*this * (1/mag));
	}

	inline Vector3 operator*(const Vector3 &v)			//cross product with another vector
	{	return(Vector3( y * v.z - z * v.y, 
						 z * v.x - x * v.z,
						 x * v.y - y * v.x) );
	}

	inline T Mag()										//magnitude
	{	return sqrt(x * x + y * y + z * z);
	}

	inline T MagSq()									//magnitude squared
	{	return x * x + y * y + z * z;
	}

	void Print(std::ostream &s)
	{	s << "(" << x << ", " << y << ", " << z << ")";
	}

	inline T &operator[](const int i)									//vector access by index
	{	
		T *p = &x;
		return p[i];
	}
};

//print vector to a stream
template<class T>
inline std::ostream &operator<<(std::ostream &s, Vector3<T> &v)
{	v.Print(s);
	return s;
}


#endif

