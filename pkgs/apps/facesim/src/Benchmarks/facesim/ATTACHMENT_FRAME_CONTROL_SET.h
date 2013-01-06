//#####################################################################
// Copyright 2004-2005, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class ATTACHMENT_FRAME_CONTROL_SET
//#####################################################################
#ifndef __ATTACHMENT_FRAME_CONTROL_SET__
#define __ATTACHMENT_FRAME_CONTROL_SET__

#include "FACE_CONTROL_SET.h"
#include "QUASI_RIGID_TRANSFORM_3D.h"

namespace PhysBAM
{

template <class T>
class ATTACHMENT_FRAME_CONTROL_SET: public FACE_CONTROL_SET<T>
{
public:
	QUASI_RIGID_TRANSFORM_3D<T> cranium_transform, jaw_transform, cranium_transform_save, jaw_transform_save;
	ARRAY<bool> coefficient_active;
	ARRAY<VECTOR_3D<T> >& X;
	ARRAY<VECTOR_3D<T> > X_save;
	DIAGONALIZED_FINITE_VOLUME_3D<T>* muscle_force;
	LIST_ARRAY<LIST_ARRAY<int> >& attached_nodes;
	int jaw_attachment_index;
	T rigidity_penalty_coefficient, jaw_constraint_penalty_coefficient;
	VECTOR_3D<T> jaw_midpoint, jaw_normal, jaw_axis, jaw_front;
	T jaw_axis_length, jaw_sliding_length, max_opening_angle;
	FRAME<T> cranium_frame_save, jaw_frame_save;

	ATTACHMENT_FRAME_CONTROL_SET (ARRAY<VECTOR_3D<T> >& X_input, LIST_ARRAY<LIST_ARRAY<int> >& attached_nodes_input, const int jaw_attachment_index_input)
		: coefficient_active (24), X (X_input), attached_nodes (attached_nodes_input), jaw_attachment_index (jaw_attachment_index_input), muscle_force (0), rigidity_penalty_coefficient ( (T) 1000),
		  jaw_constraint_penalty_coefficient ( (T) 1000), max_opening_angle ( (T).19)
	{
		Save_Controls();
		ARRAY<bool>::copy (true, coefficient_active);
		X_save = X;
	}

	int Size() const
	{
		return 24;
	}

	T operator() (const int control_id) const
	{
		return (control_id <= 12) ? cranium_transform (control_id) : jaw_transform (control_id - 12);
	}

	T& operator() (const int control_id)
	{
		return (control_id <= 12) ? cranium_transform (control_id) : jaw_transform (control_id - 12);
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
		assert (1 <= control_id && control_id <= 24);
		return control_id <= 12;
	}

	void Force_Derivative (ARRAY<VECTOR_3D<T> >& dFdl, const int control_id) const
	{
		assert (13 <= control_id && control_id <= 24);
		assert (muscle_force);
		assert (Control_Active (control_id));
		assert (jaw_attachment_index);
		MATRIX_3X3<T> affine_differential;
		VECTOR_3D<T> translation_differential;

		if (control_id <= 21) affine_differential.x[control_id - 13] = (T) 1;
		else translation_differential[control_id - 21] = (T) 1;

		ARRAY<VECTOR_3D<T> > dXdl (X.m);

		for (int i = 1; i <= attached_nodes (jaw_attachment_index).m; i++)
			dXdl (attached_nodes (jaw_attachment_index) (i)) = cranium_transform.affine_transform * (affine_differential * X (attached_nodes (jaw_attachment_index) (i)) + translation_differential);

		ARRAY<VECTOR_3D<T> >::copy (VECTOR_3D<T>(), dFdl);
		muscle_force->Add_Force_Differential (dXdl, dFdl);
	}

