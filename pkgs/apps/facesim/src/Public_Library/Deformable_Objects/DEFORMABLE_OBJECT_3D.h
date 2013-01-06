//#####################################################################
// Copyright 2004, Zhaosheng Bao, Ronald Fedkiw, Geoffrey Irving, Sergey Koltakov, Igor Neverov, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DEFORMABLE_OBJECT_3D
//#####################################################################
#ifndef __DEFORMABLE_OBJECT_3D__
#define __DEFORMABLE_OBJECT_3D__

#include "DEFORMABLE_OBJECT.h"
#include "DEFORMABLE_OBJECT_COLLISIONS_3D.h"
#include "../Fracture/EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE.h"
namespace PhysBAM
{

template<class T> class BODY_FORCES_3D;
template<class T> class DIAGONALIZED_FINITE_VOLUME_3D;
template<class T> class STRAIN_MEASURE_3D;
template<class T> class CONSTITUTIVE_MODEL_3D;
template<class T> class DIAGONALIZED_CONSTITUTIVE_MODEL_3D;
template<class T> class COLLISION_PENALTY_FORCES;

template<class T>
class DEFORMABLE_OBJECT_3D: public DEFORMABLE_OBJECT<T, VECTOR_3D<T> >
{
public:
	using DEFORMABLE_OBJECT<T, VECTOR_3D<T> >::particles;
	using DEFORMABLE_OBJECT<T, VECTOR_3D<T> >::solids_forces;
	using DEFORMABLE_OBJECT<T, VECTOR_3D<T> >::external_forces_and_velocities;
	using DEFORMABLE_OBJECT<T, VECTOR_3D<T> >::id_number;
	using DEFORMABLE_OBJECT<T, VECTOR_3D<T> >::print_diagnostics;
	using DEFORMABLE_OBJECT<T, VECTOR_3D<T> >::print_residuals;
	using DEFORMABLE_OBJECT<T, VECTOR_3D<T> >::Enforce_Definiteness;
	using DEFORMABLE_OBJECT<T, VECTOR_3D<T> >::Save_Velocity;
	using DEFORMABLE_OBJECT<T, VECTOR_3D<T> >::Restore_Velocity;
	using DEFORMABLE_OBJECT<T, VECTOR_3D<T> >::Update_Position_Based_State;
	using DEFORMABLE_OBJECT<T, VECTOR_3D<T> >::cfl_number;
	using DEFORMABLE_OBJECT<T, VECTOR_3D<T> >::Add_Velocity_Independent_Forces;
	using DEFORMABLE_OBJECT<T, VECTOR_3D<T> >::dX_full;
	using DEFORMABLE_OBJECT<T, VECTOR_3D<T> >::R_full;


	SEGMENTED_CURVE_3D<T>* segmented_curve;
	TRIANGULATED_SURFACE<T>* triangulated_surface;
	TETRAHEDRALIZED_VOLUME<T>* tetrahedralized_volume;
	EMBEDDED_TETRAHEDRALIZED_VOLUME<T>* embedded_tetrahedralized_volume;
	EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>* embedded_tetrahedralized_volume_boundary_surface;
	DEFORMABLE_OBJECT_COLLISIONS_3D<T> collisions;
	ARRAY<BODY_FORCES_3D<T>*> body_forces;
	ARRAY<DIAGONALIZED_FINITE_VOLUME_3D<T>*> diagonalized_finite_volume_3d;
	ARRAY<STRAIN_MEASURE_3D<T>*> strain_measure_3d;
	ARRAY<COLLISION_PENALTY_FORCES<T>*> collision_penalty_forces;
private:
	ARRAY<CONSTITUTIVE_MODEL_3D<T>*> constitutive_model_3d;
	ARRAY<DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T>*> diagonalized_constitutive_model_3d;
public:
	bool use_nonembedded_self_collision;

