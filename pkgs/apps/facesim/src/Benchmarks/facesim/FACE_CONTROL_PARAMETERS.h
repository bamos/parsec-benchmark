//#####################################################################
// Copyright 2004-2005, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class FACE_CONTROL_PARAMETERS
//#####################################################################
#ifndef __FACE_CONTROL_PARAMETERS__
#define __FACE_CONTROL_PARAMETERS__
#include "FACE_CONTROL_SET.h"
#include "../../Public_Library/Matrices_And_Vectors/MATRIX_NXN.h"
namespace PhysBAM
{

template <class T>
class FACE_CONTROL_PARAMETERS
{
public:
	LIST_ARRAY<FACE_CONTROL_SET<T>*> list;

	FACE_CONTROL_PARAMETERS()
	{}

	int Size() const
	{
		int n = 0;

		for (int s = 1; s <= list.m; s++) n += list (s)->Size();

		return n;
	}

	int Active_Size() const
	{
		int n = 0;

		for (int s = 1; s <= list.m; s++) for (int c = 1; c <= list (s)->Size(); c++) if (list (s)->Control_Active (c)) n++;

		return n;
	}

	int Active_Kinematic_Size() const
	{
		int n = 0;

		for (int s = 1; s <= list.m; s++) for (int c = 1; c <= list (s)->Size(); c++) if (list (s)->Control_Active (c) && list (s)->Positions_Determined_Kinematically (c)) n++;

		return n;
	}

	int Active_Nonkinematic_Size() const
	{
		int n = 0;

		for (int s = 1; s <= list.m; s++) for (int c = 1; c <= list (s)->Size(); c++) if (list (s)->Control_Active (c) && !list (s)->Positions_Determined_Kinematically (c)) n++;

		return n;
	}

	void Seek (const int control_index, int& set_subindex, int& control_subindex) const
	{
		assert (1 <= control_index && control_index <= Size());
		control_subindex = control_index;

		for (set_subindex = 1; control_subindex > list (set_subindex)->Size(); set_subindex++) control_subindex -= list (set_subindex)->Size();
	}

	T& operator() (const int control_index)
	{
		int s, c;
		Seek (control_index, s, c);
		return (*list (s)) (c);
	}

	T operator() (const int control_index) const
	{
		int s, c;
		Seek (control_index, s, c);
		return (*list (s)) (c);
	}

	bool& Active (const int control_index)
	{
		int s, c;
		Seek (control_index, s, c);
		return list (s)->Control_Active (c);
	}

	bool Active (const int control_index) const
	{
		int s, c;
		Seek (control_index, s, c);
		return list (s)->Control_Active (c);
	}

	bool Kinematic (const int control_index) const
	{
		int s, c;
		Seek (control_index, s, c);
		return list (s)->Positions_Determined_Kinematically (c);
	}

	bool Active_Kinematic (const int control_index) const
	{
		int s, c;
		Seek (control_index, s, c);
		return list (s)->Control_Active (c) && list (s)->Positions_Determined_Kinematically (c);
	}

	bool Active_Nonkinematic (const int control_index) const
	{
		int s, c;
		Seek (control_index, s, c);
		return list (s)->Control_Active (c) && !list (s)->Positions_Determined_Kinematically (c);
	}

	ARRAY<int> Active_Subset() const
	{
		ARRAY<int> result (Active_Size());
		int n = 0;

		for (int i = 1; i <= Size(); i++) if (Active (i)) result (++n) = i;

		return result;
	}

	ARRAY<int> Active_Kinematic_Subset() const
	{
		ARRAY<int> result (Active_Kinematic_Size());
		int n = 0;

		for (int i = 1; i <= Size(); i++) if (Active_Kinematic (i)) result (++n) = i;

		return result;
	}

	ARRAY<int> Active_Nonkinematic_Subset() const
	{
		ARRAY<int> result (Active_Nonkinematic_Size());
		int n = 0;

		for (int i = 1; i <= Size(); i++) if (Active_Nonkinematic (i)) result (++n) = i;

		return result;
	}

	void Get (VECTOR_ND<T>& values) const
	{
		values = VECTOR_ND<T> (Size());

		for (int i = 1; i <= Size(); i++) values (i) = (*this) (i);
	}

	void Get (VECTOR_ND<T>& values, const ARRAY<int>& subset) const
	{
		values = VECTOR_ND<T> (subset.m);

		for (int i = 1; i <= subset.m; i++) values (i) = (*this) (subset (i));
	}

	void Get_Active (ARRAY<bool>& active) const
	{
		active.Resize_Array (Size());

		for (int i = 1; i <= Size(); i++) active (i) = Active (i);
	}