	void Position_Derivative (ARRAY<VECTOR_3D<T> >& dXdl, const int control_id) const
	{
		assert (1 <= control_id && control_id <= 12);
		MATRIX_3X3<T> affine_differential, affine_differential_transformed;
		VECTOR_3D<T> translation_differential, translation_differential_transformed;

		if (control_id <= 9) affine_differential.x[control_id - 1] = (T) 1;
		else translation_differential[control_id - 9] = (T) 1;

		affine_differential_transformed = affine_differential * cranium_transform.affine_transform.Inverse();
		translation_differential_transformed = translation_differential - affine_differential_transformed * cranium_transform.translation;

		for (int i = 1; i <= dXdl.m; i++) dXdl (i) = affine_differential_transformed * X (i) + translation_differential_transformed;

		for (int i = 1; i <= attached_nodes.m; i++) for (int j = 1; j <= attached_nodes (i).m; j++)
				if (i == jaw_attachment_index) affine_differential* (jaw_transform.affine_transform * X_save (attached_nodes (i) (j)) + jaw_transform.translation) + translation_differential;
				else dXdl (attached_nodes (i) (j)) = affine_differential * X_save (attached_nodes (i) (j)) + translation_differential;
	}

	void Set_Attachment_Positions (ARRAY<VECTOR_3D<T> >&X) const
	{
		QUASI_RIGID_TRANSFORM_3D<T> jaw_transform_composite = QUASI_RIGID_TRANSFORM_3D<T>::Composite_Transform (cranium_transform, jaw_transform);

		for (int i = 1; i <= attached_nodes.m; i++) for (int j = 1; j <= attached_nodes (i).m; j++)
				if (i == jaw_attachment_index) X (attached_nodes (i) (j)) = jaw_transform_composite.affine_transform * X_save (attached_nodes (i) (j)) + jaw_transform_composite.translation;
				else X (attached_nodes (i) (j)) = cranium_transform.affine_transform * X_save (attached_nodes (i) (j)) + cranium_transform.translation;
	}

	void Save_Controls()
	{
		cranium_transform_save = cranium_transform;
		jaw_transform_save = jaw_transform;
	}

	void Kinematically_Update_Positions (ARRAY<VECTOR_3D<T> >&X) const
	{
		QUASI_RIGID_TRANSFORM_3D<T> cranium_transform_incremental = QUASI_RIGID_TRANSFORM_3D<T>::Incremental_Transform (cranium_transform, cranium_transform_save);

		for (int i = 1; i <= X.m; i++) X (i) = cranium_transform_incremental.affine_transform * X (i) + cranium_transform_incremental.translation;
	}

	void Kinematically_Update_Jacobian (ARRAY<VECTOR_3D<T> >&dX) const
	{
		QUASI_RIGID_TRANSFORM_3D<T> cranium_transform_incremental = QUASI_RIGID_TRANSFORM_3D<T>::Incremental_Transform (cranium_transform, cranium_transform_save);

		for (int i = 1; i <= X.m; i++) dX (i) = cranium_transform_incremental.affine_transform * dX (i);
	}

	T Penalty() const
	{
		T penalty = 0;
		penalty += rigidity_penalty_coefficient * cranium_transform.Rigidity_Penalty();
		penalty += rigidity_penalty_coefficient * jaw_transform.Rigidity_Penalty();
		penalty += jaw_constraint_penalty_coefficient * Jaw_Constraint_Penalty();
		return penalty;
	}

	T Penalty_Gradient (const int control_id) const
	{
		assert (1 <= control_id && control_id <= 24);
		T penalty_gradient = 0;

		if (control_id <= 12) penalty_gradient += rigidity_penalty_coefficient * cranium_transform.Rigidity_Penalty_Gradient (control_id);

		if (control_id >= 13) penalty_gradient += rigidity_penalty_coefficient * jaw_transform.Rigidity_Penalty_Gradient (control_id - 12);

		if (control_id >= 13) penalty_gradient += jaw_constraint_penalty_coefficient * Jaw_Constraint_Penalty_Gradient (control_id - 12);

		return penalty_gradient;
	}

	T Penalty_Hessian (const int control_id1, const int control_id2) const
	{
		assert (1 <= control_id1 && control_id1 <= 24 && 1 <= control_id2 && control_id2 <= 24);
		T penalty_hessian = 0;

		if (control_id1 <= 12 && control_id2 <= 12) penalty_hessian += rigidity_penalty_coefficient * cranium_transform.Ridigity_Penalty_Hessian_Definite_Part (control_id1, control_id2);

		if (control_id1 >= 13 && control_id2 >= 13) penalty_hessian += rigidity_penalty_coefficient * jaw_transform.Ridigity_Penalty_Hessian_Definite_Part (control_id1 - 12, control_id2 - 12);

		if (control_id1 >= 13 && control_id2 >= 13) penalty_hessian += jaw_constraint_penalty_coefficient * Jaw_Constraint_Penalty_Hessian (control_id1 - 12, control_id2 - 12);

		return penalty_hessian;
	}

