//#####################################################################
// Copyright 2002-2005, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Igor Neverov, Eftychios Sifakis, Joseph Teran, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class QUATERNION
//#####################################################################
#ifndef __QUATERNION__
#define __QUATERNION__

#include <math.h>
#include <iostream>
#include "VECTOR_3D.h"
#include "MATRIX_4X4.h"
#include "VECTOR_ND.h"
#include "../Math_Tools/constants.h"
#include "../Math_Tools/clamp.h"
namespace PhysBAM
{

template<class T>
class QUATERNION
{
public:
	T s; // cos(theta/2)
	VECTOR_3D<T> v;

	QUATERNION()
		: s (1) // note that the vector v is set to (0,0,0) by default
	{}

	template<class T2> QUATERNION (const QUATERNION<T2>& q)
		: s ( (T) q.s), v (q.v)
	{}

	QUATERNION (const T s, const T x, const T y, const T z)
		: s (s), v (x, y, z)
	{}

	QUATERNION (const T angle, const VECTOR_3D<T>& direction)
		: s (cos (angle / 2)), v (direction)
	{
		v.Normalize();
		v *= sin (angle / 2);
	}

	QUATERNION (const T euler_angle_x, const T euler_angle_y, const T euler_angle_z)
	{
		*this = QUATERNION<T> (euler_angle_z, VECTOR_3D<T> (0, 0, 1)) * QUATERNION<T> (euler_angle_y, VECTOR_3D<T> (0, 1, 0)) * QUATERNION<T> (euler_angle_x, VECTOR_3D<T> (1, 0, 0));
	}

	QUATERNION (const VECTOR_3D<T>& v_input) // makes a vector into a quaternion (note difference from From_Rotation_Vector!)
		: s (0), v (v_input)
	{}

	QUATERNION (const VECTOR_ND<T>& v_input) // used to make a vector from a 4 vector
		: s (v_input (1)), v (v_input (2), v_input (3), v_input (4))
	{
		assert (v_input.n == 4);
	}

	QUATERNION (const MATRIX_3X3<T>& A) // matches A with a quaternion
	{
		T trace = 1 + A (1, 1) + A (2, 2) + A (3, 3); // trace=4*cos^2(theta/2)

		if (trace > 1)
		{
			s = T (.5) * sqrt (trace);
			v.x = A (3, 2) - A (2, 3);
			v.y = A (1, 3) - A (3, 1);
			v.z = A (2, 1) - A (1, 2);
			v *= T (.25) / s;
		}
		else
		{
			int i = (A (1, 1) > A (2, 2)) ? 1 : 2;
			i = (A (i, i) > A (3, 3)) ? i : 3; // set i to be the index of the dominating diagonal term

			switch (i)
			{
			case 1:
				v.x = T (.5) * sqrt (1 + A (1, 1) - A (2, 2) - A (3, 3));
				v.y = T (.25) * (A (2, 1) + A (1, 2)) / v.x;
				v.z = T (.25) * (A (1, 3) + A (3, 1)) / v.x;
				s = T (.25) * (A (3, 2) - A (2, 3)) / v.x;
				break;
			case 2:
				v.y = T (.5) * sqrt (1 - A (1, 1) + A (2, 2) - A (3, 3));
				v.x = T (.25) * (A (2, 1) + A (1, 2)) / v.y;
				v.z = T (.25) * (A (3, 2) + A (2, 3)) / v.y;
				s = T (.25) * (A (1, 3) - A (3, 1)) / v.y;
				break;
			case 3:
				v.z = T (.5) * sqrt (1 - A (1, 1) - A (2, 2) + A (3, 3));
				v.x = T (.25) * (A (1, 3) + A (3, 1)) / v.z;
				v.y = T (.25) * (A (3, 2) + A (2, 3)) / v.z;
				s = T (.25) * (A (2, 1) - A (1, 2)) / v.z;
				break;
			}
		}
	}

