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
//  description : // simple templated 2D and 3D vector classes
//  modified : 
//--------------------------------------------------------------


#ifndef SMALL_VECTORS_H
#define SMALL_VECTORS_H

#include <math.h>
#include <iostream>

//-- defines for frequently used instantiations 

#define Vector2f Vector2<float>
#define Vector2d Vector2<double>
#define Vector2i Vector2<int>
#define Vector3f Vector3<float>
#define Vector3d Vector3<double>
#define Vector3i Vector3<int>

//------------------ 3D vector ------------------- 

//3D vector 
template<class T> 
class Vector3 {
public:
	T x, y, z;
	//c onstructors
	Vector3() {};
	inline Vector3(T xv, T yv, T zv)  { x = xv;  y = yv; z = zv; };
	inline Vector3(T *p) {x = p[0]; y = p[1]; z = p[2]; };

	~Vector3() {};

	// functions
	inline void Set(T xv, T yv, T zv)  { x = xv;  y = yv; z = zv; };

	// simple vector operations
	
	inline Vector3 operator-(const Vector3 &v) const		//subtraction
	{	return Vector3(x - v.x, y - v.y, z - v.z); 
	};

	inline Vector3 operator+(const Vector3 &v) const		//addition
	{	return Vector3(x + v.x, y + v.y, z + v.z); 
	}

	inline void operator+=(const Vector3 &v)				//in place addition 
	{	x += v.x; y += v.y; z += v.z;
	}

	inline void operator-=(const Vector3 &v)				//in place subtraction
	{	x -= v.x; y -= v.y; z -= v.z;
	}

	inline Vector3 operator*(const T s)	const				//scalar multiplication
	{	return Vector3(x * s, y * s, z * s); 
	}

	inline Vector3 operator/(const T s)	const				//scalar division
	{	return Vector3(x / s, y / s, z / s); 
	}

	inline void operator*=(const T s)						//in place scalar multiplication 
	{	x *= s; y *= s; z *= s;
	}

	inline void operator/=(const T s)						//in place scalar division
	{	x /= s; y /= s; z /= s;
	}

	inline T Dot(const Vector3 &v) const					//dot product with another vector
	{	return(v.x * x + v.y * y + v.z * z);
	}

	inline Vector3 Norm() const 							//return normalized vector
	{	T mag = sqrt(x*x + y*y + z*z);
		return(*this * (1/mag));
	}

	inline Vector3 operator*(const Vector3 &v) const		//cross product with another vector
	{	return(Vector3( y * v.z - z * v.y, 
						 z * v.x - x * v.z,
						 x * v.y - y * v.x) );
	}

	inline T Mag() const									//magnitude
	{	return sqrt(x * x + y * y + z * z);
	}

	inline T MagSq() const									//magnitude squared
	{	return x * x + y * y + z * z;
	}

	inline void Print() const
	{	std::cout << "(" << x << ", " << y << ", " << z << ")" << std::endl;
	}

	inline T &operator[](const int i)						//vector access by index
	{	
		T *p = &x;
		return p[i];
	}

	inline bool operator==(const Vector3 &v)				//equality
	{	return v.x == x && v.y == y && v.z == z;
	}
};


//multiplication by a prefix scalar 
template<class T>
inline Vector3<T> operator*(const T s, const Vector3<T> &v)
{	return Vector3<T>(s * v.x, s * v.y, s * v.z);
}


//absolute value
template<class T>
inline Vector3<T> abs(Vector3<T> &v)
{	return Vector3<T>(abs(v.x), abs(v.y), abs(v.z));
}


//------------------ 2D vector ------------------- 

template<class T> 
class Vector2 {
public:
	T x, y;
	//constructors
	Vector2() {};
	inline Vector2(T xv, T yv)  { x = xv;  y = yv; };
	inline Vector2(T *p) {x = p[0]; y = p[1]; };
	~Vector2() {};
	
	//functions

	inline void Set(T xv, T yv)  { x = xv;  y = yv; };

	//simple vector operations
	
	inline Vector2 operator-(const Vector2 &v)	const		//subtraction
	{	return Vector2(x - v.x, y - v.y); 
	};

	inline Vector2 operator+(const Vector2 &v)	const		//addition
	{	return Vector2(x + v.x, y + v.y); 
	}

	inline Vector2 operator*(const T s)	const				//scalar multiplication
	{	return Vector2(x * s, y * s); 
	}

	inline Vector2 operator/(const T s)	const				//scalar division
	{	return Vector2(x / s, y / s); 
	}

	inline T Dot(const Vector2 &v)	const					//dot product with another vector
	{	return(v.x * x + v.y * y);
	}

	inline Vector2 Norm()	const							//return normalized vector
	{	T mag = sqrt(x*x + y*y);
		return(*this * (1/mag));
	}

	inline T operator*(const Vector2 &v) const				//cross product with another vector
	{	return(x * v.y - y * v.x);
	}

	inline T Mag()	const									//magnitude
	{	return sqrt(x * x + y * y);
	}

	inline T MagSq() const									//magnitude squared
	{	return x * x + y * y;
	}

	T &operator[](int i) 									//vector access by index
	{	T *p = &x;
		return p[i];
	}

	bool operator==(Vector2 &v) const
	{	return (x == v.x && y == v.y);
	}

	inline void Print() const
	{	std::cout << "(" << x << ", " << y << ")" << std::endl;
	}

};

//multiplication by a prefix scalar 
template<class T>
inline Vector2<T> operator*(const T s, const Vector2<T> &v)
{	return Vector2<T>(s * v.x, s * v.y);
}

//absolute value
template<class T>
inline Vector2<T> abs(Vector2<T> &v)
{	return Vector2<T>(abs(v.x), abs(v.y));
}

#endif