	T Jaw_Constraint_Penalty() const
	{
		T penalty = 0;
		penalty += sqr (VECTOR_3D<T>::Dot_Product (jaw_normal, jaw_transform.affine_transform * jaw_axis));
		penalty += sqr (VECTOR_3D<T>::Dot_Product (jaw_normal, jaw_transform.affine_transform * jaw_midpoint + jaw_transform.translation - jaw_midpoint));
		penalty += sqr (VECTOR_3D<T>::Dot_Product (jaw_axis, jaw_transform.affine_transform * jaw_midpoint + jaw_transform.translation - jaw_midpoint));
		T left_sliding_validity_measure = VECTOR_3D<T>::Dot_Product (jaw_front, jaw_transform.affine_transform * (jaw_midpoint - (T).5 * jaw_axis_length * jaw_axis) + jaw_transform.translation - jaw_midpoint);
		T right_sliding_validity_measure = VECTOR_3D<T>::Dot_Product (jaw_front, jaw_transform.affine_transform * (jaw_midpoint + (T).5 * jaw_axis_length * jaw_axis) + jaw_transform.translation - jaw_midpoint);
		T opening_angle_validity_measure = VECTOR_3D<T>::Dot_Product (jaw_normal, jaw_transform.affine_transform * jaw_front);

		if (left_sliding_validity_measure < 0) penalty += sqr (left_sliding_validity_measure);

		if (left_sliding_validity_measure > jaw_sliding_length) penalty += sqr (left_sliding_validity_measure - jaw_sliding_length);

		if (right_sliding_validity_measure < 0) penalty += sqr (right_sliding_validity_measure);

		if (right_sliding_validity_measure > jaw_sliding_length) penalty += sqr (right_sliding_validity_measure - jaw_sliding_length);

		if (opening_angle_validity_measure > 0) penalty += sqr (opening_angle_validity_measure);

		T opening_angle_threshold = -asin (max_opening_angle) * (jaw_transform.affine_transform * jaw_front).Magnitude();

		if (opening_angle_validity_measure < opening_angle_threshold) penalty += sqr (opening_angle_validity_measure - opening_angle_threshold);

		return penalty;
	}