	QUATERNION (const MATRIX_4X4<T>& A) // matches rotation part of A with a quaternion
	{
		T trace = 1 + A (1, 1) + A (2, 2) + A (3, 3); // trace=4*cos^2(theta/2)

		if (trace > 1)
		{
			s = T (.5) * sqrt (trace);
			v.x = A (3, 2) - A (2, 3);
			v.y = A (1, 3) - A (3, 1);
			v.z = A (2, 1) - A (1, 2);
			v *= T (.25) / s;
		}
		else
		{
			int i = (A (1, 1) > A (2, 2)) ? 1 : 2;
			i = (A (i, i) > A (3, 3)) ? i : 3; // set i to be the index of the dominating diagonal term

			switch (i)
			{
			case 1:
				v.x = T (.5) * sqrt (1 + A (1, 1) - A (2, 2) - A (3, 3));
				v.y = T (.25) * (A (2, 1) + A (1, 2)) / v.x;
				v.z = T (.25) * (A (1, 3) + A (3, 1)) / v.x;
				s = T (.25) * (A (3, 2) - A (2, 3)) / v.x;
				break;
			case 2:
				v.y = T (.5) * sqrt (1 - A (1, 1) + A (2, 2) - A (3, 3));
				v.x = T (.25) * (A (2, 1) + A (1, 2)) / v.y;
				v.z = T (.25) * (A (3, 2) + A (2, 3)) / v.y;
				s = T (.25) * (A (1, 3) - A (3, 1)) / v.y;
				break;
			case 3:
				v.z = T (.5) * sqrt (1 - A (1, 1) - A (2, 2) + A (3, 3));
				v.x = T (.25) * (A (1, 3) + A (3, 1)) / v.z;
				v.y = T (.25) * (A (3, 2) + A (2, 3)) / v.z;
				s = T (.25) * (A (2, 1) - A (1, 2)) / v.z;
				break;
			}
		}
	}

	bool operator== (const QUATERNION<T>& q) const
	{
		return s == q.s && v == q.v;
	}

	bool operator!= (const QUATERNION<T>& q) const
	{
		return s != q.s || v != q.v;
	}

	QUATERNION<T> operator-() const
	{
		return QUATERNION<T> (-s, -v.x, -v.y, -v.z);
	}

	QUATERNION<T>& operator+= (const QUATERNION<T>& q)
	{
		s += q.s, v.x += q.v.x, v.y += q.v.y, v.z += q.v.z;
		return *this;
	}

	QUATERNION<T>& operator-= (const QUATERNION<T>& q)
	{
		s -= q.s, v.x -= q.v.x, v.y -= q.v.y, v.z -= q.v.z;
		return *this;
	}

	QUATERNION<T>& operator*= (const QUATERNION<T>& q)
	{
		return *this = *this * q;
	}

	QUATERNION<T>& operator*= (const T a)
	{
		s *= a;
		v *= a;
		return *this;
	}

	QUATERNION<T>& operator/= (const T a)
	{
		assert (a != 0);
		T r = 1 / a;
		s *= r;
		v *= r;
		return *this;
	}

	QUATERNION<T> operator+ (const QUATERNION<T>& q) const
	{
		return QUATERNION<T> (s + q.s, v.x + q.v.x, v.y + q.v.y, v.z + q.v.z);
	}

	QUATERNION<T> operator- (const QUATERNION<T>& q) const
	{
		return QUATERNION<T> (s - q.s, v.x - q.v.x, v.y - q.v.y, v.z - q.v.z);
	}

	QUATERNION<T> operator* (const QUATERNION<T>& q) const // 16 mult and 13 add/sub
	{
		VECTOR_3D<T> r = s * q.v + q.s * v + VECTOR_3D<T>::Cross_Product (v, q.v);
		return QUATERNION<T> (s * q.s - VECTOR_3D<T>::Dot_Product (v, q.v), r.x, r.y, r.z);
	}

