//#####################################################################
// Copyright 2002-2004, Doug Enright, Ronald Fedkiw, Frederic Gibou, Sergey Koltakov, Neil Molino, Igor Neverov, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "LEVELSET_3D.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
extern bool PHYSBAM_THREADED_RUN;
//#####################################################################
// Function Use_Collision_Aware_Interpolation
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Use_Collision_Aware_Interpolation (COLLISION_BODY_LIST_3D<T>& body_list_input, ARRAYS_3D<bool>* valid_value_mask_input, ARRAYS_3D<bool>* occupied_cell_input)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Compute_Normals
//#####################################################################
// note that sqrt(phix^2+phiy^2+phiz^2)=1 if it's a distance function
template<class T> void LEVELSET_3D<T>::
Compute_Normals (const T time)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Compute_Curvature
//#####################################################################
// kappa = - DIV(normal), negative for negative phi inside, positive for positive phi inside, sqrt(phix^2+phiy^2+phiy^2)=1 for distance functions
template<class T> void LEVELSET_3D<T>::
Compute_Curvature (const T time)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Compute_Cell_Minimum_And_Maximum
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Compute_Cell_Minimum_And_Maximum (const bool recompute_if_exists)
{
	if (!recompute_if_exists && cell_maximum && cell_minimum) return;

	if (!cell_maximum) cell_maximum = new ARRAYS_3D<T> (phi.m_start, phi.m_end - 1, phi.n_start, phi.n_end - 1, phi.mn_start, phi.mn_end - 1);

	if (!cell_minimum) cell_minimum = new ARRAYS_3D<T> (phi.m_start, phi.m_end - 1, phi.n_start, phi.n_end - 1, phi.mn_start, phi.mn_end - 1);

	for (int i = phi.m_start; i <= phi.m_end - 1; i++) for (int j = phi.n_start; j <= phi.n_end - 1; j++) for (int ij = phi.mn_start; ij <= phi.mn_end - 1; ij++)
			{
				int index = phi.Standard_Index (i, j, ij);
				T phi1 = phi.array[index], phi2 = phi.array[phi.I_Plus_One (index)], phi3 = phi.array[phi.J_Plus_One (index)], phi4 = phi.array[phi.IJ_Plus_One (index)],
				  phi5 = phi.array[phi.I_Plus_One_J_Plus_One (index)], phi6 = phi.array[phi.I_Plus_One_IJ_Plus_One (index)], phi7 = phi.array[phi.J_Plus_One_IJ_Plus_One (index)],
				  phi8 = phi.array[phi.I_Plus_One_J_Plus_One_IJ_Plus_One (index)];
				(*cell_maximum) (i, j, ij) = max (phi1, phi2, phi3, phi4, phi5, phi6, phi7, phi8);
				(*cell_minimum) (i, j, ij) = min (phi1, phi2, phi3, phi4, phi5, phi6, phi7, phi8);
			}
}
//#####################################################################
// Function Euler_Step
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Euler_Step (const ARRAYS_3D<T>& u, const ARRAYS_3D<T>& v, const ARRAYS_3D<T>& w, const T dt, const T time)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Euler_Step
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Euler_Step (const ARRAYS_3D<VECTOR_3D<T> >& velocity, const T dt, const T time)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function CFL
//#####################################################################
template<class T> T LEVELSET_3D<T>::
CFL (const ARRAYS_3D<T>& u, const ARRAYS_3D<T>& v, const ARRAYS_3D<T>& w) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function CFL
//#####################################################################
template<class T> T LEVELSET_3D<T>::
CFL (const ARRAYS_3D<VECTOR_3D<T> >& velocity) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Reinitialize
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Reinitialize (const int time_steps, const T time)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Euler_Step_Of_Reinitialization
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Euler_Step_Of_Reinitialization (const ARRAYS_3D<T>& sign_phi, const T dt, const T time)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Fast_Marching_Method
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Fast_Marching_Method (const T time, const T stopping_distance, const LIST_ARRAY<VECTOR_3D<int> >* seed_indices)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Get_Signed_Distance_Using_FMM
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Get_Signed_Distance_Using_FMM (ARRAYS_3D<T>& signed_distance, const T time, const T stopping_distance, const LIST_ARRAY<VECTOR_3D<int> >* seed_indices)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Fast_Marching_Method_Outside_Band
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Fast_Marching_Method_Outside_Band (const T half_band_width, const T time, const T stopping_distance)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Fast_Marching_Method
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Fast_Marching_Method (ARRAYS_3D<T>& phi_ghost, const T stopping_distance, const LIST_ARRAY<VECTOR_3D<int> >* seed_indices)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Or_Add_Neighbor
//#####################################################################
template<class T> inline void LEVELSET_3D<T>::
Update_Or_Add_Neighbor (ARRAYS_3D<T>& phi_ghost, ARRAYS_3D<bool>& done, ARRAYS_3D<int>& close_k, ARRAYS_1D<VECTOR_3D<int> >& heap, int& heap_length, const VECTOR_3D<int>& neighbor)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Interface
//#####################################################################
// pass heap_length by reference
template<class T> void LEVELSET_3D<T>::
Initialize_Interface (ARRAYS_3D<T>& phi, ARRAYS_3D<bool>& done, ARRAYS_3D<int>& close_k, ARRAYS_1D<VECTOR_3D<int> >& heap, int& heap_length,
		      const LIST_ARRAY<VECTOR_3D<int> >* seed_indices)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Initial
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Add_To_Initial (ARRAYS_3D<bool>& done, ARRAYS_3D<int>& close_k, const int index, const int i, const int j, const int ij)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Close_Point
//#####################################################################
// needs done=0 around the outside of the domain
template<class T> void LEVELSET_3D<T>::
Update_Close_Point (ARRAYS_3D<T>& phi, const ARRAYS_3D<bool>& done, const int i, const int j, const int ij)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Fast_Sweeping_Method
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Fast_Sweeping_Method (const T time, const T stopping_distance, const LIST_ARRAY<VECTOR_3D<int> >* seed_indices)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Get_Signed_Distance_Using_FSM
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Get_Signed_Distance_Using_FSM (ARRAYS_3D<T>& signed_distance, const T time, const T stopping_distance, const LIST_ARRAY<VECTOR_3D<int> >* seed_indices)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Fast_Sweeping_Method_Outside_Band
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Fast_Sweeping_Method_Outside_Band (const T half_band_width, const T time, const T stopping_distance)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Fast_Sweeping_Method
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Fast_Sweeping_Method (ARRAYS_3D<T>& phi_ghost, const T stopping_distance)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Solve_Quadratic_3D
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Solve_Quadratic_3D (ARRAYS_3D<T>& phi, ARRAYS_3D<bool>& done, int i, int j, int ij)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Narrowband
//#####################################################################
// pass heap_length by reference
template<class T> void LEVELSET_3D<T>::
Initialize_Narrowband (ARRAYS_3D<T>& phi_ghost, const T stopping_distance)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Approximate_Surface_Area
//#####################################################################
// calculates the approximate perimeter using delta functions
template<class T> T LEVELSET_3D<T>::
Approximate_Surface_Area (const T interface_thickness, const T time) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Approximate_Negative_Volume
//#####################################################################
// calculates the approximate area using Heaviside functions
template<class T> T LEVELSET_3D<T>::
Approximate_Negative_Volume (const T interface_thickness, const T time) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Approximate_Positive_Volume
//#####################################################################
// calculates the approximate area using Heaviside functions
template<class T> T LEVELSET_3D<T>::
Approximate_Positive_Volume (const T interface_thickness, const T time) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Calculate_Signed_Distance_Function
//#####################################################################
template<class T> struct FIND_CELLS_TO_REFINE_HELPER
{
	const ARRAY<T>* phi;
	LIST_ARRAY<OCTREE_CELL<T>*>* cells_to_refine;
	const T* refinement_distance;
	int* maximum_tree_depth;
};
template<class T> static void Calculate_Signed_Distance_Function_Helper (FIND_CELLS_TO_REFINE_HELPER<T>* helper, const OCTREE_CELL<T>* cell)
{
	NOT_IMPLEMENTED();
}