	T Jaw_Constraint_Penalty_Gradient (const int control_id) const
	{
		T penalty_gradient = 0;
		MATRIX_3X3<T> affine_transform_differential;

		if (control_id <= 9) affine_transform_differential.x[control_id - 1] = (T) 1;

		VECTOR_3D<T> translation_differential;

		if (control_id >= 10) translation_differential[control_id - 9] = (T) 1;

		penalty_gradient += (T) 2 * VECTOR_3D<T>::Dot_Product (jaw_normal, jaw_transform.affine_transform * jaw_axis)
				    * VECTOR_3D<T>::Dot_Product (jaw_normal, affine_transform_differential * jaw_axis);
		penalty_gradient += (T) 2 * VECTOR_3D<T>::Dot_Product (jaw_normal, jaw_transform.affine_transform * jaw_midpoint + jaw_transform.translation - jaw_midpoint)
				    * VECTOR_3D<T>::Dot_Product (jaw_normal, affine_transform_differential * jaw_midpoint + translation_differential);
		penalty_gradient += (T) 2 * VECTOR_3D<T>::Dot_Product (jaw_axis, jaw_transform.affine_transform * jaw_midpoint + jaw_transform.translation - jaw_midpoint)
				    * VECTOR_3D<T>::Dot_Product (jaw_axis, affine_transform_differential * jaw_midpoint + translation_differential);
		T left_sliding_validity_measure = VECTOR_3D<T>::Dot_Product (jaw_front, jaw_transform.affine_transform * (jaw_midpoint - (T).5 * jaw_axis_length * jaw_axis) + jaw_transform.translation - jaw_midpoint);
		T left_sliding_differential =     VECTOR_3D<T>::Dot_Product (jaw_front, affine_transform_differential * (jaw_midpoint - (T).5 * jaw_axis_length * jaw_axis) + translation_differential);
		T right_sliding_validity_measure = VECTOR_3D<T>::Dot_Product (jaw_front, jaw_transform.affine_transform * (jaw_midpoint + (T).5 * jaw_axis_length * jaw_axis) + jaw_transform.translation - jaw_midpoint);
		T right_sliding_differential =    VECTOR_3D<T>::Dot_Product (jaw_front, affine_transform_differential * (jaw_midpoint + (T).5 * jaw_axis_length * jaw_axis) + translation_differential);
		T opening_angle_validity_measure = VECTOR_3D<T>::Dot_Product (jaw_normal, jaw_transform.affine_transform * jaw_front);
		T opening_angle_differential =    VECTOR_3D<T>::Dot_Product (jaw_normal, affine_transform_differential * jaw_front);

		if (left_sliding_validity_measure < 0) penalty_gradient += (T) 2 * left_sliding_validity_measure * left_sliding_differential;

		if (left_sliding_validity_measure > jaw_sliding_length) penalty_gradient += (T) 2 * (left_sliding_validity_measure - jaw_sliding_length) * left_sliding_differential;

		if (right_sliding_validity_measure < 0) penalty_gradient += (T) 2 * right_sliding_validity_measure * right_sliding_differential;

		if (right_sliding_validity_measure > jaw_sliding_length) penalty_gradient += (T) 2 * (right_sliding_validity_measure - jaw_sliding_length) * right_sliding_differential;

		if (opening_angle_validity_measure > 0) penalty_gradient += (T) 2 * opening_angle_validity_measure * opening_angle_differential;

		T opening_angle_threshold = -asin (max_opening_angle) * (jaw_transform.affine_transform * jaw_front).Magnitude();

		if (opening_angle_validity_measure < opening_angle_threshold) penalty_gradient += (T) 2 * (opening_angle_validity_measure - opening_angle_threshold) * opening_angle_differential;

		return penalty_gradient;
	}

	T Jaw_Constraint_Penalty_Hessian (const int control_id1, const int control_id2) const
	{
		T penalty_hessian = 0;
		MATRIX_3X3<T> affine_transform1_differential, affine_transform2_differential;
		VECTOR_3D<T> translation1_differential, translation2_differential;

		if (control_id1 <= 9) affine_transform1_differential.x[control_id1 - 1] = (T) 1;

		if (control_id2 <= 9) affine_transform2_differential.x[control_id2 - 1] = (T) 1;

		if (control_id1 >= 10) translation1_differential[control_id1 - 9] = (T) 1;

		if (control_id2 >= 10) translation2_differential[control_id2 - 9] = (T) 1;

		penalty_hessian += (T) 2 * VECTOR_3D<T>::Dot_Product (jaw_normal, affine_transform1_differential * jaw_axis)
				   * VECTOR_3D<T>::Dot_Product (jaw_normal, affine_transform2_differential * jaw_axis);
		penalty_hessian += (T) 2 * VECTOR_3D<T>::Dot_Product (jaw_normal, affine_transform1_differential * jaw_midpoint + translation1_differential)
				   * VECTOR_3D<T>::Dot_Product (jaw_normal, affine_transform2_differential * jaw_midpoint + translation2_differential);
		penalty_hessian += (T) 2 * VECTOR_3D<T>::Dot_Product (jaw_axis, affine_transform1_differential * jaw_midpoint + translation1_differential)
				   * VECTOR_3D<T>::Dot_Product (jaw_axis, affine_transform2_differential * jaw_midpoint + translation2_differential);
		T left_sliding_validity_measure = VECTOR_3D<T>::Dot_Product (jaw_front, jaw_transform.affine_transform * (jaw_midpoint - (T).5 * jaw_axis_length * jaw_axis) + jaw_transform.translation - jaw_midpoint);
		T left_sliding_differential1 =    VECTOR_3D<T>::Dot_Product (jaw_front, affine_transform1_differential * (jaw_midpoint - (T).5 * jaw_axis_length * jaw_axis) + translation1_differential);
		T left_sliding_differential2 =    VECTOR_3D<T>::Dot_Product (jaw_front, affine_transform2_differential * (jaw_midpoint - (T).5 * jaw_axis_length * jaw_axis) + translation2_differential);
		T right_sliding_validity_measure = VECTOR_3D<T>::Dot_Product (jaw_front, jaw_transform.affine_transform * (jaw_midpoint + (T).5 * jaw_axis_length * jaw_axis) + jaw_transform.translation - jaw_midpoint);
		T right_sliding_differential1 =   VECTOR_3D<T>::Dot_Product (jaw_front, affine_transform1_differential * (jaw_midpoint + (T).5 * jaw_axis_length * jaw_axis) + translation1_differential);
		T right_sliding_differential2 =   VECTOR_3D<T>::Dot_Product (jaw_front, affine_transform2_differential * (jaw_midpoint + (T).5 * jaw_axis_length * jaw_axis) + translation2_differential);
		T opening_angle_validity_measure = VECTOR_3D<T>::Dot_Product (jaw_normal, jaw_transform.affine_transform * jaw_front);
		T opening_angle_differential1 =   VECTOR_3D<T>::Dot_Product (jaw_normal, affine_transform1_differential * jaw_front);
		T opening_angle_differential2 =   VECTOR_3D<T>::Dot_Product (jaw_normal, affine_transform2_differential * jaw_front);

		if (left_sliding_validity_measure < 0 || left_sliding_validity_measure > jaw_sliding_length) penalty_hessian += (T) 2 * left_sliding_differential1 * left_sliding_differential2;

		if (right_sliding_validity_measure < 0 || right_sliding_validity_measure > jaw_sliding_length) penalty_hessian += (T) 2 * right_sliding_differential1 * right_sliding_differential2;

		T opening_angle_threshold = -asin (max_opening_angle) * (jaw_transform.affine_transform * jaw_front).Magnitude();

		if (opening_angle_validity_measure > 0 || opening_angle_validity_measure < opening_angle_threshold) penalty_hessian += (T) 2 * opening_angle_differential1 * opening_angle_differential2;

		return penalty_hessian;
	}

