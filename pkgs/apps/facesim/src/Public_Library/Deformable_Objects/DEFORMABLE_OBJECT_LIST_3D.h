//#####################################################################
// Copyright 2004-2005, Ron Fedkiw, Geoffrey Irving, Sergey Koltakov, Andrew Selle, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DEFORMABLE_OBJECT_LIST_3D
//#####################################################################
#ifndef __DEFORMABLE_OBJECT_LIST_3D__
#define __DEFORMABLE_OBJECT_LIST_3D__

#include "../Utilities/STRING_UTILITIES.h"
#include "../Read_Write/READ_WRITE_FUNCTIONS.h"
#include "DEFORMABLE_OBJECT_3D.h"
namespace PhysBAM
{

template<class T>
class DEFORMABLE_OBJECT_LIST_3D
{
public:
	LIST_ARRAY<DEFORMABLE_OBJECT_3D<T>*> deformable_objects;

	DEFORMABLE_OBJECT_LIST_3D()
	{}

	~DEFORMABLE_OBJECT_LIST_3D()
	{
		Clean_Up_Memory();
	}

	void Clean_Up_Memory()
	{
		for (int i = 1; i <= deformable_objects.m; i++)
		{
			assert (deformable_objects (i));
			delete deformable_objects (i);
		}

		deformable_objects.Resize_Array (0);
	}

	DEFORMABLE_OBJECT_3D<T>& operator() (const int i)
	{
		return *deformable_objects (i);
	}

	const DEFORMABLE_OBJECT_3D<T>& operator() (const int i) const
	{
		return *deformable_objects (i);
	}

	int Add_Deformable_Object()
	{
		deformable_objects.Append_Element (new DEFORMABLE_OBJECT_3D<T>());
		return deformable_objects.m;
	}

	int Add_Deformable_Object (DEFORMABLE_OBJECT_3D<T>* deformable_object)
	{
		deformable_objects.Append_Element (deformable_object);
		return deformable_objects.m;
	}

	int Add_Deformable_Segmented_Curve()
	{
		int index = Add_Deformable_Object();
		deformable_objects (index)->Allocate_Segmented_Curve();
		return index;
	}

	int Add_Deformable_Triangulated_Surface()
	{
		int index = Add_Deformable_Object();
		deformable_objects (index)->Allocate_Triangulated_Surface();
		return index;
	}

	int Add_Deformable_Tetrahedralized_Volume()
	{
		int index = Add_Deformable_Object();
		deformable_objects (index)->Allocate_Tetrahedralized_Volume();
		return index;
	}

	int Add_Deformable_Embedded_Triangulated_Surface (const int hashtable_multiplier = 15, const T interpolation_fraction_threshold = 1e-4)
	{
		int index = Add_Deformable_Object();
		deformable_objects (index)->Allocate_Embedded_Triangulated_Surface (hashtable_multiplier, interpolation_fraction_threshold);
		return index;
	}

	int Add_Deformable_Embedded_Tetrahedralized_Volume (const int hashtable_multiplier = 15, const T interpolation_fraction_threshold = 1e-4)
	{
		int index = Add_Deformable_Object();
		deformable_objects (index)->Allocate_Embedded_Tetrahedralized_Volume (hashtable_multiplier, interpolation_fraction_threshold);
		return index;
	}

	void Set_Collision_Body_List (COLLISION_BODY_LIST_3D<T>& collision_body_list_input)
	{
		for (int i = 1; i <= deformable_objects.m; i++) deformable_objects (i)->collisions.Set_Collision_Body_List (collision_body_list_input);
	}

	void Set_External_Forces_And_Velocities (EXTERNAL_FORCES_AND_VELOCITIES<T, VECTOR_3D<T> >& external_forces_and_velocities)
	{
		for (int i = 1; i <= deformable_objects.m; i++) deformable_objects (i)->Set_External_Forces_And_Velocities (external_forces_and_velocities, i);
	}

	void Initialize_Object_Collisions (const bool collide_with_interior = false, const bool enforce_tangential_collision_velocity = false)
	{
		for (int i = 1; i <= deformable_objects.m; i++) deformable_objects (i)->Initialize_Object_Collisions (collide_with_interior, enforce_tangential_collision_velocity);
	}

	void Save_Self_Collision_Free_State()
	{
		for (int i = 1; i <= deformable_objects.m; i++) deformable_objects (i)->collisions.Save_Self_Collision_Free_State();
	}

