//#####################################################################
// Copyright 2004-2005, Ron Fedkiw, Joseph Teran, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class COLLISION_PENALTY_FORCES
//#####################################################################
#ifndef __COLLISION_PENALTY_FORCES__
#define __COLLISION_PENALTY_FORCES__

#include "../Forces_And_Torques/SOLIDS_FORCES.h"
#include "../Grids/TETRAHEDRON_MESH.h"
#include "TETRAHEDRON_COLLISION_BODY.h"
#include "COLLISION_BODY_LIST_3D.h"
#include "../Utilities/LOG.h"
namespace PhysBAM
{

template<class T>
class COLLISION_PENALTY_FORCES: public SOLIDS_FORCES<T, VECTOR_3D<T> >
{
public:
	using SOLIDS_FORCES<T, VECTOR_3D<T> >::particles;

	COLLISION_BODY_LIST_3D<T>* collision_body_list;
	ARRAY<bool> skip_collision_body;
	ARRAY<int> check_collision;
	ARRAY<VECTOR_3D<T> > collision_force;
	ARRAY<SYMMETRIC_MATRIX_3X3<T> > collision_force_derivative;
	T stiffness;
	T separation_parameter;
	T self_collision_reciprocity_factor;
	int collision_body_list_id;
	LIST_ARRAY<LIST_ARRAY<int> > partition_check_collision;
	LIST_ARRAY<int> particle_to_check_collision_index;

	COLLISION_PENALTY_FORCES (SOLIDS_PARTICLE<T, VECTOR_3D<T> >& particles_input)
		: SOLIDS_FORCES<T, VECTOR_3D<T> > (particles_input), collision_body_list_id (0)
	{
		Set_Stiffness();
		Set_Separation_Parameter();
		Set_Self_Collision_Reciprocity_Factor();
	}

	~COLLISION_PENALTY_FORCES()
	{}

	void Set_Stiffness (const T stiffness_input = (T) 1e4)
	{
		stiffness = stiffness_input;
	}

	void Set_Separation_Parameter (const T separation_parameter_input = (T) 1e-4)
	{
		separation_parameter = separation_parameter_input;
	}

	void Set_Collision_Body_List (COLLISION_BODY_LIST_3D<T>& collision_body_list_input)
	{
		collision_body_list = &collision_body_list_input;
		skip_collision_body.Resize_Array (collision_body_list->collision_bodies.m);
		ARRAY<bool>::copy (false, skip_collision_body);
	}

	void Set_Boundary_Only_Collisions (TETRAHEDRON_MESH& tetrahedron_mesh)
	{
		LIST_ARRAY<bool>* old_node_on_boundary = tetrahedron_mesh.node_on_boundary;
		tetrahedron_mesh.node_on_boundary = 0;
		tetrahedron_mesh.Initialize_Node_On_Boundary();

		for (int p = 1; p <= particles.array_size; p++) if ( (*tetrahedron_mesh.node_on_boundary) (p)) check_collision.Append_Element (p);

		collision_force.Resize_Array (check_collision.m);
		collision_force_derivative.Resize_Array (check_collision.m);
		delete tetrahedron_mesh.node_on_boundary;
		tetrahedron_mesh.node_on_boundary = old_node_on_boundary;
		ARRAY<bool> particle_checked (particles.number);
		particle_to_check_collision_index.Resize_Array (particles.number);

		for (int p = 1; p <= check_collision.m; p++)
		{
			particle_checked (check_collision (p)) = true;
			particle_to_check_collision_index (check_collision (p)) = p;
		}

		if (particles.particle_ranges)
		{
			partition_check_collision.Resize_Array (particles.particle_ranges->m);

			for (int r = 1; r <= particles.particle_ranges->m; r++)
			{
				VECTOR_2D<int>& range = (*particles.particle_ranges) (r);

				for (int p = range.x; p <= range.y; p++)
				{
					if (particle_checked (p)) partition_check_collision (r).Append_Element (p);
				}
			}
		}
	}

	void Set_Collision_Body_List_ID (const int id)
	{
		collision_body_list_id = id;
	}

	void Set_Self_Collision_Reciprocity_Factor (const T self_collision_reciprocity_factor_input = (T) 2)
	{
		self_collision_reciprocity_factor = self_collision_reciprocity_factor_input;
	}