	DEFORMABLE_OBJECT_3D()
		: segmented_curve (0), triangulated_surface (0), tetrahedralized_volume (0), embedded_tetrahedralized_volume (0),
		  embedded_tetrahedralized_volume_boundary_surface (0), use_nonembedded_self_collision (false)
	{}

	virtual ~DEFORMABLE_OBJECT_3D()
	{
		if (segmented_curve)
		{
			delete &segmented_curve->segment_mesh;
			delete segmented_curve;
		}

		if (triangulated_surface)
		{
			delete &triangulated_surface->triangle_mesh;
			delete triangulated_surface;
		}

		if (tetrahedralized_volume)
		{
			delete &tetrahedralized_volume->tetrahedron_mesh;
			delete tetrahedralized_volume;
		}

		if (embedded_tetrahedralized_volume)
		{
			delete &embedded_tetrahedralized_volume->tetrahedralized_volume.tetrahedron_mesh;
			delete &embedded_tetrahedralized_volume->tetrahedralized_volume;
			delete embedded_tetrahedralized_volume;
		}

		delete embedded_tetrahedralized_volume_boundary_surface;
		Delete_Forces();
	}

	void Allocate_Segmented_Curve()
	{
		assert (!segmented_curve);
		segmented_curve = new SEGMENTED_CURVE_3D<T> (* (new SEGMENT_MESH), particles);
		particles.Update_Velocity();
		particles.Store_Mass();
	}

	void Allocate_Triangulated_Surface()
	{
		assert (!triangulated_surface);
		triangulated_surface = new TRIANGULATED_SURFACE<T> (* (new TRIANGLE_MESH), particles);
		particles.Update_Velocity();
		particles.Store_Mass();
	}

	void Allocate_Tetrahedralized_Volume()
	{
		assert (!tetrahedralized_volume);
		tetrahedralized_volume = new TETRAHEDRALIZED_VOLUME<T> (* (new TETRAHEDRON_MESH), particles);
		particles.Update_Velocity();
		particles.Store_Mass();
	}

	void Allocate_Embedded_Triangulated_Surface (const int hashtable_multiplier = 15, const T interpolation_fraction_threshold = 1e-4)
	{}

	void Allocate_Embedded_Tetrahedralized_Volume (const int hashtable_multiplier = 15, const T interpolation_fraction_threshold = 1e-4)
	{
		assert (!embedded_tetrahedralized_volume);
		assert (!embedded_tetrahedralized_volume_boundary_surface);
		embedded_tetrahedralized_volume = new EMBEDDED_TETRAHEDRALIZED_VOLUME<T> (* (new TETRAHEDRALIZED_VOLUME<T> (* (new TETRAHEDRON_MESH), particles)));
		particles.Update_Velocity();
		particles.Store_Mass();
		embedded_tetrahedralized_volume->Initialize_Parents_To_Embedded_Particles_Hash_Table (hashtable_multiplier);
		embedded_tetrahedralized_volume->Set_Interpolation_Fraction_Threshold (interpolation_fraction_threshold);
		embedded_tetrahedralized_volume_boundary_surface = new EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T> (*embedded_tetrahedralized_volume);
	}

	virtual void Initialize_Collision_Geometry()
	{
		if (segmented_curve) collisions.Initialize_Segmented_Curve (*segmented_curve);
		else if (triangulated_surface) collisions.Initialize_Triangulated_Surface (*triangulated_surface);
		else if (tetrahedralized_volume)
		{
			if (!tetrahedralized_volume->triangulated_surface) tetrahedralized_volume->Initialize_Triangulated_Surface();

			collisions.Initialize_Triangulated_Surface (*tetrahedralized_volume->triangulated_surface);
		}
		else if (embedded_tetrahedralized_volume) collisions.Initialize_Triangulated_Surface (embedded_tetrahedralized_volume_boundary_surface->boundary_surface);
	}

