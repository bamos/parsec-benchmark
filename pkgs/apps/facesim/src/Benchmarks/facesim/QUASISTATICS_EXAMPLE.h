//#####################################################################
// Copyright 2004, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class QUASISTATICS_EXAMPLE
//#####################################################################
#ifndef __QUASISTATICS_EXAMPLE__
#define __QUASISTATICS_EXAMPLE__

#include "../../Public_Library/Solids_And_Fluids/SOLIDS_FLUIDS_EXAMPLE_3D.h"
#include "../../Public_Library/Rigid_Bodies/RIGID_BODY_LIST_3D.h"
#include "../../Public_Library/Deformable_Objects/DEFORMABLE_OBJECT_LIST_3D.h"
#include "../../Public_Library/Read_Write/FILE_UTILITIES.h"
#include "../../Public_Library/Utilities/STRING_UTILITIES.h"
#include "../../Public_Library/Collisions_And_Interactions/COLLISION_BODY_LIST_3D.h"
#include "../../Public_Library/Forces_And_Torques/BODY_FORCES_3D.h"
namespace PhysBAM
{

template <class T, class RW>
class QUASISTATICS_EXAMPLE: public SOLIDS_FLUIDS_EXAMPLE_3D<T, RW>
{
public:
	using SOLIDS_FLUIDS_EXAMPLE_3D<T, RW>::solids_parameters;
	using SOLIDS_FLUIDS_EXAMPLE<T>::output_directory;
	int newton_iterations;
	T newton_tolerance;
	bool use_partially_converged_result;
	ARRAY<LIST_ARRAY<int> > colliding_nodes;
	bool allow_tangential_slipping_for_rigid_collisions;
	BODY_FORCES_3D<T>* body_forces;
	VECTOR_3D<T> external_force_rate;
	ARRAY<LIST_ARRAY<int> > inverted_tets;
	bool print_mesh_information;

	QUASISTATICS_EXAMPLE()
		: SOLIDS_FLUIDS_EXAMPLE_3D<T, RW>(), newton_iterations (1), newton_tolerance ( (T) 1e-3), use_partially_converged_result (false),
		  allow_tangential_slipping_for_rigid_collisions (false), body_forces (0), print_mesh_information (false)
	{}

	virtual ~QUASISTATICS_EXAMPLE()
	{}

	void Default() const
	{
		std::cout << "THIS QUASISTATICS_EXAMPLE FUNCTION IS NOT DEFINED!" << std::endl;
	}

//#####################################################################
	virtual void Initialize_Bodies();
	virtual void Add_External_Forces (ARRAY<VECTOR_3D<T> >& F, const T time, const int id_number) {}
	virtual void Set_External_Positions (ARRAY<VECTOR_3D<T> >& X, const T time, const int id_number) {}
	virtual void Zero_Out_Enslaved_Position_Nodes (ARRAY<VECTOR_3D<T> >& X, const T time, const int id_number) {}
	virtual void Update_Time_Varying_Material_Properties (const T time, const int id_number);
	virtual void Update_Collision_Body_Positions (const T time) {}
	virtual void Print_Mesh_Information (const int frame) const;
	virtual void Compute_Inverted_Tets();
	virtual void Write_Output_Files (const int frame) const;
//#####################################################################
};
//#####################################################################
// Function Initialize_Deformable_Objects
//#####################################################################
template<class T, class RW> void QUASISTATICS_EXAMPLE<T, RW>::
Initialize_Bodies()
{
	SOLIDS_FLUIDS_EXAMPLE_3D<T, RW>::Initialize_Bodies();
	colliding_nodes.Resize_Array (solids_parameters.deformable_body_parameters.list.deformable_objects.m);
	inverted_tets.Resize_Array (solids_parameters.deformable_body_parameters.list.deformable_objects.m);

	for (int i = 1; i <= solids_parameters.deformable_body_parameters.list.deformable_objects.m; i++) solids_parameters.deformable_body_parameters.list.deformable_objects (i)->collisions.collision_tolerance = solids_parameters.collision_tolerance;
}
//#####################################################################
// Function Update_Time_Varying_Material_Properties
//#####################################################################
template<class T, class RW> void QUASISTATICS_EXAMPLE<T, RW>::
Update_Time_Varying_Material_Properties (const T time, const int id_number)
{
	if (body_forces)
	{
		T magnitude = external_force_rate.Magnitude();
		T incremented_gravity = max (- (T) 9.8, time * magnitude);

		if (magnitude) body_forces->downward_direction = external_force_rate / magnitude;

		body_forces->gravity = incremented_gravity;
	}
}
//#####################################################################
// Function Print_Mesh_Information
//#####################################################################
template<class T, class RW> void QUASISTATICS_EXAMPLE<T, RW>::
Print_Mesh_Information (const int frame) const
{
	std::ostream* output;
	std::string prefix = output_directory + "/";

	for (int d = 1; d <= solids_parameters.deformable_body_parameters.list.deformable_objects.m; d++)
	{
		output = FILE_UTILITIES::Safe_Open_Output (STRING_UTILITIES::string_sprintf ("%scolliding_nodes_%d.%d", prefix.c_str(), d, frame));
		colliding_nodes (d).template Write<RW> (*output);
		delete output;
		output = FILE_UTILITIES::Safe_Open_Output (STRING_UTILITIES::string_sprintf ("%ssubset_%d.%d", prefix.c_str(), d, frame));
		inverted_tets (d).template Write<RW> (*output);
		delete output;
	}
}
//#####################################################################
// Function Compute_Inverted_Tets
//#####################################################################
template<class T, class RW> void QUASISTATICS_EXAMPLE<T, RW>::
Compute_Inverted_Tets()
{
	for (int d = 1; d <= solids_parameters.deformable_body_parameters.list.deformable_objects.m; d++)
	{
		TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume = *solids_parameters.deformable_body_parameters.list.deformable_objects (d)->tetrahedralized_volume;
		SOLIDS_PARTICLE<T, VECTOR_3D<T> > particles = solids_parameters.deformable_body_parameters.list.deformable_objects (d)->particles;

		for (int t = 1; t <= tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.m; t++)
		{
			int i, j, k, l;
			tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.Get (t, i, j, k, l);
			T signed_volume = TETRAHEDRON<T>::Signed_Volume (particles.X.array (i), particles.X.array (j), particles.X.array (k), particles.X.array (l));

			if (signed_volume < 0) inverted_tets (d).Append_Element (t);
		}
	}
}
//#####################################################################
// Function Compute_Inverted_Tets
//#####################################################################
template<class T, class RW> void QUASISTATICS_EXAMPLE<T, RW>::
Write_Output_Files (const int frame) const
{
	SOLIDS_FLUIDS_EXAMPLE_3D<T, RW>::Write_Output_Files (frame);

	if (print_mesh_information) Print_Mesh_Information (frame);
}
//#####################################################################
}
#endif