	void Check_Collision (const int particle)
	{
		check_collision.Append_Unique_Element (particle);
	}

	void Omit_Collision (const int particle)
	{
		int index;

		if (check_collision.Find (particle, index)) check_collision.Remove_Index (index);
	}

	void Resize_Collision_Arrays_From_Check_Collision()
	{
		collision_force.Resize_Array (check_collision.m);
		collision_force_derivative.Resize_Array (check_collision.m);
	}

	void Enforce_Definiteness (const bool enforce_definiteness_input)
	{}

	void Update_Position_Based_State()
	{
		LOG::Time ("UPBS (CPF)");

		if (collision_body_list_id && collision_body_list->collision_bodies (collision_body_list_id)->body_type == COLLISION_BODY_3D<T>::TETRAHEDRON_TYPE)
		{
			TETRAHEDRON_COLLISION_BODY<T>* collision_body = (TETRAHEDRON_COLLISION_BODY<T>*) collision_body_list->collision_bodies (collision_body_list_id);
			collision_body->tetrahedralized_volume.tetrahedron_hierarchy->Update_Boxes (collision_body->collision_thickness);
			collision_body->tetrahedralized_volume.triangulated_surface->Update_Vertex_Normals();
		}

		LOG::Stop_Time();
	}

	void Update_Forces_And_Derivatives()
	{
		for (int p = 1; p <= check_collision.m; p++)
		{
			int index = check_collision (p);
			collision_force (p) = VECTOR_3D<T>();
			collision_force_derivative (p) = SYMMETRIC_MATRIX_3X3<T>();

			for (int r = 1; r <= collision_body_list->collision_bodies.m; r++) if (!skip_collision_body (r))
				{
					int collision_body_particle_index = 0;

					if (collision_body_list_id == r) collision_body_particle_index = index;

					T phi_value;
					int aggregate = -1;
					VECTOR_3D<T> normal = collision_body_list->collision_bodies (r)->Implicit_Surface_Extended_Normal (particles.X (index), phi_value, aggregate, collision_body_particle_index);

					if (phi_value <= 0)
					{
						collision_force (p) += stiffness * (-phi_value + separation_parameter) * normal;

						if (collision_body_list_id == r) collision_force_derivative (p) -= self_collision_reciprocity_factor * stiffness * SYMMETRIC_MATRIX_3X3<T>::Outer_Product (normal);
						else collision_force_derivative (p) -= stiffness * SYMMETRIC_MATRIX_3X3<T>::Outer_Product (normal);
					}
					else if (phi_value < collision_body_list->collision_bodies (r)->collision_thickness)
					{
						collision_force (p) += stiffness * separation_parameter * (T) exp (-phi_value / separation_parameter) * normal;

						if (collision_body_list_id == r)
							collision_force_derivative (p) -= self_collision_reciprocity_factor * stiffness * (T) exp (-phi_value / separation_parameter) * SYMMETRIC_MATRIX_3X3<T>::Outer_Product (normal);
						else collision_force_derivative (p) -= stiffness * (T) exp (-phi_value / separation_parameter) * SYMMETRIC_MATRIX_3X3<T>::Outer_Product (normal);
					}
				}
		}
	}

	void Add_Velocity_Independent_Forces (ARRAY<VECTOR_3D<T> >& F) const
	{
		LOG::Time ("AVIF (CPF)");

		for (int p = 1; p <= check_collision.m; p++) F (check_collision (p)) += collision_force (p);

		LOG::Stop_Time();
	}

	void Add_Force_Differential (const ARRAY<VECTOR_3D<T> >& dX, ARRAY<VECTOR_3D<T> >& dF) const
	{
		//LOG::Time("AFD (CPF)");
		for (int p = 1; p <= check_collision.m; p++) dF (check_collision (p)) += collision_force_derivative (p) * dX (check_collision (p));

		//LOG::Stop_Time();
	}

	void Add_Force_Differential (const ARRAY<VECTOR_3D<T> >& dX, ARRAY<VECTOR_3D<T> >& dF, const int partition_id) const
	{
		for (int i = 1; i <= partition_check_collision (partition_id).m; i++)
		{
			int particle = partition_check_collision (partition_id) (i), index = particle_to_check_collision_index (particle);
			dF (particle) += collision_force_derivative (index) * dX (particle);
		}
	}

//#####################################################################
};
}
#endif