	void Initialize_Object_Collisions (const bool collide_with_interior = false, const bool enforce_tangential_collision_velocity = false)
	{
		if (segmented_curve) collisions.Initialize_Object_Collisions (segmented_curve->particles, enforce_tangential_collision_velocity);
		else if (triangulated_surface) collisions.Initialize_Object_Collisions (triangulated_surface->particles, enforce_tangential_collision_velocity);
		else if (tetrahedralized_volume)
		{
			collisions.Initialize_Object_Collisions (tetrahedralized_volume->particles, enforce_tangential_collision_velocity);

			if (!collide_with_interior)
			{
				TETRAHEDRON_MESH& tetrahedron_mesh = tetrahedralized_volume->tetrahedron_mesh;
				LIST_ARRAY<bool>* old_node_on_boundary = tetrahedron_mesh.node_on_boundary;
				tetrahedron_mesh.node_on_boundary = 0;
				tetrahedron_mesh.Initialize_Node_On_Boundary();
				ARRAY<bool>::exchange_arrays (collisions.check_collision, tetrahedron_mesh.node_on_boundary->array);
				delete tetrahedron_mesh.node_on_boundary;
				tetrahedron_mesh.node_on_boundary = old_node_on_boundary;
			}
		}
		else if (embedded_tetrahedralized_volume)
		{
			collisions.Initialize_Object_Collisions (embedded_tetrahedralized_volume->tetrahedralized_volume.particles, enforce_tangential_collision_velocity);

			if (collide_with_interior) assert (false);
		}
	}

	void Allocate_Asynchronous_Time_Stepping()
	{}

	// Should be accelerated by using the triangle_hierarchy to eliminate tests on parts of the surface that aren't close to collision bodies.
	int Adjust_Nodes_For_Collision_Body_Collisions (const T dt)
	{
		if (embedded_tetrahedralized_volume) return collisions.Adjust_Nodes_For_Collision_Body_Collisions (*embedded_tetrahedralized_volume, *embedded_tetrahedralized_volume_boundary_surface, dt);
		else return collisions.Adjust_Nodes_For_Collision_Body_Collisions (dt);
	}

	void Restore_Self_Collision_Free_State()
	{
		collisions.Restore_Self_Collision_Free_State();

		if (embedded_tetrahedralized_volume) // triangle_collisions only restores the boundary mesh, need to propagate this back to the tetrahedralized volume
		{
			for (int p = 1; p <= embedded_tetrahedralized_volume->tetrahedralized_volume.particles.number; p++)
			{
				embedded_tetrahedralized_volume->tetrahedralized_volume.particles.X (p) =
					embedded_tetrahedralized_volume_boundary_surface->boundary_particles.X (p + embedded_tetrahedralized_volume->embedded_particles.number);
				embedded_tetrahedralized_volume->tetrahedralized_volume.particles.V (p) =
					embedded_tetrahedralized_volume_boundary_surface->boundary_particles.V (p + embedded_tetrahedralized_volume->embedded_particles.number);
			}

			for (int p = 1; p <= embedded_tetrahedralized_volume->tetrahedralized_volume.particles.number; p++) // THIS IS NEW!
			{
				embedded_tetrahedralized_volume->embedded_particles.X (p) = embedded_tetrahedralized_volume_boundary_surface->boundary_particles.X (p);
				embedded_tetrahedralized_volume->embedded_particles.V (p) = embedded_tetrahedralized_volume_boundary_surface->boundary_particles.V (p);
			}
		}
	}

	void Disable_Finite_Volume_Damping()
	{
		for (int k = 1; k <= diagonalized_finite_volume_3d.m; k++) diagonalized_finite_volume_3d (k)->Use_Velocity_Dependent_Forces (false);
	}

	void Disable_Spring_Elasticity()
	{}