	template <class RW>
	void Initialize_Jaw_Joint_From_File (const std::string& filename)
	{
		FILE_UTILITIES::Read_From_File<RW> (filename, jaw_midpoint, jaw_normal, jaw_axis, jaw_front, jaw_axis_length, jaw_sliding_length);
	}

	void Set_Original_Attachment_Configuration (const FRAME<T>& cranium_frame_input, const FRAME<T>& jaw_frame_input)
	{
		cranium_frame_save = cranium_frame_input;
		jaw_frame_save = jaw_frame_input;
	}

	FRAME<T> Cranium_Frame()
	{
		return cranium_transform.Frame() * cranium_frame_save;
	}

	FRAME<T> Jaw_Frame()
	{
		return QUASI_RIGID_TRANSFORM_3D<T>::Composite_Transform (cranium_transform, jaw_transform).Frame() * jaw_frame_save;
	}

	void Project_Parameters_To_Allowable_Range (const bool active_controls_only)
	{
		if (!active_controls_only || coefficient_active (1)) cranium_transform.Make_Rigid();

		if (!active_controls_only || coefficient_active (13))
		{
			jaw_transform.Make_Rigid();
			T left_sliding_parameter = VECTOR_3D<T>::Dot_Product (jaw_front, jaw_transform.affine_transform * (jaw_midpoint - (T).5 * jaw_axis_length * jaw_axis) + jaw_transform.translation - jaw_midpoint) / jaw_sliding_length;
			T right_sliding_parameter = VECTOR_3D<T>::Dot_Product (jaw_front, jaw_transform.affine_transform * (jaw_midpoint + (T).5 * jaw_axis_length * jaw_axis) + jaw_transform.translation - jaw_midpoint) / jaw_sliding_length;
			T opening_angle = -asin (VECTOR_3D<T>::Dot_Product (jaw_normal, (jaw_transform.affine_transform * jaw_front).Normalized()));
			left_sliding_parameter = clamp<T> (left_sliding_parameter, 0, (T) 1);
			right_sliding_parameter = clamp<T> (right_sliding_parameter, 0, (T) 1);
			opening_angle = clamp<T> (opening_angle, 0, max_opening_angle);
			VECTOR_3D<T> left_rotation_point = jaw_midpoint - (T).5 * jaw_axis_length * jaw_axis + jaw_sliding_length * left_sliding_parameter * jaw_front;
			VECTOR_3D<T> right_rotation_point = jaw_midpoint + (T).5 * jaw_axis_length * jaw_axis + jaw_sliding_length * right_sliding_parameter * jaw_front;
			T in_plane_rotation_angle = asin (VECTOR_3D<T>::Triple_Product (jaw_axis, (left_rotation_point - right_rotation_point).Normalized(), jaw_normal));
			jaw_transform.affine_transform = MATRIX_3X3<T>::Rotation_Matrix (jaw_normal, -in_plane_rotation_angle) * MATRIX_3X3<T>::Rotation_Matrix (jaw_axis, -opening_angle);
			jaw_transform.translation = (T).5 * (left_rotation_point + right_rotation_point) - jaw_transform.affine_transform * jaw_midpoint;
		}
	}

