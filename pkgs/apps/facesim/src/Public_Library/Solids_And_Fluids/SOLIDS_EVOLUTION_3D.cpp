//#####################################################################
// Copyright 2004, Ron Fedkiw, Eran Guendelman, Geoffrey Irving, Andrew Selle, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SOLIDS_EVOLUTION_3D
//#####################################################################
#include "SOLIDS_EVOLUTION_3D.h"
#include "SOLIDS_FLUIDS_EXAMPLE.h"
#include "SOLIDS_PARAMETERS_3D.h"
#include "RIGID_BODY_PARAMETERS_3D.h"
#include "DEFORMABLE_BODY_PARAMETERS_3D.h"
#include "../Utilities/LOG.h"
#include "../Rigid_Bodies/RIGID_BODY_EVOLUTION_3D.h"
#include "../Rigid_Bodies/RIGID_BODY_COLLISIONS_3D.h"
using namespace PhysBAM;
//#####################################################################
// Destructor
//#####################################################################
template<class T> SOLIDS_EVOLUTION_3D<T>::
~SOLIDS_EVOLUTION_3D()
{
	delete rigid_body_collisions;
	delete rigid_body_evolution;
}
//#####################################################################
// Function Initialize_Deformable_Objects
//#####################################################################
template<class T> void SOLIDS_EVOLUTION_3D<T>::
Initialize_Deformable_Objects (const T frame_rate, const bool restart, const bool verbose_dt)
{
	DEFORMABLE_BODY_PARAMETERS_3D<T>& deformable_body_parameters = solids_parameters.deformable_body_parameters;

	for (int i = 1; i <= deformable_body_parameters.list.deformable_objects.m; i++)
		if (deformable_body_parameters.list (i).embedded_tetrahedralized_volume)
		{
			deformable_body_parameters.list (i).embedded_tetrahedralized_volume->embedded_particles.Store_Mass();
			deformable_body_parameters.list (i).embedded_tetrahedralized_volume->embedded_particles.Store_Velocity (!quasistatic);
			deformable_body_parameters.list (i).embedded_tetrahedralized_volume->Update_Embedded_Particle_Masses();
			deformable_body_parameters.list (i).embedded_tetrahedralized_volume->Update_Embedded_Particle_Positions();

			if (!quasistatic) deformable_body_parameters.list (i).embedded_tetrahedralized_volume->Update_Embedded_Particle_Velocities();

			deformable_body_parameters.list (i).embedded_tetrahedralized_volume->Initialize_Embedded_Sub_Elements_In_Parent_Element();
			deformable_body_parameters.list (i).embedded_tetrahedralized_volume->Initialize_Embedded_Children();
			deformable_body_parameters.list (i).embedded_tetrahedralized_volume->tetrahedralized_volume.tetrahedron_mesh.Initialize_Incident_Tetrahedrons();
			deformable_body_parameters.list (i).embedded_tetrahedralized_volume->Initialize_Node_In_Tetrahedron_Is_Material();
		}

	deformable_body_parameters.list.Initialize_Object_Collisions (solids_parameters.collide_with_interior, solids_parameters.enforce_tangential_collision_velocity);

	if (restart)
	{
		deformable_body_parameters.list.Update_Position_Based_State(); // some CFL's require this

		if (!quasistatic) deformable_body_parameters.list.CFL (verbose_dt);
	} // needs to be initialized before positions change
}
//#####################################################################
// Function Initialize_Rigid_Bodies
//#####################################################################
template<class T> void SOLIDS_EVOLUTION_3D<T>::
Initialize_Rigid_Bodies (const T frame_rate)
{
	RIGID_BODY_PARAMETERS_3D<T>& rigid_body_parameters = solids_parameters.rigid_body_parameters;

	if (!rigid_body_parameters.simulate) solids_evolution_callbacks->Update_Collision_Body_Positions_And_Velocities (time); // initialize object positions if not simulating
	else
	{

		RIGID_BODY_COLLISIONS_3D<T>::Adjust_Bounding_Boxes (rigid_body_parameters.list.rigid_bodies);

		// compute voxel size for spatial partition
		T voxel_size = 0;

		if (rigid_body_parameters.spatial_partition_based_on_scene_size)
		{
			VECTOR_3D<T> scene_box_size = RIGID_BODY_COLLISIONS_3D<T>::Scene_Bounding_Box (rigid_body_parameters.list.rigid_bodies).Size();
			voxel_size = (T) one_third * (scene_box_size.x + scene_box_size.y + scene_box_size.z) / rigid_body_parameters.spatial_partition_number_of_cells;
		}
		else if (rigid_body_parameters.spatial_partition_based_on_object_size)
		{
			if (rigid_body_parameters.spatial_partition_with_max_size) voxel_size = RIGID_BODY_COLLISIONS_3D<T>::Maximum_Bounding_Box_Size (rigid_body_parameters.list.rigid_bodies);
			else voxel_size = RIGID_BODY_COLLISIONS_3D<T>::Average_Bounding_Box_Size (rigid_body_parameters.list.rigid_bodies);

			voxel_size *= 8;
		} // just to make it bigger

		// collisions
		rigid_body_collisions = new RIGID_BODY_COLLISIONS_3D<T> (rigid_body_parameters.list.rigid_bodies, voxel_size);

		if (rigid_body_parameters.use_collision_matrix) rigid_body_collisions->collision_manager.Use_Collision_Matrix();

		rigid_body_collisions->verbose = verbose;

		if (rigid_body_parameters.use_particle_partition)
		{
			rigid_body_collisions->intersections.Use_Particle_Partition (true, rigid_body_parameters.particle_partition_size, rigid_body_parameters.particle_partition_size,
					rigid_body_parameters.particle_partition_size);

			if (rigid_body_parameters.use_particle_partition_center_phi_test) rigid_body_collisions->intersections.Use_Particle_Partition_Center_Phi_Test();
		}

		if (rigid_body_parameters.use_triangle_hierarchy)
		{
			rigid_body_collisions->intersections.Use_Triangle_Hierarchy();

			if (rigid_body_parameters.use_triangle_hierarchy_center_phi_test) rigid_body_collisions->intersections.Use_Triangle_Hierarchy_Center_Phi_Test();

			if (rigid_body_parameters.use_edge_intersection) rigid_body_collisions->intersections.Use_Edge_Intersection();
		}

		// dynamics
		rigid_body_evolution = new RIGID_BODY_EVOLUTION_3D<T> (rigid_body_parameters.list.rigid_bodies, *rigid_body_collisions);
		rigid_body_evolution->Set_CFL_Number (solids_parameters.cfl);
		rigid_body_evolution->Set_Max_Rotation_Per_Time_Step (rigid_body_parameters.max_rotation_per_time_step);
		rigid_body_evolution->Set_Max_Linear_Movement_Fraction_Per_Time_Step (rigid_body_parameters.max_linear_movement_fraction_per_time_step);
		rigid_body_evolution->Set_Minimum_And_Maximum_Time_Step (0, (T).1 / frame_rate);

		if (rigid_body_parameters.artificial_maximum_speed) rigid_body_evolution->Set_Artificial_Maximum_Speed (rigid_body_parameters.artificial_maximum_speed);

		rigid_body_evolution->collisions.Initialize_Data_Structures(); // Must be done before we check interpenetration statistics

		// precompute normals - faster, but less accurate
		//RIGID_BODY_COLLISIONS_3D<T>::Precompute_Normals(rigid_body_parameters.list.rigid_bodies);

		// check for bad initial data
		if (rigid_body_parameters.print_interpenetration_statistics) rigid_body_evolution->collisions.Print_Interpenetration_Statistics();
		else if (!rigid_body_evolution->collisions.Check_For_Any_Interpenetration()) LOG::cout << "No initial interpenetration" << std::endl;
	}
}
//#####################################################################
// Function Advance_To_Target_Time
//#####################################################################
template<class T> void SOLIDS_EVOLUTION_3D<T>::
Advance_To_Target_Time (const T target_time, const bool verbose_dt)
{
	RIGID_BODY_PARAMETERS_3D<T>& rigid_body_parameters = solids_parameters.rigid_body_parameters;

	if (!rigid_body_parameters.simulate)
	{
		Advance_Deformable_Bodies_To_Target_Time (target_time, verbose_dt);
		return;
	}

	bool done = false;

	for (int substep = 1; !done; substep++)
	{
		LOG::Push_Scope ("rigid body", "Rigid body substep %d", substep);
		solids_evolution_callbacks->Update_Rigid_Body_Parameters (rigid_body_collisions->collision_manager, time);
		rigid_body_evolution->collisions.Initialize_Data_Structures(); // call here to handle possibly changed number of rigid bodies
		T dt_cfl = rigid_body_evolution->CFL (verbose_dt), dt = dt_cfl;
		SOLIDS_FLUIDS_EXAMPLE<T>::Clamp_Time_Step_With_Target_Time (time, target_time, dt, done);
		solids_evolution_callbacks->Update_Kinematic_Rigid_Body_States (dt, time);

		if (verbose) LOG::cout << "rigid substep = " << substep << ", time = " << time << ", dt = " << dt << " (cfl = " << dt_cfl << ")" << std::endl;

		// Rigid body states are saved in order to do interpolation
		for (int i = 1; i <= rigid_body_parameters.list.rigid_bodies.m; i++) rigid_body_parameters.list.rigid_bodies (i)->Save_State (COLLISION_BODY_3D<T>::SOLIDS_EVOLUTION_RIGID_BODY_OLD_STATE, time);

		LOG::Time ("advancing rigid bodies");
		rigid_body_evolution->Advance_One_Time_Step (dt, time);

		for (int i = 1; i <= rigid_body_parameters.list.rigid_bodies.m; i++) rigid_body_parameters.list.rigid_bodies (i)->Save_State (COLLISION_BODY_3D<T>::SOLIDS_EVOLUTION_RIGID_BODY_NEW_STATE, time + dt);

		Advance_Deformable_Bodies_To_Target_Time (time + dt, verbose_dt);

		for (int i = 1; i <= rigid_body_parameters.list.rigid_bodies.m; i++) rigid_body_parameters.list.rigid_bodies (i)->Restore_State (COLLISION_BODY_3D<T>::SOLIDS_EVOLUTION_RIGID_BODY_NEW_STATE);

		solids_evolution_callbacks->Apply_Constraints (dt, time);
		LOG::Pop_Scope();
	}
}
//#####################################################################
// Function Advance_Deformable_Bodies_To_Target_Time
//#####################################################################
template<class T> void SOLIDS_EVOLUTION_3D<T>::
Advance_Deformable_Bodies_To_Target_Time (const T target_time, const bool verbose_dt)
{
	int iteration = 0;
	LOG::Push_Scope ("ADB", "ADB");
	DEFORMABLE_BODY_PARAMETERS_3D<T>& deformable_body_parameters = solids_parameters.deformable_body_parameters;

	if (!deformable_body_parameters.list.deformable_objects.m)
	{
		time = target_time;
		solids_evolution_callbacks->Postprocess_Solids_Substep (time, 1);
		LOG::Pop_Scope(); // from ADB
		return;
	}

	// prepare for force computation
	deformable_body_parameters.list.Update_Time_Varying_Material_Properties (time);
	deformable_body_parameters.list.Update_Position_Based_State();

	Advance_Deformable_Objects_In_Time (target_time, INT_MAX, verbose_dt);
	solids_evolution_callbacks->Postprocess_Solids_Substep (time, 1);
	LOG::Pop_Scope();
}
//#####################################################################
// Function Advance_Deformable_Objects_In_Time
//#####################################################################
template<class T> void SOLIDS_EVOLUTION_3D<T>::
Advance_Deformable_Objects_In_Time (const T final_time, const int total_loops, const bool verbose_dt)
{
	DEFORMABLE_BODY_PARAMETERS_3D<T>& deformable_body_parameters = solids_parameters.deformable_body_parameters;
	T initial_time = time;

	for (int i = 1; i <= deformable_body_parameters.list.deformable_objects.m; i++) if (deformable_body_parameters.list (i).simulate)
		{
			LOG::Push_Scope ("ADO", "ADO");

			time = initial_time;
			bool body_done = false;

			while (!body_done)
			{
				T dt = FLT_MAX;

				if (quasistatic)
				{
					SOLIDS_FLUIDS_EXAMPLE<T>::Clamp_Time_Step_With_Target_Time (time, final_time, dt, body_done);
					LOG::Time ("ADO - Update collision bodies");
					solids_evolution_callbacks->Update_Collision_Body_Positions_And_Velocities (time + dt);
					LOG::Stop_Time();
					deformable_body_parameters.list (i).Advance_One_Time_Step_Quasistatic (time, dt, solids_parameters.cg_tolerance, solids_parameters.cg_iterations, newton_tolerance,
							newton_iterations, use_partially_converged_result, verbose);
				}
				else if (!solids_parameters.semi_implicit)
				{
					LOG::Push_Scope ("CFL", "CFL");
					dt = deformable_body_parameters.list.deformable_objects (i)->CFL (verbose_dt);
					LOG::Pop_Scope();

					if (verbose) LOG::cout << "raw dt = " << dt << std::endl;

					LOG::Time ("ADO - Update collision bodies");
					SOLIDS_FLUIDS_EXAMPLE<T>::Clamp_Time_Step_With_Target_Time (time, final_time, dt, body_done);
					solids_evolution_callbacks->Update_Collision_Body_Positions_And_Velocities (time + dt);
					LOG::Stop_Time();
					deformable_body_parameters.list (i).Advance_One_Time_Step (time, dt, solids_parameters.cg_tolerance, solids_parameters.cg_iterations,
							solids_parameters.perform_collision_body_collisions, verbose);
				}
				else
				{
					dt = deformable_body_parameters.list.deformable_objects (i)->CFL (verbose_dt);

					if (verbose) LOG::cout << "raw dt = " << dt << std::endl;

					SOLIDS_FLUIDS_EXAMPLE<T>::Clamp_Time_Step_With_Target_Time (time, final_time, dt, body_done);
					solids_evolution_callbacks->Update_Collision_Body_Positions_And_Velocities (time + dt);
					deformable_body_parameters.list (i).Advance_One_Time_Step_Semi_Implicit (time, dt, solids_parameters.perform_collision_body_collisions, verbose);
				}

				time += dt;

				if (verbose) LOG::cout << "dt = " << dt << std::endl;
			}

			LOG::Pop_Scope();
		}

	if (!solids_parameters.asynchronous) time = final_time;
}
//#####################################################################


template class SOLIDS_EVOLUTION_3D<float>;
template class SOLIDS_EVOLUTION_3D<double>;