	template<class RW>
	void Read_Static_Variables (const std::string& prefix, const int frame = -1)
	{
		char version;
		bool has_segmented_curve, has_triangulated_surface, has_tetrahedralized_volume, has_embedded_tetrahedralized_volume, has_embedded_triangulated_surface = false;
		std::string f = frame == -1 ? "" : STRING_UTILITIES::string_sprintf (".%d", frame);
		std::istream* mesh_type_input = FILE_UTILITIES::Safe_Open_Input (prefix + "mesh_type" + f);
		Read_Binary<RW> (*mesh_type_input, version, has_segmented_curve, has_triangulated_surface, has_tetrahedralized_volume, has_embedded_tetrahedralized_volume);

		if (version == 2) Read_Binary<RW> (*mesh_type_input, has_embedded_triangulated_surface);
		else if (version != 1)
		{
			std::cerr << "Unrecognized deformable object version number " << (int) version << std::endl;
			exit (1);
		}

		delete mesh_type_input;

		if (has_segmented_curve)
		{
			Allocate_Segmented_Curve();
			FILE_UTILITIES::Read_From_File<RW> (prefix + "segment_mesh" + f, segmented_curve->segment_mesh);
		}

		if (has_triangulated_surface)
		{
			Allocate_Triangulated_Surface();
			FILE_UTILITIES::Read_From_File<RW> (prefix + "triangle_mesh" + f, triangulated_surface->triangle_mesh);
		}

		if (has_tetrahedralized_volume)
		{
			Allocate_Tetrahedralized_Volume();
			FILE_UTILITIES::Read_From_File<RW> (prefix + "tetrahedron_mesh" + f, tetrahedralized_volume->tetrahedron_mesh);
		}

		if (has_embedded_tetrahedralized_volume)
		{
			Allocate_Embedded_Tetrahedralized_Volume();
			FILE_UTILITIES::Read_From_File<RW> (prefix + "embedded_tetrahedralized_volume" + f, *embedded_tetrahedralized_volume);
		}
	}

	template<class RW>
	void Read_Dynamic_Variables (const std::string& prefix, const int frame)
	{
		particles.template Read<RW> (prefix, frame);

		if (embedded_tetrahedralized_volume) embedded_tetrahedralized_volume->embedded_particles.template Read<RW> (prefix + "embedded_surface_", frame);
	}

	template<class RW>
	void Write_Static_Variables (const std::string& prefix, const int frame = -1) const
	{
		char version = 2;
		std::string f = frame == -1 ? "" : STRING_UTILITIES::string_sprintf (".%d", frame);
		FILE_UTILITIES::Write_To_File<RW> (prefix + "mesh_type" + f, version, segmented_curve != 0, triangulated_surface != 0, tetrahedralized_volume != 0, embedded_tetrahedralized_volume != 0, false);

		if (segmented_curve) FILE_UTILITIES::Write_To_File<RW> (prefix + "segment_mesh" + f, segmented_curve->segment_mesh);

		if (triangulated_surface) FILE_UTILITIES::Write_To_File<RW> (prefix + "triangle_mesh" + f, triangulated_surface->triangle_mesh);

		if (tetrahedralized_volume) FILE_UTILITIES::Write_To_File<RW> (prefix + "tetrahedron_mesh" + f, tetrahedralized_volume->tetrahedron_mesh);

		if (embedded_tetrahedralized_volume) // writes positions
		{
			FILE_UTILITIES::Write_To_File<RW> (prefix + "embedded_tetrahedralized_volume" + f, *embedded_tetrahedralized_volume);
			FILE_UTILITIES::Write_To_File<RW> (prefix + "embedded_tetrahedralized_volume_boundary_surface" + f, embedded_tetrahedralized_volume_boundary_surface->boundary_surface);
		}
	}

