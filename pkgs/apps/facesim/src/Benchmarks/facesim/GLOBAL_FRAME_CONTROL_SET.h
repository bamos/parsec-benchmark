//#####################################################################
// Copyright 2004, Igor Neverov, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class GLOBAL_FRAME_CONTROL_SET
//#####################################################################
#ifndef __GLOBAL_FRAME_CONTROL_SET__
#define __GLOBAL_FRAME_CONTROL_SET__

#include "FACE_CONTROL_SET.h"
#include "QUASI_RIGID_TRANSFORM_3D.h"

namespace PhysBAM
{

template <class T>
class GLOBAL_FRAME_CONTROL_SET: public FACE_CONTROL_SET<T>
{
public:
	QUASI_RIGID_TRANSFORM_3D<T> global_transform, global_transform_save;
	ARRAY<bool> coefficient_active;
	ARRAY<VECTOR_3D<T> >& X;
	ARRAY<VECTOR_3D<T> > X_save;
	LIST_ARRAY<LIST_ARRAY<int> >& attached_nodes;
	T rigidity_penalty_coefficient;

	GLOBAL_FRAME_CONTROL_SET (ARRAY<VECTOR_3D<T> >& X_input, LIST_ARRAY<LIST_ARRAY<int> >& attached_nodes_input)
		: coefficient_active (12), X (X_input), attached_nodes (attached_nodes_input), rigidity_penalty_coefficient ( (T) 1000)
	{
		Save_Controls();
		ARRAY<bool>::copy (true, coefficient_active);
		X_save = X;
	}

	int Size() const
	{
		return 12;
	}

	T operator() (const int control_id) const
	{
		return global_transform (control_id);
	}

	T& operator() (const int control_id)
	{
		return global_transform (control_id);
	}

	bool Control_Active (const int control_id) const
	{
		return coefficient_active (control_id);
	}

	bool& Control_Active (const int control_id)
	{
		return coefficient_active (control_id);
	}

	bool Positions_Determined_Kinematically (const int control_id) const
	{
		return true;
	}

	void Position_Derivative (ARRAY<VECTOR_3D<T> >& dXdl, const int control_id) const
	{
		assert (1 <= control_id && control_id <= 12);
		MATRIX_3X3<T> affine_differential, affine_differential_transformed;
		VECTOR_3D<T> translation_differential, translation_differential_transformed;

		if (control_id <= 9) affine_differential.x[control_id - 1] = (T) 1;
		else translation_differential[control_id - 9] = (T) 1;

		affine_differential_transformed = affine_differential * global_transform.affine_transform.Inverse();
		translation_differential_transformed = translation_differential - affine_differential_transformed * global_transform.translation;

		for (int i = 1; i <= dXdl.m; i++) dXdl (i) = affine_differential_transformed * X (i) + translation_differential_transformed;

		for (int i = 1; i <= attached_nodes.m; i++) for (int j = 1; j <= attached_nodes (i).m; j++) dXdl (attached_nodes (i) (j)) = affine_differential * X_save (attached_nodes (i) (j)) + translation_differential;
	}

	void Set_Attachment_Positions (ARRAY<VECTOR_3D<T> >&X) const
	{
		for (int i = 1; i <= attached_nodes.m; i++) for (int j = 1; j <= attached_nodes (i).m; j++) X (attached_nodes (i) (j)) = global_transform.affine_transform * X_save (attached_nodes (i) (j)) + global_transform.translation;
	}

	void Save_Controls()
	{
		global_transform_save = global_transform;
	}

	void Kinematically_Update_Positions (ARRAY<VECTOR_3D<T> >&X) const
	{
		QUASI_RIGID_TRANSFORM_3D<T> global_transform_incremental = QUASI_RIGID_TRANSFORM_3D<T>::Incremental_Transform (global_transform, global_transform_save);

		for (int i = 1; i <= X.m; i++) X (i) = global_transform_incremental.affine_transform * X (i) + global_transform_incremental.translation;
	}

	void Kinematically_Update_Jacobian (ARRAY<VECTOR_3D<T> >&dX) const
	{
		QUASI_RIGID_TRANSFORM_3D<T> global_transform_incremental = QUASI_RIGID_TRANSFORM_3D<T>::Incremental_Transform (global_transform, global_transform_save);

		for (int i = 1; i <= X.m; i++) dX (i) = global_transform_incremental.affine_transform * dX (i);
	}

	T Penalty() const
	{
		return rigidity_penalty_coefficient * global_transform.Rigidity_Penalty();
	}

	T Penalty_Gradient (const int control_id) const
	{
		return rigidity_penalty_coefficient * global_transform.Rigidity_Penalty_Gradient (control_id);
	}

	T Penalty_Hessian (const int control_id1, const int control_id2) const
	{
		return rigidity_penalty_coefficient * global_transform.Ridigity_Penalty_Hessian_Definite_Part (control_id1, control_id2);
	}

	void Print_Diagnostics (std::ostream& output = std::cout) const
	{
		global_transform.Print_Diagnostics (output);
	}

//#####################################################################
};
}
#endif