	void Restore_Self_Collision_Free_State()
	{
		for (int i = 1; i <= deformable_objects.m; i++)
		{
			deformable_objects (i)->Restore_Self_Collision_Free_State();
			deformable_objects (i)->Update_Position_Based_State();
		}
	}

	void Adjust_Mesh_For_Self_Collision (const T time, const bool verbose = true)
	{
		for (int i = 1; i <= deformable_objects.m; i++) if (deformable_objects (i)->collisions.perform_self_collision)
			{
				if (deformable_objects (i)->embedded_triangulated_surface)
				{
					if (deformable_objects (i)->use_nonembedded_self_collision)
					{
						deformable_objects (i)->triangles_of_material->Update_Particle_Positions();
						deformable_objects (i)->triangles_of_material->Update_Particle_Velocities();
					}
					else
					{
						int interactions = deformable_objects (i)->collisions.Adjust_Mesh_For_Embedded_Self_Collision (*deformable_objects (i)->embedded_triangulated_surface,
								   *deformable_objects (i)->triangles_of_material);
						int total = deformable_objects (i)->embedded_triangulated_surface->embedded_particles.number;

						if (verbose) std::cout << "deformable object " << i << " TOTAL EMBEDDED SELF COLLISIONS = " << interactions << " OUT OF " << total << std::endl;
					}
				}
				else if (deformable_objects (i)->embedded_tetrahedralized_volume)
				{
					int interactions = deformable_objects (i)->collisions.Adjust_Mesh_For_Embedded_Self_Collision (*deformable_objects (i)->embedded_tetrahedralized_volume,
							   *deformable_objects (i)->embedded_tetrahedralized_volume_boundary_surface);
					int total = deformable_objects (i)->embedded_tetrahedralized_volume->embedded_particles.number;

					if (verbose) std::cout << "deformable object " << i << " TOTAL EMBEDDED SELF COLLISIONS = " << interactions << " OUT OF " << total << std::endl;
				}

				deformable_objects (i)->Update_Position_Based_State();
				deformable_objects (i)->external_forces_and_velocities->Update_Time_Varying_Material_Properties (time, i);
			}
	}

	void Advance_One_Time_Step_Quasistatic (const T time, const T dt, const T cg_tolerance, const int cg_iterations, const T newton_tolerance, const int newton_iterations,
						const bool use_partially_converged_result, const bool verbose, const bool interleave_newton_iteration)
	{
		if (!interleave_newton_iteration) for (int i = 1; i <= deformable_objects.m; i++)
			{
				if (verbose) std::cout << "Advancing deformable object " << i << " from " << time << " to " << time + dt << std::endl;

				deformable_objects (i)->Advance_One_Time_Step_Quasistatic (time, dt, cg_tolerance, cg_iterations, newton_tolerance, newton_iterations, use_partially_converged_result, verbose);
			}
		else
		{
			if (verbose) std::cout << "Advancing deformable objects from " << time << " to " << time + dt << std::endl;

			for (int i = 1; i <= deformable_objects.m; i++) deformable_objects (i)->Enforce_Definiteness (true);

			Update_Time_Varying_Material_Properties (time + dt);
			// Iterate to steady state
			double supnorm = 0;
			int iteration;
			ARRAY<bool> object_converged (deformable_objects.m);

			for (iteration = 0; iteration < newton_iterations; iteration++)
			{
				for (int i = 1; i <= deformable_objects.m; i++)
				{
					if (verbose) std::cout << "Advancing object " << i << std::endl;

					DEFORMABLE_OBJECT_3D<T>& deformable_object = *deformable_objects (i);
					SOLIDS_PARTICLE<T, VECTOR_3D<T> >& particles = deformable_object.particles;
					ARRAY<VECTOR_3D<T> > dX (particles.number), R (particles.number);
					deformable_object.Update_Collision_Penalty_Forces_And_Derivatives();

					if (deformable_object.One_Newton_Step_Toward_Steady_State (cg_tolerance, cg_iterations, time + dt, dX, false, 0, iteration == 0) || use_partially_converged_result) particles.X.array += dX;

					deformable_object.Update_Position_Based_State();
					deformable_object.Update_Collision_Penalty_Forces_And_Derivatives();
					deformable_object.Add_Velocity_Independent_Forces (R);
					deformable_object.external_forces_and_velocities->Add_External_Forces (R, time, i);
					deformable_object.external_forces_and_velocities->Zero_Out_Enslaved_Position_Nodes (R, time, i);
					supnorm = (T) 0;

					for (int p = 1; p <= particles.number; p++)
					{
						double s2 = R (p).Magnitude_Squared();
						supnorm = max (supnorm, s2);
					}

					supnorm = sqrt (supnorm);

					if (deformable_object.print_residuals) std::cout << "Newton iteration residual after " << iteration + 1 << " iterations for object " << i << " = " << supnorm << std::endl;

					if (supnorm <= newton_tolerance)
					{
						std::cout << "Newton converged in " << iteration + 1 << " steps for object " << i << std::endl;
						object_converged (i) = true;
					}
				}

				bool converged = true;

				for (int i = 1; i <= deformable_objects.m; i++) if (!object_converged (i)) converged = false;

				if (converged) break;
			}
		}
	}