	template<class RW>
	void Write_Dynamic_Variables (const std::string& prefix, const int frame) const
	{
		particles.template Write<RW> (prefix, frame);

		if (embedded_tetrahedralized_volume) embedded_tetrahedralized_volume->embedded_particles.template Write<RW> (prefix + "embedded_surface_", frame);
	}

//#####################################################################
	void Delete_Forces();
	void Update_Collision_Penalty_Forces_And_Derivatives();
	void Advance_One_Time_Step (const T time, const T dt, const T cg_tolerance, const int cg_iterations, const bool perform_collision_body_collisions, const bool verbose);
	void Advance_One_Time_Step_Semi_Implicit (const T time, const T dt, const bool perform_collision_body_collisions, const bool verbose);
	void Advance_One_Time_Step_Quasistatic (const T time, const T dt, const T cg_tolerance, const int cg_iterations, const T newton_tolerance, const int newton_iterations, const bool use_partially_converged_result,
						const bool verbose);
public:
	int Add_Quasistatic_Neo_Hookean_Elasticity (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const T youngs_modulus = 3e6, const T poissons_ratio = .475, const bool verbose = true);
	int Add_Quasistatic_Diagonalized_Linear_Finite_Volume (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const T youngs_modulus = 3e6, const T poissons_ratio = .475, const bool verbose = true,
			const bool precompute_stiffness_matrix = false);
	int Add_Quasistatic_Diagonalized_Neo_Hookean_Elasticity (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const T youngs_modulus = 3e6, const T poissons_ratio = .475, const T failure_threshold = .25,
			const bool verbose = true, const bool precompute_stiffness_matrix = false);
	int Add_Quasistatic_Diagonalized_Linear_Elasticity (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const T youngs_modulus = 3e6, const T poissons_ratio = .475, const bool verbose = true);
	int Add_Diagonalized_Mooney_Rivlin (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const T mu_10 = (T) 6e4, const T mu_01 = (T) 2e4, const T kappa = (T) 6e4, const T Rayleigh_coefficient = .05,
					    const T failure_threshold = (T).25, const bool limit_time_step_by_strain_rate = true, const T max_strain_per_time_step = .1, const bool use_rest_state_for_strain_rate = true, const bool verbose = true);
	int Add_Quasistatic_Diagonalized_Mooney_Rivlin (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const T mu_10 = (T) 6e4, const T mu_01 = (T) 2e4, const T kappa = (T) 6e4, const T failure_threshold = (T).25,
			const bool verbose = true, const bool precompute_stiffness_matrix = false);
	int Add_Diagonalized_Face (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const LIST_ARRAY<LIST_ARRAY<int> >& muscle_tets, const LIST_ARRAY<LIST_ARRAY<VECTOR_3D<T> > >& muscle_fibers,
				   const LIST_ARRAY<LIST_ARRAY<T> >& muscle_densities, LIST_ARRAY<T>& muscle_activations, const LIST_ARRAY<T>* peak_isometric_stress = 0, const T mu_10 = (T) 6e4, const T mu_01 = (T) 2e4, const T kappa = (T) 6e4,
				   const T Rayleigh_coefficient = (T).05, const T failure_threshold = (T).25, const bool limit_time_step_by_strain_rate = true, const T max_strain_per_time_step = (T).1,
				   const bool use_rest_state_for_strain_rate = true, const bool verbose = true);
	int Add_Quasistatic_Diagonalized_Face (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const LIST_ARRAY<LIST_ARRAY<int> >& muscle_tets, const LIST_ARRAY<LIST_ARRAY<VECTOR_3D<T> > >& muscle_fibers,
					       const LIST_ARRAY<LIST_ARRAY<T> >& muscle_densities, LIST_ARRAY<T>& muscle_activations, int* single_activation_used_for_force_derivative = 0, const LIST_ARRAY<T>* peak_isometric_stress = 0,
					       DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T> **face_constitutive_model = 0, const T mu_10 = (T) 6e4, const T mu_01 = (T) 2e4, const T kappa = (T) 6e4, const T failure_threshold = (T).25, const bool verbose = false);
//#####################################################################
};
}
#endif