	void Get_Active (ARRAY<bool>& active, const ARRAY<int>& subset) const
	{
		active.Resize_Array (subset.m);

		for (int i = 1; i <= subset.m; i++) active (i) = Active (subset (i));
	}

	void Set (const VECTOR_ND<T>& values)
	{
		assert (values.n == Size());

		for (int i = 1; i <= Size(); i++) (*this) (i) = values (i);
	}

	void Set (const VECTOR_ND<T>& values, const ARRAY<int>& subset)
	{
		assert (values.n == subset.m);

		for (int i = 1; i <= subset.m; i++) (*this) (subset (i)) = values (i);
	}

	void Set (const T value_input, const ARRAY<int>& subset)
	{
		for (int i = 1; i <= subset.m; i++) (*this) (subset (i)) = value_input;
	}

	void Set_Active (const ARRAY<bool>& active)
	{
		assert (active.m == Size());

		for (int i = 1; i <= Size(); i++) Active (i) = active (i);
	}

	void Set_Active (const ARRAY<bool>& active, const ARRAY<int>& subset)
	{
		assert (active.m == subset.m);

		for (int i = 1; i <= subset.m; i++) Active (subset (i)) = active (i);
	}

	void Set_Active (const bool active_input, const ARRAY<int>& subset)
	{
		for (int i = 1; i <= subset.m; i++) Active (subset (i)) = active_input;
	}

	T Penalty() const
	{
		T result = 0;

		for (int s = 1; s <= list.m; s++) result += list (s)->Penalty();

		return result;
	}

	VECTOR_ND<T> Penalty_Gradient() const
	{
		ARRAY<int> subset (Active_Subset());
		VECTOR_ND<T> result (subset.m);

		for (int i = 1; i <= subset.m; i++)
		{
			int s, c;
			Seek (subset (i), s, c);
			result (i) = list (s)->Penalty_Gradient (c);
		}

		return result;
	}

	MATRIX_NXN<T> Penalty_Hessian() const
	{
		ARRAY<int> subset (Active_Subset());
		MATRIX_NXN<T> result (subset.m);

		for (int i1 = 1; i1 <= subset.m; i1++) for (int i2 = 1; i2 <= subset.m; i2++)
			{
				int s1, c1, s2, c2;
				Seek (subset (i1), s1, c1);
				Seek (subset (i2), s2, c2);

				if (s1 == s2) result (i1, i2) = list (s1)->Penalty_Hessian (c1, c2);
			}

		return result;
	}

	void Save_Controls()
	{
		for (int s = 1; s <= list.m; s++) list (s)->Save_Controls();
	}

	void Print_Diagnostics (std::ostream& output = std::cout) const
	{
		for (int s = 1; s <= list.m; s++) list (s)->Print_Diagnostics (output);
	}

	void Kinematically_Update_Positions (ARRAY<VECTOR_3D<T> >&X) const
	{
		for (int s = 1; s <= list.m; s++) list (s)->Kinematically_Update_Positions (X);
	}

	void Kinematically_Update_Jacobian (ARRAY<VECTOR_3D<T> >&dX) const
	{
		for (int s = 1; s <= list.m; s++) list (s)->Kinematically_Update_Jacobian (dX);
	}

	void Force_Derivative (ARRAY<VECTOR_3D<T> >& dFdl, const int control_index) const
	{
		int s, c;
		Seek (control_index, s, c);
		list (s)->Force_Derivative (dFdl, c);
	}

	void Position_Derivative (ARRAY<VECTOR_3D<T> >& dXdl, const int control_index) const
	{
		int s, c;
		Seek (control_index, s, c);
		list (s)->Position_Derivative (dXdl, c);
	}

	void Project_Parameters_To_Allowable_Range (const bool active_controls_only = false)
	{
		for (int s = 1; s <= list.m; s++) list (s)->Project_Parameters_To_Allowable_Range (active_controls_only);
	}

	void Interpolate (const T interpolation_fraction)
	{
		for (int s = 1; s <= list.m; s++) list (s)->Interpolate (interpolation_fraction);
	}

	template <class RW>
	void Read (std::istream& input_stream)
	{
		VECTOR_ND<T> values;
		ARRAY<bool> active;
		values.template Read<RW> (input_stream);
		active.template Read<RW> (input_stream);
		assert (values.n == Size() && active.m == Size());
		Set (values);
		Set_Active (active);
	}

	template <class RW>
	void Write (std::ostream& output_stream) const
	{
		VECTOR_ND<T> values;
		ARRAY<bool> active;
		Get (values);
		Get_Active (active);
		values.template Write<RW> (output_stream);
		active.template Write<RW> (output_stream);
	}

//#####################################################################
};
}
#endif