template<class T> void LEVELSET_3D<T>::
Calculate_Signed_Distance_Function (OCTREE_GRID<T>& octree_grid, ARRAY<T>& phi, bool output_statistics) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Calculate_Signed_Distance_Function_Based_On_Curvature
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Calculate_Signed_Distance_Function_Based_On_Curvature (OCTREE_GRID<T>& octree_grid, ARRAYS_1D<T>& phi, const int levels, bool output_statistics)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Compare_Curvature_To_DX
//#####################################################################
template<class T> bool LEVELSET_3D<T>::
Compare_Curvature_To_DX (OCTREE_GRID<T>& octree_grid, const OCTREE_CELL<T>* cell_input) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Calculate_Triangulated_Surface_From_Marching_Tetrahedra
//#####################################################################
// uses levelset grid for tet marching - faster than version below because we don't need to interpolate phi
template<class T> void LEVELSET_3D<T>::
Calculate_Triangulated_Surface_From_Marching_Tetrahedra (TRIANGULATED_SURFACE<T>& triangulated_surface, const bool include_ghost_values) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function If_Zero_Crossing_Add_Particle
//#####################################################################
template<class T> int LEVELSET_3D<T>::
If_Zero_Crossing_Add_Particle_By_Index (TRIANGULATED_SURFACE<T>& triangulated_surface, const VECTOR_3D<int>& index1, const VECTOR_3D<int>& index2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Calculate_Triangulated_Surface_From_Marching_Tetrahedra
//#####################################################################
template<class T> void LEVELSET_3D<T>::
Calculate_Triangulated_Surface_From_Marching_Tetrahedra (const GRID_3D<T>& tet_grid, TRIANGULATED_SURFACE<T>& triangulated_surface) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function If_Zero_Crossing_Add_Particle
//#####################################################################
template<class T> int LEVELSET_3D<T>::
If_Zero_Crossing_Add_Particle (TRIANGULATED_SURFACE<T>& triangulated_surface, const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Append_Triangles
//#####################################################################
// looking down at the node with phi1, e1-e2-e3 is conunter clockwise
// edges 1,2,3 come out of phi1 - edge4 is opposite edge3 - edge2 is opposite edge6 - edge1 is opposite edge5
template<class T> void LEVELSET_3D<T>::
Append_Triangles (TRIANGULATED_SURFACE<T>& triangulated_surface, const int e1, const int e2, const int e3, const int e4, const int e5, const int e6, const T phi1) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################

template class LEVELSET_3D<float>;
template class LEVELSET_3D<double>;
