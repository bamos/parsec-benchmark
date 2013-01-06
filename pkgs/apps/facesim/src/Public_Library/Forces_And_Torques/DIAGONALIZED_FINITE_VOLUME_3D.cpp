//#####################################################################
// Copyright 2003-2004, Ron Fedkiw, Geoffrey Irving, Igor Neverov, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "DIAGONALIZED_FINITE_VOLUME_3D.h"
#include "../Thread_Utilities/THREAD_POOL.h"
#include "../Utilities/LOG.h"
#include "../Arrays/ARRAY_RANGE.h"
#include "../Arrays/ARRAYS_RANGE.h"
//#define USE_REDUCTION_ROUTINES
#ifdef USE_REDUCTION_ROUTINES
#include "../Arrays/ARRAY_PARALLEL_OPERATIONS.h"
#include "../Thread_Utilities/THREAD_ARRAY_LOCK.h"
#endif
using namespace PhysBAM;
extern bool PHYSBAM_THREADED_RUN;
//#define OUTPUT_THREADING_AUXILIARY_STRUCTURES
//#define OUTPUT_BENCHMARK_DATA
//#####################################################################
// Function Update_Threading_Auxiliary_Structures
//#####################################################################
inline int Extended_Segment (const LIST_ARRAYS<int>& extended_edges, const LIST_ARRAY<LIST_ARRAY<int> >& incident_extended_edges, const int i, const int j)
{
	for (int k = 1; k <= incident_extended_edges (i).m; k++)
	{
		int e = incident_extended_edges (i) (k), m, n;
		extended_edges.Get (e, m, n);
		assert (m == i || n == i);

		if (m == j || n == j) return e;
	}

	return 0;
}
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Update_Threading_Auxiliary_Structures()
{
#ifndef USE_REDUCTION_ROUTINES
	assert (strain_measure.tetrahedron_mesh.segment_mesh);

	if (!strain_measure.tetrahedron_mesh.segment_mesh->incident_segments) strain_measure.tetrahedron_mesh.segment_mesh->Initialize_Incident_Segments();

	if (!strain_measure.tetrahedron_mesh.neighbor_nodes) strain_measure.tetrahedron_mesh.Initialize_Neighbor_Nodes();

	if (!strain_measure.tetrahedron_mesh.incident_tetrahedrons) strain_measure.tetrahedron_mesh.Initialize_Incident_Tetrahedrons();

	LIST_ARRAY<LIST_ARRAY<int> >& neighbor_nodes = *strain_measure.tetrahedron_mesh.neighbor_nodes;
	LIST_ARRAY<LIST_ARRAY<int> >& incident_tetrahedrons = *strain_measure.tetrahedron_mesh.incident_tetrahedrons;
	assert (strain_measure.tetrahedron_mesh.tetrahedron_edges);
	LIST_ARRAYS<int>& tetrahedron_edges = *strain_measure.tetrahedron_mesh.tetrahedron_edges;
	LIST_ARRAYS<int>& tetrahedrons = strain_measure.tetrahedron_mesh.tetrahedrons;
	LIST_ARRAYS<int>& segments = strain_measure.tetrahedron_mesh.segment_mesh->segments;

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = *THREAD_POOL::Singleton();
#endif

	if (threading_auxiliary_structures) delete threading_auxiliary_structures;

	threading_auxiliary_structures = new DIAGONALIZED_FINITE_VOLUME_3D_THREADING_AUXILIARY_STRUCTURES<T>;
	int number_of_nodes = strain_measure.tetrahedralized_volume.particles.number;
	threading_auxiliary_structures->node_ranges = new ARRAY<VECTOR_2D<int> >;
	ARRAY<VECTOR_2D<int> >& node_ranges = *threading_auxiliary_structures->node_ranges;
	node_ranges = *strain_measure.tetrahedralized_volume.particles.particle_ranges;
	LIST_ARRAY<LIST_ARRAY<int> > internal_higher_neighbors (number_of_nodes), external_neighbors (number_of_nodes);
	threading_auxiliary_structures->extended_edges = new LIST_ARRAYS<int> (2, 0);
	LIST_ARRAYS<int>& extended_edges = *threading_auxiliary_structures->extended_edges;
#ifndef NEW_SERIAL_IMPLEMENTATIOM
	threading_auxiliary_structures->internal_edge_ranges = new ARRAY<VECTOR_2D<int> > (pool.number_of_threads);
	ARRAY<VECTOR_2D<int> >& internal_edge_ranges = *threading_auxiliary_structures->internal_edge_ranges;
	threading_auxiliary_structures->external_edge_ranges = new ARRAY<VECTOR_2D<int> > (pool.number_of_threads);
#else
	threading_auxiliary_structures->internal_edge_ranges = new ARRAY<VECTOR_2D<int> > (1);
	ARRAY<VECTOR_2D<int> >& internal_edge_ranges = *threading_auxiliary_structures->internal_edge_ranges;
	threading_auxiliary_structures->external_edge_ranges = new ARRAY<VECTOR_2D<int> > (1);
#endif
	ARRAY<VECTOR_2D<int> >& external_edge_ranges = *threading_auxiliary_structures->external_edge_ranges;

	for (int p = 1; p <= node_ranges.m; p++)
	{
		for (int i = node_ranges (p).x; i <= node_ranges (p).y; i++)
		{
			for (int j = 1; j <= neighbor_nodes (i).m; j++)
				if (neighbor_nodes (i) (j) < node_ranges (p).x || neighbor_nodes (i) (j) > node_ranges (p).y)
					external_neighbors (i).Append_Element (neighbor_nodes (i) (j));
				else if (neighbor_nodes (i) (j) > i)
					internal_higher_neighbors (i).Append_Element (neighbor_nodes (i) (j));

			LIST_ARRAY<int>::sort (internal_higher_neighbors (i));
			LIST_ARRAY<int>::sort (external_neighbors (i));
		}

		internal_edge_ranges (p).x = extended_edges.m + 1;

		for (int i = node_ranges (p).x; i <= node_ranges (p).y; i++) for (int j = 1; j <= internal_higher_neighbors (i).m; j++) extended_edges.Append_Element (i, internal_higher_neighbors (i) (j));

		internal_edge_ranges (p).y = extended_edges.m;
		external_edge_ranges (p).x = extended_edges.m + 1;

		for (int i = node_ranges (p).x; i <= node_ranges (p).y; i++) for (int j = 1; j <= external_neighbors (i).m; j++) extended_edges.Append_Element (i, external_neighbors (i) (j));

		external_edge_ranges (p).y = extended_edges.m;
	}

	LIST_ARRAY<LIST_ARRAY<int> > incident_extended_edges (number_of_nodes);

	for (int p = 1; p <= node_ranges.m; p++)
	{
		for (int e = internal_edge_ranges (p).x; e <= internal_edge_ranges (p).y; e++)
		{
			int m, n;
			extended_edges.Get (e, m, n);
			incident_extended_edges (m).Append_Element (e);
			incident_extended_edges (n).Append_Element (e);
		}

		for (int e = external_edge_ranges (p).x; e <= external_edge_ranges (p).y; e++)
		{
			int m, n;
			extended_edges.Get (e, m, n);
			incident_extended_edges (m).Append_Element (e);
		}
	}

	threading_auxiliary_structures->extended_tetrahedrons = new LIST_ARRAYS<int> (4, 0);
	LIST_ARRAYS<int>& extended_tetrahedrons = *threading_auxiliary_structures->extended_tetrahedrons;
#ifndef NEW_SERIAL_IMPLEMENTATIOM
	threading_auxiliary_structures->extended_tetrahedron_ranges = new ARRAY<VECTOR_2D<int> > (pool.number_of_threads);
#else
	threading_auxiliary_structures->extended_tetrahedron_ranges = new ARRAY<VECTOR_2D<int> > (1);
#endif
	ARRAY<VECTOR_2D<int> >& extended_tetrahedron_ranges = *threading_auxiliary_structures->extended_tetrahedron_ranges;
	threading_auxiliary_structures->extended_tetrahedron_extended_edges = new LIST_ARRAYS<int> (6, 0);
	LIST_ARRAYS<int>& extended_tetrahedron_extended_edges = *threading_auxiliary_structures->extended_tetrahedron_extended_edges;
	threading_auxiliary_structures->extended_tetrahedron_parents = new LIST_ARRAY<int>;
	LIST_ARRAY<int>& extended_tetrahedron_parents = *threading_auxiliary_structures->extended_tetrahedron_parents;

	for (int p = 1; p <= node_ranges.m; p++)
	{
		extended_tetrahedron_ranges (p).x = extended_tetrahedrons.m + 1;

		for (int i = node_ranges (p).x; i <= node_ranges (p).y; i++) for (int j = 1; j <= incident_tetrahedrons (i).m; j++)
			{
				ARRAY<int> vertex (4);
				tetrahedrons.Get (incident_tetrahedrons (i) (j), vertex (1), vertex (2), vertex (3), vertex (4));

				if ( (vertex (1) >= node_ranges (p).x && vertex (1) < i) || (vertex (2) >= node_ranges (p).x && vertex (2) < i) ||
						(vertex (3) >= node_ranges (p).x && vertex (3) < i) || (vertex (4) >= node_ranges (p).x && vertex (4) < i)) continue;

				extended_tetrahedrons.Append_Element (vertex (1), vertex (2), vertex (3), vertex (4));
				extended_tetrahedron_parents.Append_Element (incident_tetrahedrons (i) (j));
				ARRAY<int> edge (6);

				for (int v1 = 1, e = 1; v1 <= 3; v1++) for (int v2 = v1 + 1; v2 <= 4; v2++, e++)
					{
						if (vertex (v1) >= node_ranges (p).x && vertex (v1) <= node_ranges (p).y) edge (e) = Extended_Segment (extended_edges, incident_extended_edges, vertex (v1), vertex (v2));
						else if (vertex (v2) >= node_ranges (p).x && vertex (v2) <= node_ranges (p).y) edge (e) = Extended_Segment (extended_edges, incident_extended_edges, vertex (v2), vertex (v1));
						else edge (e) = 0;
					}

				extended_tetrahedron_extended_edges.Append_Element (edge (1), edge (2), edge (3), edge (4), edge (5), edge (6));
			}

		extended_tetrahedron_ranges (p).y = extended_tetrahedrons.m;
	}

	U.Resize_Array (0);
	De_inverse_hat.Resize_Array (0);
	Fe_hat.Resize_Array (0);

	if (edge_stiffness) delete edge_stiffness;

	edge_stiffness = 0;

	if (dP_dFe) delete dP_dFe;

	dP_dFe = 0;

	if (V) delete V;

	V = 0;
	threading_auxiliary_structures->extended_edge_stiffness = new LIST_ARRAY<MATRIX_3X3<T> >;
	threading_auxiliary_structures->extended_U = new LIST_ARRAY<MATRIX_3X3<T> >;
	threading_auxiliary_structures->extended_De_inverse_hat = new LIST_ARRAY<MATRIX_3X3<T> >;
	threading_auxiliary_structures->extended_Fe_hat = new LIST_ARRAY<DIAGONAL_MATRIX_3X3<T> >;
	threading_auxiliary_structures->extended_dP_dFe = new LIST_ARRAY<DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D<T> >;
	threading_auxiliary_structures->extended_V = new LIST_ARRAY<MATRIX_3X3<T> >;

#ifndef NEW_SERIAL_IMPLEMENTATIOM
#ifdef OUTPUT_THREADING_AUXILIARY_STRUCTURES
	FILE_UTILITIES::Write_To_File<T> (STRING_UTILITIES::string_sprintf ("threading_auxiliary_structures_%d.dat", pool.number_of_threads), *threading_auxiliary_structures);
	exit (0);
#endif

#ifdef OUTPUT_BENCHMARK_DATA
	FILE_UTILITIES::Write_To_File<T> (STRING_UTILITIES::string_sprintf ("extended_edges_%d.dat", pool.number_of_threads), extended_edges);
	FILE_UTILITIES::Write_To_File<T> (STRING_UTILITIES::string_sprintf ("node_ranges_%d.dat", pool.number_of_threads), node_ranges);
	FILE_UTILITIES::Write_To_File<T> (STRING_UTILITIES::string_sprintf ("internal_edge_ranges_%d.dat", pool.number_of_threads), internal_edge_ranges);
	FILE_UTILITIES::Write_To_File<T> (STRING_UTILITIES::string_sprintf ("external_edge_ranges_%d.dat", pool.number_of_threads), external_edge_ranges);
#endif

#else //NEW_SERIAL_IMPLEMENTATIOM

#ifdef OUTPUT_THREADING_AUXILIARY_STRUCTURES
	FILE_UTILITIES::Write_To_File<T> (STRING_UTILITIES::string_sprintf ("threading_auxiliary_structures_%d.dat", 1), *threading_auxiliary_structures);
	exit (0);
#endif

#ifdef OUTPUT_BENCHMARK_DATA
	FILE_UTILITIES::Write_To_File<T> (STRING_UTILITIES::string_sprintf ("extended_edges_%d.dat", 1), extended_edges);
	FILE_UTILITIES::Write_To_File<T> (STRING_UTILITIES::string_sprintf ("node_ranges_%d.dat", 1), node_ranges);
	FILE_UTILITIES::Write_To_File<T> (STRING_UTILITIES::string_sprintf ("internal_edge_ranges_%d.dat", 1), internal_edge_ranges);
	FILE_UTILITIES::Write_To_File<T> (STRING_UTILITIES::string_sprintf ("external_edge_ranges_%d.dat", 1), external_edge_ranges);
#endif

#endif //NEW_SERIAL_IMPLEMENTATIOM


#else
	edge_stiffness = new LIST_ARRAY<MATRIX_3X3<T> >;
#endif
}
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Read_Threading_Auxiliary_Structures()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Position_Based_State
//#####################################################################
template<class T> struct UPDATE_POSITION_BASED_STATE_HELPER
{
#ifndef USE_REDUCTION_ROUTINES
	DIAGONALIZED_FINITE_VOLUME_3D<T>const* diagonalized_finite_volume_3d;
	VECTOR_2D<int> node_range;
	VECTOR_2D<int> internal_edge_range;
	VECTOR_2D<int> external_edge_range;
	VECTOR_2D<int> extended_tetrahedron_range;
#else
	DIAGONALIZED_FINITE_VOLUME_3D<T>* diagonalized_finite_volume_3d;
	VECTOR_2D<int> element_range;
	THREAD_ARRAY_LOCK<int>* node_locks;
	THREAD_ARRAY_LOCK<int>* edge_locks;
#endif
};