	T Opening_Angle() const
	{
		return -asin (VECTOR_3D<T>::Dot_Product (jaw_normal, (jaw_transform.affine_transform * jaw_front).Normalized()));
	}

	void Increment_Opening_Angle (const T angle)
	{
		VECTOR_3D<T> rotated_axis = jaw_transform.affine_transform * jaw_axis;
		VECTOR_3D<T> old_midpoint_rotation = jaw_transform.affine_transform * jaw_midpoint;
		MATRIX_3X3<T> rotation = MATRIX_3X3<T>::Rotation_Matrix (rotated_axis, angle);
		jaw_transform.affine_transform = rotation * jaw_transform.affine_transform;
		VECTOR_3D<T> new_midpoint_rotation = jaw_transform.affine_transform * jaw_midpoint;
		jaw_transform.translation += (old_midpoint_rotation - new_midpoint_rotation);
	}

	void Interpolate (const T interpolation_fraction)
	{
		cranium_transform = QUASI_RIGID_TRANSFORM_3D<T>::Interpolate (cranium_transform_save, cranium_transform, interpolation_fraction);
		jaw_transform = QUASI_RIGID_TRANSFORM_3D<T>::Interpolate (jaw_transform_save, jaw_transform, interpolation_fraction);
	}

	void Print_Diagnostics (std::ostream& output = std::cout) const
	{
		output << "Cranium frame control diagnostics" << std::endl;
		cranium_transform.Print_Diagnostics (output);
		output << "Jaw frame control diagnostics" << std::endl;
		jaw_transform.Print_Diagnostics (output);
		output << "Jaw plane deviation angle : " << asin (VECTOR_3D<T>::Dot_Product (jaw_normal, (jaw_transform.affine_transform * jaw_axis).Normalized())) << std::endl;
		output << "Jaw midpoint off-plane deviation : " << fabs (VECTOR_3D<T>::Dot_Product (jaw_normal, jaw_transform.affine_transform * jaw_midpoint + jaw_transform.translation - jaw_midpoint)) << std::endl;
		output << "Jaw midpoint off-axis deviation : " << fabs (VECTOR_3D<T>::Dot_Product (jaw_axis, jaw_transform.affine_transform * jaw_midpoint + jaw_transform.translation - jaw_midpoint)) << std::endl;
		output << "Left condyle sliding parameter : " << VECTOR_3D<T>::Dot_Product (jaw_front, jaw_transform.affine_transform * (jaw_midpoint - (T).5 * jaw_axis_length * jaw_axis) + jaw_transform.translation - jaw_midpoint) / jaw_sliding_length << std::endl;
		output << "Right condyle sliding parameter : " << VECTOR_3D<T>::Dot_Product (jaw_front, jaw_transform.affine_transform * (jaw_midpoint + (T).5 * jaw_axis_length * jaw_axis) + jaw_transform.translation - jaw_midpoint) / jaw_sliding_length << std::endl;
		output << "Opening angle : " << -asin (VECTOR_3D<T>::Dot_Product (jaw_normal, (jaw_transform.affine_transform * jaw_front).Normalized())) << std::endl;
		output << "Jaw constraint penalty at current configuration : " << Jaw_Constraint_Penalty() << std::endl;
	}

//#####################################################################
};
}
#endif