	void Update_Position_Based_State()
	{
		for (int i = 1; i <= deformable_objects.m; i++) deformable_objects (i)->Update_Position_Based_State();
	}

	void Update_Time_Varying_Material_Properties (const T time)
	{
		for (int i = 1; i <= deformable_objects.m; i++) deformable_objects (i)->external_forces_and_velocities->Update_Time_Varying_Material_Properties (time, i);
	}

	void Set_CFL_Number (const T cfl_number)
	{
		for (int i = 1; i <= deformable_objects.m; i++) deformable_objects (i)->Set_CFL_Number (cfl_number);
	}

	T CFL (const bool verbose = false) // takes the min CFL of all objects
	{
		T dt = FLT_MAX;

		for (int i = 1; i <= deformable_objects.m; i++) dt = min (dt, deformable_objects (i)->CFL (verbose));

		return dt;
	}

	void Print_Diagnostics (const bool print_diagnostics)
	{
		for (int i = 1; i <= deformable_objects.m; i++) deformable_objects (i)->Print_Diagnostics (print_diagnostics);
	}

	void Print_Residuals (const bool print_residuals)
	{
		for (int i = 1; i <= deformable_objects.m; i++) deformable_objects (i)->Print_Residuals (print_residuals);
	}

	template<class RW>
	void Read_Static_Variables (const std::string& prefix, const int frame = -1)
	{
		Clean_Up_Memory();
		std::istream* input;
		std::string f = frame == -1 ? "" : STRING_UTILITIES::string_sprintf (".%d", frame);
		input = FILE_UTILITIES::Safe_Open_Input (prefix + "deformable_object_key" + f);
		int m;
		Read_Binary<RW> (*input, m);
		delete input;
		deformable_objects.Resize_Array (m);

		for (int i = 1; i <= m; i++)
		{
			deformable_objects (i) = new DEFORMABLE_OBJECT_3D<T>();
			deformable_objects (i)->template Read_Static_Variables<RW> (STRING_UTILITIES::string_sprintf ("%sdeformable_object_%d_", prefix.c_str(), i), frame);
		}
	}

	template<class RW>
	void Write_Static_Variables (const std::string& prefix, const int frame = -1) const
	{
		std::ostream* output;
		std::string f = frame == -1 ? "" : STRING_UTILITIES::string_sprintf (".%d", frame);
		output = FILE_UTILITIES::Safe_Open_Output (prefix + "deformable_object_key" + f);
		Write_Binary<RW> (*output, deformable_objects.m);
		delete output;

		for (int i = 1; i <= deformable_objects.m; i++)
			deformable_objects (i)->template Write_Static_Variables<RW> (STRING_UTILITIES::string_sprintf ("%sdeformable_object_%d_", prefix.c_str(), i), frame);
	}

	template<class RW>
	void Read_Dynamic_Variables (const std::string& prefix, const int frame) // assumes static variables are already read in
	{
		for (int i = 1; i <= deformable_objects.m; i++) deformable_objects (i)->template Read_Dynamic_Variables<RW> (STRING_UTILITIES::string_sprintf ("%sdeformable_object_%d_", prefix.c_str(), i), frame);
	}

	template<class RW>
	void Write_Dynamic_Variables (const std::string& prefix, const int frame) const
	{
		for (int i = 1; i <= deformable_objects.m; i++) deformable_objects (i)->template Write_Dynamic_Variables<RW> (STRING_UTILITIES::string_sprintf ("%sdeformable_object_%d_", prefix.c_str(), i), frame);
	}

//#####################################################################
};
}
#endif