template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Update_Position_Based_State()
{
	if (PHYSBAM_THREADED_RUN) Update_Position_Based_State_Parallel();

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	else Update_Position_Based_State_Serial();
#else
	else Update_Position_Based_State_Parallel();
#endif
}

template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Update_Position_Based_State_Helper (long thread_id, void* helper_raw)
{
#ifndef USE_REDUCTION_ROUTINES
	UPDATE_POSITION_BASED_STATE_HELPER<T>const& helper = * (UPDATE_POSITION_BASED_STATE_HELPER<T>*) helper_raw;
	DIAGONALIZED_FINITE_VOLUME_3D<T>const& diagonalized_finite_volume_3d = *helper.diagonalized_finite_volume_3d;
	DIAGONALIZED_FINITE_VOLUME_3D_THREADING_AUXILIARY_STRUCTURES<T>const& threading_auxiliary_structures = *helper.diagonalized_finite_volume_3d->threading_auxiliary_structures;
	LIST_ARRAYS<int>const& extended_edges = *threading_auxiliary_structures.extended_edges;
	LIST_ARRAYS<int>const& extended_tetrahedrons = *threading_auxiliary_structures.extended_tetrahedrons;
	LIST_ARRAY<SYMMETRIC_MATRIX_3X3<T> >& node_stiffness = *diagonalized_finite_volume_3d.node_stiffness;
	LIST_ARRAY<MATRIX_3X3<T> >& extended_edge_stiffness = *threading_auxiliary_structures.extended_edge_stiffness;
	VECTOR_2D<int>const& node_range = helper.node_range;
	VECTOR_2D<int>const& internal_edge_range = helper.internal_edge_range;
	VECTOR_2D<int>const& external_edge_range = helper.external_edge_range;
	VECTOR_2D<int>const& extended_tetrahedron_range = helper.extended_tetrahedron_range;
	STRAIN_MEASURE_3D<T>const& strain_measure = diagonalized_finite_volume_3d.strain_measure;
	DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T>& constitutive_model = diagonalized_finite_volume_3d.constitutive_model;
	LIST_ARRAY<T>const& Be_scales = diagonalized_finite_volume_3d.Be_scales;
	LIST_ARRAYS<int>const& extended_tetrahedron_extended_edges = *threading_auxiliary_structures.extended_tetrahedron_extended_edges;
	LIST_ARRAY<int>const& extended_tetrahedron_parents = *threading_auxiliary_structures.extended_tetrahedron_parents;
	LIST_ARRAY<MATRIX_3X3<T> >& extended_U = *threading_auxiliary_structures.extended_U;
	LIST_ARRAY<MATRIX_3X3<T> >& extended_De_inverse_hat = *threading_auxiliary_structures.extended_De_inverse_hat;
	LIST_ARRAY<DIAGONAL_MATRIX_3X3<T> >& extended_Fe_hat = *threading_auxiliary_structures.extended_Fe_hat;
	LIST_ARRAY<DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D<T> >& extended_dP_dFe = *threading_auxiliary_structures.extended_dP_dFe;
	LIST_ARRAY<MATRIX_3X3<T> >& extended_V = *threading_auxiliary_structures.extended_V;
	MATRIX_3X3<T> V_local;
	ARRAYS_2D<MATRIX_3X3<T> > dF_dX_local (1, 4, 1, 4);
	ARRAY<int> vertex (4), edge (6);

	for (int i = node_range.x; i <= node_range.y; i++) node_stiffness (i) = SYMMETRIC_MATRIX_3X3<T>();

	for (int e = internal_edge_range.x; e <= external_edge_range.y; e++) extended_edge_stiffness (e) = MATRIX_3X3<T>();

	for (int t = extended_tetrahedron_range.x; t <= extended_tetrahedron_range.y; t++)
	{
		int t_parent = extended_tetrahedron_parents (t);
		strain_measure.F (t_parent).Fast_Singular_Value_Decomposition (extended_U (t), extended_Fe_hat (t), V_local);
		constitutive_model.Isotropic_Stress_Derivative (extended_Fe_hat (t), extended_dP_dFe (t), t_parent);

		if (constitutive_model.anisotropic) constitutive_model.Update_State_Dependent_Auxiliary_Variables (extended_Fe_hat (t), V_local, t_parent);

		extended_De_inverse_hat (t) = strain_measure.Dm_inverse (t_parent) * V_local;
		extended_V (t) = V_local;

		for (int k = 1; k <= 3; k++) for (int l = 1; l <= 3; l++)
			{
				MATRIX_3X3<T> dDs, dG;
				dDs (k, l) = (T) 1;

				if (constitutive_model.anisotropic)
					dG = extended_U (t) * constitutive_model.dP_From_dF (extended_U (t).Transposed() * dDs * extended_De_inverse_hat (t), extended_Fe_hat (t), extended_V (t), extended_dP_dFe (t), Be_scales (t_parent), t_parent).Multiply_With_Transpose (extended_De_inverse_hat (t));
				else dG = extended_U (t) * constitutive_model.dP_From_dF (extended_U (t).Transposed() * dDs * extended_De_inverse_hat (t), extended_dP_dFe (t), Be_scales (t_parent), t_parent).Multiply_With_Transpose (extended_De_inverse_hat (t));

				for (int i = 1; i <= 3; i++) for (int j = 1; j <= 3; j++) dF_dX_local (l + 1, j + 1) (k, i) = dG (i, j);
			}

		for (int i = 2; i <= 4; i++)
		{
			dF_dX_local (i, 1) = MATRIX_3X3<T>();

			for (int j = 2; j <= 4; j++) dF_dX_local (i, 1) -= dF_dX_local (i, j);
		}

		for (int j = 1; j <= 4; j++)
		{
			dF_dX_local (1, j) = MATRIX_3X3<T>();

			for (int i = 2; i <= 4; i++) dF_dX_local (1, j) -= dF_dX_local (i, j);
		}

		extended_tetrahedrons.Get (t, vertex (1), vertex (2), vertex (3), vertex (4));

		for (int v = 1; v <= 4; v++) if (vertex (v) >= node_range.x && vertex (v) <= node_range.y) node_stiffness (vertex (v)) += dF_dX_local (v, v).Symmetric_Part();

		extended_tetrahedron_extended_edges.Get (t, edge (1), edge (2), edge (3), edge (4), edge (5), edge (6));

		for (int e = 1; e <= 6; e++) if (edge (e))
			{
				int i, j;
				extended_edges.Get (edge (e), i, j);
				int m, n;

				for (m = 1; m <= 4; m++) if (i == vertex (m)) break;

				for (n = 1; n <= 4; n++) if (j == vertex (n)) break;

				assert (m <= 4 && n <= 4 && m != n);
				extended_edge_stiffness (edge (e)) += dF_dX_local (m, n);
			}
	}

#else
	UPDATE_POSITION_BASED_STATE_HELPER<T>& helper = * (UPDATE_POSITION_BASED_STATE_HELPER<T>*) helper_raw;
	DIAGONALIZED_FINITE_VOLUME_3D<T>& diagonalized_finite_volume_3d = *helper.diagonalized_finite_volume_3d;
	LIST_ARRAY<SYMMETRIC_MATRIX_3X3<T> >* node_stiffness = diagonalized_finite_volume_3d.node_stiffness;
	LIST_ARRAY<MATRIX_3X3<T> >* edge_stiffness = diagonalized_finite_volume_3d.edge_stiffness;
	VECTOR_2D<int>const& element_range = helper.element_range;
	STRAIN_MEASURE_3D<T>const& strain_measure = diagonalized_finite_volume_3d.strain_measure;
	DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T>& constitutive_model = diagonalized_finite_volume_3d.constitutive_model;
	LIST_ARRAY<T>const& Be_scales = diagonalized_finite_volume_3d.Be_scales;
	LIST_ARRAY<MATRIX_3X3<T> >& U = diagonalized_finite_volume_3d.U;
	LIST_ARRAY<MATRIX_3X3<T> >& De_inverse_hat = diagonalized_finite_volume_3d.De_inverse_hat;
	LIST_ARRAY<DIAGONAL_MATRIX_3X3<T> >& Fe_hat = diagonalized_finite_volume_3d.Fe_hat;
	LIST_ARRAY<DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D<T> >* dP_dFe = diagonalized_finite_volume_3d.dP_dFe;
	LIST_ARRAY<MATRIX_3X3<T> >* V = diagonalized_finite_volume_3d.V;
	MATRIX_3X3<T> V_local;
	THREAD_ARRAY_LOCK<int>& node_locks = *helper.node_locks;
	THREAD_ARRAY_LOCK<int>& edge_locks = *helper.edge_locks;

	for (int t = element_range.x; t <= element_range.y; t++)
	{
		strain_measure.F (t).Fast_Singular_Value_Decomposition (U (t), Fe_hat (t), V_local);

		if (dP_dFe) constitutive_model.Isotropic_Stress_Derivative (Fe_hat (t), (*dP_dFe) (t), t);

		if (dP_dFe && constitutive_model.anisotropic) constitutive_model.Update_State_Dependent_Auxiliary_Variables (Fe_hat (t), V_local, t);

		De_inverse_hat (t) = strain_measure.Dm_inverse (t) * V_local;

		if (V) (*V) (t) = V_local;

		if (node_stiffness && edge_stiffness)
		{
			ARRAYS_2D<MATRIX_3X3<T> > dfdx (1, 4, 1, 4);

			for (int k = 1; k <= 3; k++) for (int l = 1; l <= 3; l++)
				{
					MATRIX_3X3<T> dDs, dG;
					dDs (k, l) = (T) 1;

					if (constitutive_model.anisotropic) dG = U (t) * constitutive_model.dP_From_dF (U (t).Transposed() * dDs * De_inverse_hat (t), Fe_hat (t), (*V) (t), (*dP_dFe) (t), Be_scales (t), t).Multiply_With_Transpose (De_inverse_hat (t));
					else dG = U (t) * constitutive_model.dP_From_dF (U (t).Transposed() * dDs * De_inverse_hat (t), (*dP_dFe) (t), Be_scales (t), t).Multiply_With_Transpose (De_inverse_hat (t));

					for (int i = 1; i <= 3; i++) for (int j = 1; j <= 3; j++) dfdx (l + 1, j + 1) (k, i) = dG (i, j);
				}

			for (int i = 2; i <= 4; i++) for (int j = 2; j <= 4; j++) dfdx (i, 1) -= dfdx (i, j);

			for (int j = 1; j <= 4; j++) for (int i = 2; i <= 4; i++) dfdx (1, j) -= dfdx (i, j);

			ARRAY<int> vertex (4);
			strain_measure.tetrahedron_mesh.tetrahedrons.Get (t, vertex (1), vertex (2), vertex (3), vertex (4));

			for (int v = 1; v <= 4; v++)
			{
				node_locks.Lock (vertex (v));
				(*node_stiffness) (vertex (v)) += dfdx (v, v).Symmetric_Part();
				node_locks.Unlock (vertex (v));
			}

			ARRAY<int> edge (6);
			strain_measure.tetrahedron_mesh.tetrahedron_edges->Get (t, edge (1), edge (2), edge (3), edge (4), edge (5), edge (6));

			for (int e = 1; e <= 6; e++)
			{
				int i, j;
				strain_measure.tetrahedron_mesh.segment_mesh->segments.Get (edge (e), i, j);
				int m, n;

				for (m = 1; m <= 4; m++) if (i == vertex (m)) break;

				for (n = 1; n <= 4; n++) if (j == vertex (n)) break;

				assert (m <= 4 && n <= 4 && m != n);
				edge_locks.Lock (edge (e));
				(*edge_stiffness) (edge (e)) += dfdx (m, n);
				edge_locks.Unlock (edge (e));
			}
		}
	}

#endif
}
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Update_Position_Based_State_Parallel()
{
#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = *THREAD_POOL::Singleton();
#endif
#ifdef USE_REDUCTION_ROUTINES
	THREAD_DIVISION_PARAMETERS<T>& parameters = *THREAD_DIVISION_PARAMETERS<T>::Singleton();
#endif

	LOG::Time ("UPBS (FEM) - Initialize");
#ifndef USE_REDUCTION_ROUTINES
	int extended_elements = threading_auxiliary_structures->extended_tetrahedrons->m;
	threading_auxiliary_structures->extended_U->Resize_Array (extended_elements);
	threading_auxiliary_structures->extended_De_inverse_hat->Resize_Array (extended_elements);
	threading_auxiliary_structures->extended_Fe_hat->Resize_Array (extended_elements);
	threading_auxiliary_structures->extended_dP_dFe->Resize_Array (extended_elements);
	threading_auxiliary_structures->extended_V->Resize_Array (extended_elements);
	node_stiffness->Resize_Array (strain_measure.tetrahedralized_volume.particles.number);
	threading_auxiliary_structures->extended_edge_stiffness->Resize_Array (threading_auxiliary_structures->extended_edges->m);
#else
	int elements = strain_measure.Dm_inverse.m;
	U.Resize_Array (elements);
	De_inverse_hat.Resize_Array (elements);
	Fe_hat.Resize_Array (elements);

	if (dP_dFe) dP_dFe->Resize_Array (elements);

	if (V) V->Resize_Array (elements);

	if (node_stiffness)
	{
		node_stiffness->Exact_Resize_Array (strain_measure.tetrahedralized_volume.particles.number);
		ARRAY<VECTOR_2D<int> > node_ranges;
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		parameters.Initialize_Array_Divisions (strain_measure.tetrahedralized_volume.particles.number, pool.number_of_threads, node_ranges);
#else
		parameters.Initialize_Array_Divisions (strain_measure.tetrahedralized_volume.particles.number, 1, node_ranges);
#endif
		ARRAY_PARALLEL_OPERATIONS<SYMMETRIC_MATRIX_3X3<T>, T, VECTOR_3D<T> >::Clear_Parallel (node_stiffness->array, node_ranges);
	}

	if (edge_stiffness)
	{
		edge_stiffness->Exact_Resize_Array (strain_measure.tetrahedron_mesh.segment_mesh->segments.m);
		ARRAY<VECTOR_2D<int> > edge_ranges;
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		parameters.Initialize_Array_Divisions (strain_measure.tetrahedron_mesh.segment_mesh->segments.m, pool.number_of_threads, edge_ranges);
#else
		parameters.Initialize_Array_Divisions (strain_measure.tetrahedron_mesh.segment_mesh->segments.m, 1, edge_ranges);
#endif
		ARRAY_PARALLEL_OPERATIONS<MATRIX_3X3<T>, T, VECTOR_3D<T> >::Clear_Parallel (edge_stiffness->array, edge_ranges);
	}

#endif

	LOG::Time ("UPBS (FEM) - Element Loop");
#ifdef USE_REDUCTION_ROUTINES
	ARRAY<VECTOR_2D<int> > element_ranges;
#ifndef NEW_SERIAL_IMPLEMENTATIOM
	parameters.Initialize_Array_Divisions (elements, pool.number_of_threads, element_ranges);
#else
	parameters.Initialize_Array_Divisions (elements, 1, element_ranges);
#endif
	THREAD_ARRAY_LOCK<int> node_locks, edge_locks;
#endif
#ifndef USE_REDUCTION_ROUTINES
#ifndef NEW_SERIAL_IMPLEMENTATIOM
	ARRAY<UPDATE_POSITION_BASED_STATE_HELPER<T> > helpers (pool.number_of_threads);
#else
	ARRAY<UPDATE_POSITION_BASED_STATE_HELPER<T> > helpers (1);
#endif

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).diagonalized_finite_volume_3d = this;
		helpers (i).node_range = (*threading_auxiliary_structures->node_ranges) (i);
		helpers (i).internal_edge_range = (*threading_auxiliary_structures->internal_edge_ranges) (i);
		helpers (i).external_edge_range = (*threading_auxiliary_structures->external_edge_ranges) (i);
		helpers (i).extended_tetrahedron_range = (*threading_auxiliary_structures->extended_tetrahedron_ranges) (i);
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Update_Position_Based_State_Helper, (void*) &helpers (i));
#else
		Update_Position_Based_State_Helper (i, (void*) &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
#else
#ifndef NEW_SERIAL_IMPLEMENTATIOM
	ARRAY<UPDATE_POSITION_BASED_STATE_HELPER<T> > helpers (pool.number_of_threads);
#else
	ARRAY<UPDATE_POSITION_BASED_STATE_HELPER<T> > helpers (1);
#endif

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).diagonalized_finite_volume_3d = this;
		helpers (i).element_range = element_ranges (i);
		helpers (i).node_locks = &node_locks;
		helpers (i).edge_locks = &edge_locks;
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Update_Position_Based_State_Helper, (void*) &helpers (i));
#else
		Update_Position_Based_State_Helper (i, (void*) &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
#endif
	LOG::Stop_Time();

#ifdef OUTPUT_BENCHMARK_DATA
#ifndef NEW_SERIAL_IMPLEMENTATIOM
	FILE_UTILITIES::Write_To_File<T> (STRING_UTILITIES::string_sprintf ("node_stiffness_%d.dat", pool.number_of_threads), *node_stiffness);
	FILE_UTILITIES::Write_To_File<T> (STRING_UTILITIES::string_sprintf ("extended_edge_stiffness_%d.dat", pool.number_of_threads), *threading_auxiliary_structures->extended_edge_stiffness);
#else
	FILE_UTILITIES::Write_To_File<T> (STRING_UTILITIES::string_sprintf ("node_stiffness_%d.dat", 1), *node_stiffness);
	FILE_UTILITIES::Write_To_File<T> (STRING_UTILITIES::string_sprintf ("extended_edge_stiffness_%d.dat", 1), *threading_auxiliary_structures->extended_edge_stiffness);
#endif
#endif
}
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Update_Position_Based_State_Serial()
{
	LOG::Time ("UPBS (FEM) - Initialize");
	int elements = strain_measure.Dm_inverse.m;
	U.Resize_Array (elements);
	De_inverse_hat.Resize_Array (elements);
	Fe_hat.Resize_Array (elements);

	if (dP_dFe) dP_dFe->Resize_Array (elements);

	if (V) V->Resize_Array (elements);

	if (node_stiffness)
	{
		node_stiffness->Resize_Array (strain_measure.tetrahedralized_volume.particles.number);
		LIST_ARRAY<SYMMETRIC_MATRIX_3X3<T> >::copy (SYMMETRIC_MATRIX_3X3<T>(), *node_stiffness);
	}

	if (edge_stiffness)
	{
		edge_stiffness->Resize_Array (strain_measure.tetrahedron_mesh.segment_mesh->segments.m);
		LIST_ARRAY<MATRIX_3X3<T> >::copy (MATRIX_3X3<T>(), *edge_stiffness);
	}

	MATRIX_3X3<T> V_local;
	LOG::Time ("UPBS (FEM) - Element loop");

	for (int t = 1; t <= elements; t++)
	{
		strain_measure.F (t).Fast_Singular_Value_Decomposition (U (t), Fe_hat (t), V_local);

		if (dP_dFe) constitutive_model.Isotropic_Stress_Derivative (Fe_hat (t), (*dP_dFe) (t), t);

		if (dP_dFe && constitutive_model.anisotropic) constitutive_model.Update_State_Dependent_Auxiliary_Variables (Fe_hat (t), V_local, t);

		De_inverse_hat (t) = strain_measure.Dm_inverse (t) * V_local;

		if (V) (*V) (t) = V_local;

		if (node_stiffness && edge_stiffness)
		{
			ARRAYS_2D<MATRIX_3X3<T> > dfdx (1, 4, 1, 4);

			for (int k = 1; k <= 3; k++) for (int l = 1; l <= 3; l++)
				{
					MATRIX_3X3<T> dDs, dG;
					dDs (k, l) = (T) 1;

					if (constitutive_model.anisotropic) dG = U (t) * constitutive_model.dP_From_dF (U (t).Transposed() * dDs * De_inverse_hat (t), Fe_hat (t), (*V) (t), (*dP_dFe) (t), Be_scales (t), t).Multiply_With_Transpose (De_inverse_hat (t));
					else dG = U (t) * constitutive_model.dP_From_dF (U (t).Transposed() * dDs * De_inverse_hat (t), (*dP_dFe) (t), Be_scales (t), t).Multiply_With_Transpose (De_inverse_hat (t));

					for (int i = 1; i <= 3; i++) for (int j = 1; j <= 3; j++) dfdx (l + 1, j + 1) (k, i) = dG (i, j);
				}

			for (int i = 2; i <= 4; i++) for (int j = 2; j <= 4; j++) dfdx (i, 1) -= dfdx (i, j);

			for (int j = 1; j <= 4; j++) for (int i = 2; i <= 4; i++) dfdx (1, j) -= dfdx (i, j);

			ARRAY<int> vertex (4);
			strain_measure.tetrahedron_mesh.tetrahedrons.Get (t, vertex (1), vertex (2), vertex (3), vertex (4));

			for (int v = 1; v <= 4; v++) (*node_stiffness) (vertex (v)) += dfdx (v, v).Symmetric_Part();

			ARRAY<int> edge (6);
			strain_measure.tetrahedron_mesh.tetrahedron_edges->Get (t, edge (1), edge (2), edge (3), edge (4), edge (5), edge (6));

			for (int e = 1; e <= 6; e++)
			{
				int i, j;
				strain_measure.tetrahedron_mesh.segment_mesh->segments.Get (edge (e), i, j);
				int m, n;

				for (m = 1; m <= 4; m++) if (i == vertex (m)) break;

				for (n = 1; n <= 4; n++) if (j == vertex (n)) break;

				assert (m <= 4 && n <= 4 && m != n);
				(*edge_stiffness) (edge (e)) += dfdx (m, n);
			}
		}
	}

	LOG::Stop_Time();
}
//#####################################################################
// Function Delete_Position_Based_State
//#####################################################################
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Delete_Position_Based_State()
{
	U.Resize_Array (0);
	De_inverse_hat.Resize_Array (0);
	Fe_hat.Resize_Array (0);

	if (V) V->Resize_Array (0);
}
//#####################################################################
// Function Add_Velocity_Independent_Forces
//#####################################################################
template<class T> struct ADD_VELOCITY_INDEPENDENT_FORCES_HELPER
{
	DIAGONALIZED_FINITE_VOLUME_3D<T>const* diagonalized_finite_volume_3d;
#ifndef USE_REDUCTION_ROUTINES
	VECTOR_2D<int> node_range;
	VECTOR_2D<int> extended_tetrahedron_range;
#else
	VECTOR_2D<int> element_range;
	THREAD_ARRAY_LOCK<int>* node_locks;
#endif
	ARRAY<VECTOR_3D<T> >* F;
};
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Add_Velocity_Independent_Forces (ARRAY<VECTOR_3D<T> >& F) const
{
	if (PHYSBAM_THREADED_RUN) Add_Velocity_Independent_Forces_Parallel (F);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	else Add_Velocity_Independent_Forces_Serial (F);

#else
	else Add_Velocity_Independent_Forces_Parallel (F);

#endif
}
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Add_Velocity_Independent_Forces_Helper (long thread_id, void* helper_raw)
{
#ifndef USE_REDUCTION_ROUTINES
	ADD_VELOCITY_INDEPENDENT_FORCES_HELPER<T>& helper = * (ADD_VELOCITY_INDEPENDENT_FORCES_HELPER<T>*) helper_raw;
	DIAGONALIZED_FINITE_VOLUME_3D<T>const& diagonalized_finite_volume_3d = *helper.diagonalized_finite_volume_3d;
	DIAGONALIZED_FINITE_VOLUME_3D_THREADING_AUXILIARY_STRUCTURES<T>const& threading_auxiliary_structures = *helper.diagonalized_finite_volume_3d->threading_auxiliary_structures;
	LIST_ARRAYS<int>const& extended_tetrahedrons = *threading_auxiliary_structures.extended_tetrahedrons;
	VECTOR_2D<int>const& node_range = helper.node_range;
	VECTOR_2D<int>const& extended_tetrahedron_range = helper.extended_tetrahedron_range;
	DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T>const& constitutive_model = diagonalized_finite_volume_3d.constitutive_model;
	LIST_ARRAY<T>const& Be_scales = diagonalized_finite_volume_3d.Be_scales;
	LIST_ARRAY<int>const& extended_tetrahedron_parents = *threading_auxiliary_structures.extended_tetrahedron_parents;
	LIST_ARRAY<MATRIX_3X3<T> >const& extended_U = *threading_auxiliary_structures.extended_U;
	LIST_ARRAY<MATRIX_3X3<T> >const& extended_De_inverse_hat = *threading_auxiliary_structures.extended_De_inverse_hat;
	LIST_ARRAY<DIAGONAL_MATRIX_3X3<T> >const& extended_Fe_hat = *threading_auxiliary_structures.extended_Fe_hat;
	LIST_ARRAY<MATRIX_3X3<T> >const& extended_V = *threading_auxiliary_structures.extended_V;
	ARRAY<VECTOR_3D<T> >& F = *helper.F;

	if (constitutive_model.anisotropic)
	{
		for (int t = extended_tetrahedron_range.x; t <= extended_tetrahedron_range.y; t++)
		{
			int t_parent = extended_tetrahedron_parents (t);
			MATRIX_3X3<T> forces = extended_U (t) * constitutive_model.P_From_Strain (extended_Fe_hat (t), extended_V (t), Be_scales (t_parent), t_parent).Multiply_With_Transpose (extended_De_inverse_hat (t));
			int node1;
			int node2;
			int node3;
			int node4;
			extended_tetrahedrons.Get (t, node1, node2, node3, node4);

			if (node1 >= node_range.x && node1 <= node_range.y) F (node1) -= VECTOR_3D<T> (forces.x[0] + forces.x[3] + forces.x[6], forces.x[1] + forces.x[4] + forces.x[7], forces.x[2] + forces.x[5] + forces.x[8]);

			if (node2 >= node_range.x && node2 <= node_range.y) F (node2) += VECTOR_3D<T> (forces.x[0], forces.x[1], forces.x[2]);

			if (node3 >= node_range.x && node3 <= node_range.y) F (node3) += VECTOR_3D<T> (forces.x[3], forces.x[4], forces.x[5]);

			if (node4 >= node_range.x && node4 <= node_range.y) F (node4) += VECTOR_3D<T> (forces.x[6], forces.x[7], forces.x[8]);
		}
	}
	else
	{
		for (int t = extended_tetrahedron_range.x; t <= extended_tetrahedron_range.y; t++)
		{
			int t_parent = extended_tetrahedron_parents (t);
			MATRIX_3X3<T> forces = extended_U (t) * constitutive_model.P_From_Strain (extended_Fe_hat (t), Be_scales (t_parent)).Multiply_With_Transpose (extended_De_inverse_hat (t));
			int node1;
			int node2;
			int node3;
			int node4;
			extended_tetrahedrons.Get (t, node1, node2, node3, node4);

			if (node1 >= node_range.x && node1 <= node_range.y) F (node1) -= VECTOR_3D<T> (forces.x[0] + forces.x[3] + forces.x[6], forces.x[1] + forces.x[4] + forces.x[7], forces.x[2] + forces.x[5] + forces.x[8]);

			if (node2 >= node_range.x && node2 <= node_range.y) F (node2) += VECTOR_3D<T> (forces.x[0], forces.x[1], forces.x[2]);

			if (node3 >= node_range.x && node3 <= node_range.y) F (node3) += VECTOR_3D<T> (forces.x[3], forces.x[4], forces.x[5]);

			if (node4 >= node_range.x && node4 <= node_range.y) F (node4) += VECTOR_3D<T> (forces.x[6], forces.x[7], forces.x[8]);
		}
	}

#else
	ADD_VELOCITY_INDEPENDENT_FORCES_HELPER<T>& helper = * (ADD_VELOCITY_INDEPENDENT_FORCES_HELPER<T>*) helper_raw;
	DIAGONALIZED_FINITE_VOLUME_3D<T>const& diagonalized_finite_volume_3d = *helper.diagonalized_finite_volume_3d;
	VECTOR_2D<int>const& element_range = helper.element_range;
	ARRAY<VECTOR_3D<T> >& F = *helper.F;
	STRAIN_MEASURE_3D<T>const& strain_measure = diagonalized_finite_volume_3d.strain_measure;
	DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T>& constitutive_model = diagonalized_finite_volume_3d.constitutive_model;
	LIST_ARRAY<T>const& Be_scales = diagonalized_finite_volume_3d.Be_scales;
	LIST_ARRAY<MATRIX_3X3<T> >const& U = diagonalized_finite_volume_3d.U;
	LIST_ARRAY<MATRIX_3X3<T> >const& De_inverse_hat = diagonalized_finite_volume_3d.De_inverse_hat;
	LIST_ARRAY<DIAGONAL_MATRIX_3X3<T> >const& Fe_hat = diagonalized_finite_volume_3d.Fe_hat;
	LIST_ARRAY<MATRIX_3X3<T> >const* V = diagonalized_finite_volume_3d.V;
	THREAD_ARRAY_LOCK<int>& node_locks = *helper.node_locks;

	if (constitutive_model.anisotropic)
	{
		for (int t = element_range.x; t <= element_range.y; t++)
		{
			MATRIX_3X3<T> forces = U (t) * constitutive_model.P_From_Strain (Fe_hat (t), (*V) (t), Be_scales (t), t).Multiply_With_Transpose (De_inverse_hat (t));
			int node1;
			int node2;
			int node3;
			int node4;
			strain_measure.tetrahedron_mesh.tetrahedrons.Get (t, node1, node2, node3, node4);
			node_locks.Lock (node1);
			F (node1) -= VECTOR_3D<T> (forces.x[0] + forces.x[3] + forces.x[6], forces.x[1] + forces.x[4] + forces.x[7], forces.x[2] + forces.x[5] + forces.x[8]);
			node_locks.Unlock (node1);
			node_locks.Lock (node2);
			F (node2) += VECTOR_3D<T> (forces.x[0], forces.x[1], forces.x[2]);
			node_locks.Unlock (node2);
			node_locks.Lock (node3);
			F (node3) += VECTOR_3D<T> (forces.x[3], forces.x[4], forces.x[5]);
			node_locks.Unlock (node3);
			node_locks.Lock (node4);
			F (node4) += VECTOR_3D<T> (forces.x[6], forces.x[7], forces.x[8]);
			node_locks.Unlock (node4);
		}
	}
	else
	{
		for (int t = element_range.x; t <= element_range.y; t++)
		{
			MATRIX_3X3<T> forces = U (t) * constitutive_model.P_From_Strain (Fe_hat (t), Be_scales (t)).Multiply_With_Transpose (De_inverse_hat (t));
			int node1;
			int node2;
			int node3;
			int node4;
			strain_measure.tetrahedron_mesh.tetrahedrons.Get (t, node1, node2, node3, node4);
			node_locks.Lock (node1);
			F (node1) -= VECTOR_3D<T> (forces.x[0] + forces.x[3] + forces.x[6], forces.x[1] + forces.x[4] + forces.x[7], forces.x[2] + forces.x[5] + forces.x[8]);
			node_locks.Unlock (node1);
			node_locks.Lock (node2);
			F (node2) += VECTOR_3D<T> (forces.x[0], forces.x[1], forces.x[2]);
			node_locks.Unlock (node2);
			node_locks.Lock (node3);
			F (node3) += VECTOR_3D<T> (forces.x[3], forces.x[4], forces.x[5]);
			node_locks.Unlock (node3);
			node_locks.Lock (node4);
			F (node4) += VECTOR_3D<T> (forces.x[6], forces.x[7], forces.x[8]);
			node_locks.Unlock (node4);
		}
	}

#endif
}
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Add_Velocity_Independent_Forces_Parallel (ARRAY<VECTOR_3D<T> >& F) const
{
#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = *THREAD_POOL::Singleton();
#endif

#ifdef USE_REDUCTION_ROUTINES
	THREAD_DIVISION_PARAMETERS<T>& parameters = *THREAD_DIVISION_PARAMETERS<T>::Singleton();
#endif

	LOG::Time ("AVIF (FEM)");
#ifdef USE_REDUCTION_ROUTINES
	ARRAY<VECTOR_2D<int> > element_ranges;
#ifndef NEW_SERIAL_IMPLEMENTATIOM
	parameters.Initialize_Array_Divisions (strain_measure.Dm_inverse.m, pool.number_of_threads, element_ranges);
#else
	parameters.Initialize_Array_Divisions (strain_measure.Dm_inverse.m, 1,  element_ranges);
#endif
	THREAD_ARRAY_LOCK<int> node_locks;
#endif
#ifndef NEW_SERIAL_IMPLEMENTATIOM
	ARRAY<ADD_VELOCITY_INDEPENDENT_FORCES_HELPER<T> > helpers (pool.number_of_threads);
#else
	ARRAY<ADD_VELOCITY_INDEPENDENT_FORCES_HELPER<T> > helpers (1);
#endif
#ifndef USE_REDUCTION_ROUTINES

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).diagonalized_finite_volume_3d = this;
		helpers (i).node_range = (*threading_auxiliary_structures->node_ranges) (i);
		helpers (i).extended_tetrahedron_range = (*threading_auxiliary_structures->extended_tetrahedron_ranges) (i);
		helpers (i).F = &F;
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Add_Velocity_Independent_Forces_Helper, (void*) &helpers (i));
#else
		Add_Velocity_Independent_Forces_Helper (i, (void*) &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
#else

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).diagonalized_finite_volume_3d = this;
		helpers (i).element_range = element_ranges (i);
		helpers (i).node_locks = &node_locks;
		helpers (i).F = &F;
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Add_Velocity_Independent_Forces_Helper, (void*) &helpers (i));
#else
		Add_Velocity_Independent_Forces_Helper (i, (void*) &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
#endif
	LOG::Stop_Time();
}
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Add_Velocity_Independent_Forces_Serial (ARRAY<VECTOR_3D<T> >& F) const
{
	LOG::Time ("AVIF (FEM)");

	if (constitutive_model.anisotropic)
	{
		for (int t = 1; t <= strain_measure.Dm_inverse.m; t++)
		{
			MATRIX_3X3<T> forces = U (t) * constitutive_model.P_From_Strain (Fe_hat (t), (*V) (t), Be_scales (t), t).Multiply_With_Transpose (De_inverse_hat (t));
			int node1;
			int node2;
			int node3;
			int node4;
			strain_measure.tetrahedron_mesh.tetrahedrons.Get (t, node1, node2, node3, node4);
			F (node1) -= VECTOR_3D<T> (forces.x[0] + forces.x[3] + forces.x[6], forces.x[1] + forces.x[4] + forces.x[7], forces.x[2] + forces.x[5] + forces.x[8]);
			F (node2) += VECTOR_3D<T> (forces.x[0], forces.x[1], forces.x[2]);
			F (node3) += VECTOR_3D<T> (forces.x[3], forces.x[4], forces.x[5]);
			F (node4) += VECTOR_3D<T> (forces.x[6], forces.x[7], forces.x[8]);
		}
	}
	else
	{
		for (int t = 1; t <= strain_measure.Dm_inverse.m; t++)
		{
			MATRIX_3X3<T> forces = U (t) * constitutive_model.P_From_Strain (Fe_hat (t), Be_scales (t)).Multiply_With_Transpose (De_inverse_hat (t));
			int node1;
			int node2;
			int node3;
			int node4;
			strain_measure.tetrahedron_mesh.tetrahedrons.Get (t, node1, node2, node3, node4);
			F (node1) -= VECTOR_3D<T> (forces.x[0] + forces.x[3] + forces.x[6], forces.x[1] + forces.x[4] + forces.x[7], forces.x[2] + forces.x[5] + forces.x[8]);
			F (node2) += VECTOR_3D<T> (forces.x[0], forces.x[1], forces.x[2]);
			F (node3) += VECTOR_3D<T> (forces.x[3], forces.x[4], forces.x[5]);
			F (node4) += VECTOR_3D<T> (forces.x[6], forces.x[7], forces.x[8]);
		}
	}

	LOG::Stop_Time();
}
//#####################################################################
// Function Add_Velocity_Dependent_Forces
//#####################################################################
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Add_Velocity_Dependent_Forces (ARRAY<VECTOR_3D<T> >& F) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Force_Differential
//#####################################################################
template<class T> struct ADD_FORCE_DIFFERENTIAL_HELPER
{
	DIAGONALIZED_FINITE_VOLUME_3D<T>const* diagonalized_finite_volume_3d;
#ifndef USE_REDUCTION_ROUTINES
	VECTOR_2D<int> node_range;
	VECTOR_2D<int> internal_edge_range;
	VECTOR_2D<int> external_edge_range;
#else
	VECTOR_2D<int> node_range;
	VECTOR_2D<int> edge_range;
	THREAD_ARRAY_LOCK<int>* node_locks;
#endif
	ARRAY<VECTOR_3D<T> >const* dX;
	ARRAY<VECTOR_3D<T> >* dF;
};
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Add_Force_Differential (const ARRAY<VECTOR_3D<T> >& dX, ARRAY<VECTOR_3D<T> >& dF) const
{
	if (PHYSBAM_THREADED_RUN) Add_Force_Differential_Parallel (dX, dF);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	else Add_Force_Differential_Serial (dX, dF);

#else
	else Add_Force_Differential_Parallel (dX, dF);

#endif
}
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Add_Force_Differential_Helper (long thread_id, void* helper_raw)
{
#ifndef USE_REDUCTION_ROUTINES
	ADD_FORCE_DIFFERENTIAL_HELPER<T>& helper = * (ADD_FORCE_DIFFERENTIAL_HELPER<T>*) helper_raw;
	DIAGONALIZED_FINITE_VOLUME_3D<T>const& diagonalized_finite_volume_3d = *helper.diagonalized_finite_volume_3d;
	DIAGONALIZED_FINITE_VOLUME_3D_THREADING_AUXILIARY_STRUCTURES<T>const& threading_auxiliary_structures = *helper.diagonalized_finite_volume_3d->threading_auxiliary_structures;
	LIST_ARRAYS<int>const& extended_edges = *threading_auxiliary_structures.extended_edges;
	LIST_ARRAY<SYMMETRIC_MATRIX_3X3<T> >const& node_stiffness = *diagonalized_finite_volume_3d.node_stiffness;
	LIST_ARRAY<MATRIX_3X3<T> >const& extended_edge_stiffness = *threading_auxiliary_structures.extended_edge_stiffness;
	VECTOR_2D<int>const& node_range = helper.node_range;
	VECTOR_2D<int>const& internal_edge_range = helper.internal_edge_range;
	VECTOR_2D<int>const& external_edge_range = helper.external_edge_range;
	ARRAY<VECTOR_3D<T> >const& dX = *helper.dX;
	ARRAY<VECTOR_3D<T> >& dF = *helper.dF;

	for (int v = node_range.x; v <= node_range.y; v++) dF (v) += node_stiffness (v) * dX (v);

	for (int e = internal_edge_range.x; e <= internal_edge_range.y; e++)
	{
		int m, n;
		extended_edges.Get (e, m, n);
		dF (m) += extended_edge_stiffness (e) * dX (n);
		dF (n) += extended_edge_stiffness (e).Transpose_Times (dX (m));
	}

	for (int e = external_edge_range.x; e <= external_edge_range.y; e++)
	{
		int m, n;
		extended_edges.Get (e, m, n);
		dF (m) += extended_edge_stiffness (e) * dX (n);
	}

#else
	ADD_FORCE_DIFFERENTIAL_HELPER<T>& helper = * (ADD_FORCE_DIFFERENTIAL_HELPER<T>*) helper_raw;
	DIAGONALIZED_FINITE_VOLUME_3D<T>const& diagonalized_finite_volume_3d = *helper.diagonalized_finite_volume_3d;
	VECTOR_2D<int>const& node_range = helper.node_range;
	VECTOR_2D<int>const& edge_range = helper.edge_range;
	ARRAY<VECTOR_3D<T> >const& dX = *helper.dX;
	ARRAY<VECTOR_3D<T> >& dF = *helper.dF;
	STRAIN_MEASURE_3D<T>const& strain_measure = diagonalized_finite_volume_3d.strain_measure;
	LIST_ARRAY<SYMMETRIC_MATRIX_3X3<T> >const& node_stiffness = *diagonalized_finite_volume_3d.node_stiffness;;
	LIST_ARRAY<MATRIX_3X3<T> >const& edge_stiffness = *diagonalized_finite_volume_3d.edge_stiffness;
	THREAD_ARRAY_LOCK<int>& node_locks = *helper.node_locks;

	for (int i = node_range.x; i <= node_range.y; i++)
	{
		node_locks.Lock (i);
		dF (i) += node_stiffness (i) * dX (i);
		node_locks.Unlock (i);
	}

	for (int e = edge_range.x; e <= edge_range.y; e++)
	{
		int m;
		int n;
		strain_measure.tetrahedron_mesh.segment_mesh->segments.Get (e, m, n);
		node_locks.Lock (m);
		dF (m) += edge_stiffness (e) * dX (n);
		node_locks.Unlock (m);
		node_locks.Lock (n);
		dF (n) += edge_stiffness (e).Transpose_Times (dX (m));
		node_locks.Unlock (n);
	}

#endif
}
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Add_Force_Differential_Parallel (const ARRAY<VECTOR_3D<T> >& dX, ARRAY<VECTOR_3D<T> >& dF) const
{
#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = *THREAD_POOL::Singleton();
#endif

#ifdef USE_REDUCTION_ROUTINES
	THREAD_DIVISION_PARAMETERS<T>& parameters = *THREAD_DIVISION_PARAMETERS<T>::Singleton();
#endif

	LOG::Time ("AFD (FEM)");
#ifdef USE_REDUCTION_ROUTINES
	ARRAY<VECTOR_2D<int> > node_ranges;
#ifndef NEW_SERIAL_IMPLEMENTATIOM
	parameters.Initialize_Array_Divisions (node_stiffness->m, pool.number_of_threads, node_ranges);
	ARRAY<VECTOR_2D<int> > edge_ranges;
	parameters.Initialize_Array_Divisions (edge_stiffness->m, pool.number_of_threads, edge_ranges);
#else
	parameters.Initialize_Array_Divisions (node_stiffness->m, 1, node_ranges);
	ARRAY<VECTOR_2D<int> > edge_ranges;
	parameters.Initialize_Array_Divisions (edge_stiffness->m, 1, edge_ranges);

#endif
	THREAD_ARRAY_LOCK<int> node_locks;
#endif //USE_REDUCTION_ROUTINES

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	ARRAY<ADD_FORCE_DIFFERENTIAL_HELPER<T> > helpers (pool.number_of_threads);
#else
	ARRAY<ADD_FORCE_DIFFERENTIAL_HELPER<T> > helpers (1);
#endif

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).diagonalized_finite_volume_3d = this;
#ifndef USE_REDUCTION_ROUTINES
		helpers (i).node_range = (*threading_auxiliary_structures->node_ranges) (i);
		helpers (i).internal_edge_range = (*threading_auxiliary_structures->internal_edge_ranges) (i);
		helpers (i).external_edge_range = (*threading_auxiliary_structures->external_edge_ranges) (i);