	QUATERNION<T> operator* (const T a) const
	{
		return QUATERNION<T> (s * a, v.x * a, v.y * a, v.z * a);
	}

	QUATERNION<T> operator/ (const T a) const
	{
		assert (a != 0);
		T r = 1 / a;
		return QUATERNION<T> (s * r, v.x * r, v.y * r, v.z * r);
	}

	T Magnitude() const
	{
		return sqrt (sqr (s) + sqr (v.x) + sqr (v.y) + sqr (v.z));
	}

	T Magnitude_Squared() const
	{
		return sqr (s) + sqr (v.x) + sqr (v.y) + sqr (v.z);
	}

	T Max_Abs_Element() const
	{
		return maxabs (s, v.x, v.y, v.z);
	}

	void Normalize()
	{
		T magnitude = Magnitude();
		assert (magnitude != 0);
		T r = 1 / magnitude;
		s *= r;
		v.x *= r;
		v.y *= r;
		v.z *= r;
	}

	T Robust_Normalize (T tolerance = (T) 1e-8, const QUATERNION<T>& fallback = QUATERNION<T> (1, 0, 0, 0))
	{
		T magnitude = Magnitude();

		if (magnitude > tolerance)
		{
			T r = 1 / magnitude;
			s *= r;
			v.x *= r;
			v.y *= r;
			v.z *= r;
		}
		else
		{
			*this = fallback;
		}

		return magnitude;
	}

	QUATERNION<T> Normalized() const
	{
		QUATERNION<T> q (*this);
		q.Normalize();
		return q;
	}

	QUATERNION<T> Robust_Normalized (T tolerance = (T) 1e-8, const QUATERNION<T>& fallback = QUATERNION<T> (1, 0, 0, 0))
	{
		T magnitude = Magnitude();

		if (magnitude > tolerance) return *this / magnitude;
		else return fallback;
	}

	void Invert()
	{
		*this = Inverse();
	}

	QUATERNION<T> Inverse() const
	{
		return QUATERNION<T> (s, -v.x, -v.y, -v.z);
	}

	static QUATERNION<T> Switch_Hemisphere (const QUATERNION<T>& current_hemisphere_quaternion, const QUATERNION<T>& check_quaternion)
	{
		if (Dot_Product (current_hemisphere_quaternion, check_quaternion) < 0) return -check_quaternion;
		else return check_quaternion;
	}

	static T Dot_Product (const QUATERNION<T>& q1, const QUATERNION<T>& q2)
	{
		return q1.s * q2.s + q1.v.x * q2.v.x + q1.v.y * q2.v.y + q1.v.z * q2.v.z;
	}

	VECTOR_3D<T> Rotation_Vector() const
	{
		if (v.Magnitude() == 0.0) return VECTOR_3D<T>(); // return identity

		return 2 * (T) acos (s) * v.Normalized();
	}

	static QUATERNION<T> From_Rotation_Vector (const VECTOR_3D<T>& v)
	{
		if (v.Magnitude() == 0.0) return QUATERNION<T>(); // return identity
		else return QUATERNION<T> (v.Magnitude(), v);
	}

	VECTOR_3D<T> Rotate (const VECTOR_3D<T>& v) const // 32 mult and 26 add/sub
	{
		QUATERNION<T> q = *this * QUATERNION<T> (v) * Inverse();
		return q.v;
	}

	VECTOR_3D<T> Inverse_Rotate (const VECTOR_3D<T>& v) const
	{
		QUATERNION<T> q = Inverse() * QUATERNION<T> (v) * (*this);
		return q.v;
	}