#else
		helpers (i).node_range = node_ranges (i);
		helpers (i).edge_range = edge_ranges (i);
		helpers (i).node_locks = &node_locks;
#endif
		helpers (i).dX = &dX;
		helpers (i).dF = &dF;
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Add_Force_Differential_Helper, (void*) &helpers (i));
#else
		Add_Force_Differential_Helper (i, (void*) &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
	LOG::Stop_Time();
}
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Add_Force_Differential (const ARRAY<VECTOR_3D<T> >& dX_full, ARRAY<VECTOR_3D<T> >& dF_full, const int partition_id) const
{
	//LOG::Push_Scope ("AFDS-I", "AFDS-I");
	const ARRAY_RANGE<const ARRAY<VECTOR_3D<T> > > dX (dX_full, (*threading_auxiliary_structures->node_ranges) (partition_id));
	ARRAY_RANGE<ARRAY<VECTOR_3D<T> > > dF (dF_full, (*threading_auxiliary_structures->node_ranges) (partition_id));
	const ARRAY_RANGE<const LIST_ARRAY<SYMMETRIC_MATRIX_3X3<T> > > node_stiffness (*this->node_stiffness, (*threading_auxiliary_structures->node_ranges) (partition_id));
	const ARRAYS_RANGE<const LIST_ARRAYS<int> > internal_edges (*threading_auxiliary_structures->extended_edges, (*threading_auxiliary_structures->internal_edge_ranges) (partition_id));
	const ARRAYS_RANGE<const LIST_ARRAYS<int> > external_edges (*threading_auxiliary_structures->extended_edges, (*threading_auxiliary_structures->external_edge_ranges) (partition_id));
	const ARRAY_RANGE<const LIST_ARRAY<MATRIX_3X3<T> > > internal_edge_stiffness (*threading_auxiliary_structures->extended_edge_stiffness, (*threading_auxiliary_structures->internal_edge_ranges) (partition_id));
	const ARRAY_RANGE<const LIST_ARRAY<MATRIX_3X3<T> > > external_edge_stiffness (*threading_auxiliary_structures->extended_edge_stiffness, (*threading_auxiliary_structures->external_edge_ranges) (partition_id));

	for (int p = 1; p <= node_stiffness.m; p++) dF (p) += node_stiffness (p) * dX (p);

	for (int e = 1; e <= internal_edges.m; e++)
	{
		int m, n;
		internal_edges.Get (e, m, n);
		dF_full (m) += internal_edge_stiffness (e) * dX_full (n);
		dF_full (n) += internal_edge_stiffness (e).Transpose_Times (dX_full (m));
	}

	for (int e = 1; e <= external_edges.m; e++)
	{
		int m, n;
		external_edges.Get (e, m, n);
		dF_full (m) += external_edge_stiffness (e) * dX_full (n);
	}
}
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Add_Force_Differential_Serial (const ARRAY<VECTOR_3D<T> >& dX, ARRAY<VECTOR_3D<T> >& dF) const
{
	//LOG::Time("AFD (FEM)");
	if (node_stiffness && edge_stiffness)
	{
		for (int i = 1; i <= strain_measure.tetrahedralized_volume.particles.number; i++) dF (i) += (*node_stiffness) (i) * dX (i);

		for (int e = 1; e <= strain_measure.tetrahedron_mesh.segment_mesh->segments.m; e++)
		{
			int m;
			int n;

			strain_measure.tetrahedron_mesh.segment_mesh->segments.Get (e, m, n);
			dF (m) += (*edge_stiffness) (e) * dX (n);
			dF (n) += (*edge_stiffness) (e).Transpose_Times (dX (m));
		}

	}
	else for (int t = 1; t <= strain_measure.Dm_inverse.m; t++)
		{
			int node1;
			int node2;
			int node3;
			int node4;
			strain_measure.tetrahedron_mesh.tetrahedrons.Get (t, node1, node2, node3, node4);
			MATRIX_3X3<T> dDs = MATRIX_3X3<T> (dX (node2) - dX (node1), dX (node3) - dX (node1), dX (node4) - dX (node1));
			MATRIX_3X3<T> dG;

			if (constitutive_model.anisotropic) dG = U (t) * constitutive_model.dP_From_dF (U (t).Transposed() * dDs * De_inverse_hat (t), Fe_hat (t), (*V) (t), (*dP_dFe) (t), Be_scales (t), t).Multiply_With_Transpose (De_inverse_hat (t));
			else dG = U (t) * constitutive_model.dP_From_dF (U (t).Transposed() * dDs * De_inverse_hat (t), (*dP_dFe) (t), Be_scales (t), t).Multiply_With_Transpose (De_inverse_hat (t));

			dF (node1) -= VECTOR_3D<T> (dG.x[0] + dG.x[3] + dG.x[6], dG.x[1] + dG.x[4] + dG.x[7], dG.x[2] + dG.x[5] + dG.x[8]);
			dF (node2) += VECTOR_3D<T> (dG.x[0], dG.x[1], dG.x[2]);
			dF (node3) += VECTOR_3D<T> (dG.x[3], dG.x[4], dG.x[5]);
			dF (node4) += VECTOR_3D<T> (dG.x[6], dG.x[7], dG.x[8]);
		}

	//LOG::Stop_Time();
}
//#####################################################################
// Function Intialize_CFL
//#####################################################################
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Initialize_CFL()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function CFL_Strain_Rate
//#####################################################################
template<class T> T DIAGONALIZED_FINITE_VOLUME_3D<T>::
CFL_Strain_Rate() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Semi_Implicit_Step
//#####################################################################
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Semi_Implicit_Impulse_Precomputation (const T time, const T cfl, const T max_dt, ARRAY<T>* time_plus_dt, const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Semi_Implicit_Recompute_Dt
//#####################################################################
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Semi_Implicit_Recompute_Dt (const int element, T& time_plus_dt)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Semi_Implicit_Step
//#####################################################################
template<class T> void DIAGONALIZED_FINITE_VOLUME_3D<T>::
Add_Semi_Implicit_Impulse (const int element, const T dt, T* time_plus_dt)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class DIAGONALIZED_FINITE_VOLUME_3D<float>;
template class DIAGONALIZED_FINITE_VOLUME_3D<double>;