	void Euler_Angles (T& euler_angle_x, T& euler_angle_y, T& euler_angle_z) const
	{
		T r11 = 1 - 2 * sqr (v.y) - 2 * sqr (v.z), r12 = 2 * (v.x * v.y - s * v.z), r21 = 2 * (v.x * v.y + s * v.z), r22 = 1 - 2 * sqr (v.x) - 2 * sqr (v.z), r31 = 2 * (v.x * v.z - v.y * s), r32 = 2 * (v.y * v.z + v.x * s), r33 = 1 - 2 * sqr (v.x) - 2 * sqr (v.y);
		T cos_beta = sqrt (sqr (r11) + sqr (r21));

		if (cos_beta < 1e-14)
		{
			euler_angle_z = 0;

			if (r31 > 0)
			{
				euler_angle_x = -atan2 (r12, r22);
				euler_angle_y = - (T).5 * (T) pi;
			}
			else
			{
				euler_angle_x = atan2 (r12, r22);
				euler_angle_y = (T).5 * (T) pi;
			}
		}
		else
		{
			T secant_beta = 1 / cos_beta;
			euler_angle_x = atan2 (r32 * secant_beta, r33 * secant_beta); // between -pi and pi
			euler_angle_y = atan2 (-r31, cos_beta); // between -pi/2 and pi/2
			euler_angle_z = atan2 (r21 * secant_beta, r11 * secant_beta);
		}
	} // between -pi and pi

	void Get_Rotated_Frame (VECTOR_3D<T>& x_axis, VECTOR_3D<T>& y_axis, VECTOR_3D<T>& z_axis) const // assumes quaternion is normalized
	{
		T vx2 = sqr (v.x), vy2 = sqr (v.y), vz2 = sqr (v.z), vxvy = v.x * v.y, vxvz = v.x * v.z, vyvz = v.y * v.z, svx = s * v.x, svy = s * v.y, svz = s * v.z;
		x_axis = VECTOR_3D<T> (1 - 2 * (vy2 + vz2), 2 * (vxvy + svz), 2 * (vxvz - svy)); // Q*(1,0,0)
		y_axis = VECTOR_3D<T> (2 * (vxvy - svz), 1 - 2 * (vx2 + vz2), 2 * (vyvz + svx)); // Q*(0,1,0)
		z_axis = VECTOR_3D<T> (2 * (vxvz + svy), 2 * (vyvz - svx), 1 - 2 * (vx2 + vy2));
	} // Q*(0,0,1)

	void Get_Angle_Axis (T& angle, VECTOR_3D<T>& axis) const
	{
		if (s == 1)
		{
			angle = 0;
			axis = VECTOR_3D<T>();
		}

		angle = acos (s);
		axis = v / sin (angle);
		angle *= 2;
	}

	T Angle() const
	{
		return 2 * acos (s);
	}

	VECTOR_ND<T> Four_Vector() const
	{
		VECTOR_ND<T> vector (4);
		vector.x[0] = s;
		vector.x[1] = v.x;
		vector.x[2] = v.y;
		vector.x[3] = v.z;
		return vector;
	}

	VECTOR_3D<T> Get_Rotation_Vector() const
	{
		if (s == 1) return VECTOR_3D<T>();

		T angle_over_two = acos (s), angle = 2 * angle_over_two;
		return v * (angle / sin (angle_over_two));
	}

	MATRIX_3X3<T> Matrix_3X3() const // assumes quaternion is normalized; 18 mult and 12 add/sub
	{
		T vx2 = sqr (v.x), vy2 = sqr (v.y), vz2 = sqr (v.z), vxvy = v.x * v.y, vxvz = v.x * v.z, vyvz = v.y * v.z, svx = s * v.x, svy = s * v.y, svz = s * v.z;
		return MATRIX_3X3<T> (1 - 2 * (vy2 + vz2), 2 * (vxvy + svz), 2 * (vxvz - svy),
				      2 * (vxvy - svz), 1 - 2 * (vx2 + vz2), 2 * (vyvz + svx),
				      2 * (vxvz + svy), 2 * (vyvz - svx), 1 - 2 * (vx2 + vy2));
	}

	MATRIX_4X4<T> Matrix_4X4() const // assumes quaternion is normalized
	{
		T vx2 = sqr (v.x), vy2 = sqr (v.y), vz2 = sqr (v.z), vxvy = v.x * v.y, vxvz = v.x * v.z, vyvz = v.y * v.z, svx = s * v.x, svy = s * v.y, svz = s * v.z;
		return MATRIX_4X4<T> (1 - 2 * (vy2 + vz2), 2 * (vxvy + svz), 2 * (vxvz - svy), 0,
				      2 * (vxvy - svz), 1 - 2 * (vx2 + vz2), 2 * (vyvz + svx), 0,
				      2 * (vxvz + svy), 2 * (vyvz - svx), 1 - 2 * (vx2 + vy2), 0,
				      0, 0, 0, 1);
	}

	static QUATERNION<T> Linear_Interpolation (const QUATERNION<T>& q1, const QUATERNION<T>& q2, const T t)
	{
		QUATERNION<T> q = (1 - t) * q1 + t * q2;
		q.Normalize();
		return q;
	}

	static QUATERNION<T> Spherical_Linear_Interpolation (const QUATERNION<T>& q1, const QUATERNION<T>& q2, const T t)
	{
		T sign = 1, cos_theta = Dot_Product (q1, q2);

		if (cos_theta < 0)
		{
			cos_theta *= -1;
			sign = -1;
		}

		cos_theta = min (cos_theta, (T) 1);
		T theta = acos (cos_theta);

		if (theta < 1e-6) return (sign * (1 - t)) * q1 + t * q2; // Linear but not normalized because the worst normalization error will be on the order of 1e-12
		else
		{
			T sin_theta = sqrt (1 - sqr (cos_theta)), sin_t_theta = sin (t * theta), sin_one_minus_t_theta = sin_theta * sqrt (1 - sqr (sin_t_theta)) - sin_t_theta * cos_theta;
			return (1 / sin_theta) * ( (sign * sin_one_minus_t_theta) * q1 + sin_t_theta * q2);
		}
	}

	static QUATERNION<T> Rotation_Quaternion (const VECTOR_3D<T>& initial_vector, const VECTOR_3D<T>& final_vector)
	{
		VECTOR_3D<T> initial_unit = initial_vector / initial_vector.Magnitude(), final_unit = final_vector / final_vector.Magnitude();
		T cos_theta = clamp (VECTOR_3D<T>::Dot_Product (initial_unit, final_unit), (T) - 1.0, (T) 1.0);
		VECTOR_3D<T> v = VECTOR_3D<T>::Cross_Product (initial_unit, final_unit);
		T v_magnitude = v.Magnitude();

		if (v_magnitude == 0) return QUATERNION<T>(); //initial and final vectors are collinear

		T s_squared = (T).5 * (1 + cos_theta); // uses the half angle formula
		T v_magnitude_desired = sqrt (1 - s_squared);
		v *= (v_magnitude_desired / v_magnitude);
		return QUATERNION<T> (sqrt (s_squared), v.x, v.y, v.z);
	}

	void Print() const
	{
		std::cout << "QUATERNION<T>: " << s << " " << v.x << " " << v.y << " " << v.z << std::endl;
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		Read_Binary<RW> (input_stream, s);
		v.template Read<RW> (input_stream);
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		Write_Binary<RW> (output_stream, s);
		v.template Write<RW> (output_stream);
	}

//#####################################################################
};
template<class T>
inline QUATERNION<T> operator* (const T a, const QUATERNION<T>& q)
{
	return QUATERNION<T> (q.s * a, q.v.x * a, q.v.y * a, q.v.z * a);
}

template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const QUATERNION<T>& q)
{
	output_stream << q.s << " " << q.v;
	return output_stream;
}

template<class T>
inline std::istream& operator>> (std::istream& input_stream, QUATERNION<T>& q)
{
	input_stream >> q.s >> q.v;
	q.Normalize();
	return input_stream;
}

}
#endif




