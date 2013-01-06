//#####################################################################
// Copyright 2003-2004, Ron Fedkiw, Geoffrey Irving, Neil Molino, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DIAGONALIZED_FINITE_VOLUME_3D
//#####################################################################
#ifndef __DIAGONALIZED_FINITE_VOLUME_3D__
#define __DIAGONALIZED_FINITE_VOLUME_3D__

#include "SOLIDS_FORCES.h"
#include "../Constitutive_Models/STRAIN_MEASURE_3D.h"
#include "../Constitutive_Models/DIAGONALIZED_CONSTITUTIVE_MODEL_3D.h"
#include "DIAGONALIZED_SEMI_IMPLICIT_ELEMENT_3D.h"
#include "../Grids/SEGMENT_MESH.h"
#include "../Thread_Utilities/THREAD_DIVISION_PARAMETERS.h"

//#define READ_THREADING_AUXILIARY_STRUCTURES_FROM_SNAPSHOT

namespace PhysBAM
{

template<class T>
class DIAGONALIZED_FINITE_VOLUME_3D_THREADING_AUXILIARY_STRUCTURES
{
public:
	LIST_ARRAYS<int>* extended_edges;
	LIST_ARRAYS<int>* extended_tetrahedrons;
	LIST_ARRAYS<int>* extended_tetrahedron_extended_edges;
	LIST_ARRAY<MATRIX_3X3<T> >* extended_edge_stiffness;
	ARRAY<VECTOR_2D<int> >* node_ranges;
	ARRAY<VECTOR_2D<int> >* internal_edge_ranges;
	ARRAY<VECTOR_2D<int> >* external_edge_ranges;
	ARRAY<VECTOR_2D<int> >* extended_tetrahedron_ranges;
	LIST_ARRAY<int>* extended_tetrahedron_parents;
	LIST_ARRAY<MATRIX_3X3<T> >* extended_U;
	LIST_ARRAY<MATRIX_3X3<T> >* extended_De_inverse_hat;
	LIST_ARRAY<DIAGONAL_MATRIX_3X3<T> >* extended_Fe_hat;
	LIST_ARRAY<DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D<T> >* extended_dP_dFe;
	LIST_ARRAY<MATRIX_3X3<T> >* extended_V;

	DIAGONALIZED_FINITE_VOLUME_3D_THREADING_AUXILIARY_STRUCTURES()
		: extended_edges (0), extended_tetrahedrons (0), extended_tetrahedron_extended_edges (0), extended_edge_stiffness (0), node_ranges (0), internal_edge_ranges (0), external_edge_ranges (0),
		  extended_tetrahedron_ranges (0), extended_tetrahedron_parents (0), extended_U (0), extended_De_inverse_hat (0), extended_Fe_hat (0), extended_dP_dFe (0), extended_V (0)
	{}

	~DIAGONALIZED_FINITE_VOLUME_3D_THREADING_AUXILIARY_STRUCTURES()
	{
		delete extended_edges;
		delete extended_tetrahedrons;
		delete extended_tetrahedron_extended_edges;
		delete extended_edge_stiffness;
		delete node_ranges;
		delete internal_edge_ranges;
		delete external_edge_ranges;
		delete extended_tetrahedron_ranges;
		delete extended_tetrahedron_parents;
		delete extended_U;
		delete extended_De_inverse_hat;
		delete extended_Fe_hat;
		delete extended_dP_dFe;
		delete extended_V;
	}

	void Allocate()
	{
		extended_edges = new LIST_ARRAYS<int>;
		extended_tetrahedrons = new LIST_ARRAYS<int>;
		extended_tetrahedron_extended_edges = new LIST_ARRAYS<int>;
		extended_edge_stiffness = new LIST_ARRAY<MATRIX_3X3<T> >;
		node_ranges = new ARRAY<VECTOR_2D<int> >;
		internal_edge_ranges = new ARRAY<VECTOR_2D<int> >;
		external_edge_ranges = new ARRAY<VECTOR_2D<int> >;
		extended_tetrahedron_ranges = new ARRAY<VECTOR_2D<int> >;
		extended_tetrahedron_parents = new LIST_ARRAY<int>;
		extended_U = new LIST_ARRAY<MATRIX_3X3<T> >;
		extended_De_inverse_hat = new LIST_ARRAY<MATRIX_3X3<T> >;
		extended_Fe_hat = new LIST_ARRAY<DIAGONAL_MATRIX_3X3<T> >;
		extended_dP_dFe = new LIST_ARRAY<DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D<T> >;
		extended_V = new LIST_ARRAY<MATRIX_3X3<T> >;
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		extended_edges->template Read<RW> (input_stream);
		extended_tetrahedrons->template Read<RW> (input_stream);
		extended_tetrahedron_extended_edges->template Read<RW> (input_stream);
		node_ranges->template Read<RW> (input_stream);
		internal_edge_ranges->template Read<RW> (input_stream);
		external_edge_ranges->template Read<RW> (input_stream);
		extended_tetrahedron_ranges->template Read<RW> (input_stream);
		extended_tetrahedron_parents->template Read<RW> (input_stream);
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		extended_edges->template Write<RW> (output_stream);
		extended_tetrahedrons->template Write<RW> (output_stream);
		extended_tetrahedron_extended_edges->template Write<RW> (output_stream);
		node_ranges->template Write<RW> (output_stream);
		internal_edge_ranges->template Write<RW> (output_stream);
		external_edge_ranges->template Write<RW> (output_stream);
		extended_tetrahedron_ranges->template Write<RW> (output_stream);
		extended_tetrahedron_parents->template Write<RW> (output_stream);
	}
};

template<class T>
class DIAGONALIZED_FINITE_VOLUME_3D: public SOLIDS_FORCES<T, VECTOR_3D<T> >
{
public:
	using SOLIDS_FORCES<T, VECTOR_3D<T> >::particles;
	using SOLIDS_FORCES<T, VECTOR_3D<T> >::CFL_initialized;
	using SOLIDS_FORCES<T, VECTOR_3D<T> >::CFL_elastic_time_step;
	using SOLIDS_FORCES<T, VECTOR_3D<T> >::CFL_damping_time_step;
	using SOLIDS_FORCES<T, VECTOR_3D<T> >::max_strain_per_time_step;

	STRAIN_MEASURE_3D<T>& strain_measure;
	DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T>& constitutive_model;
	LIST_ARRAY<T> Be_scales;
	LIST_ARRAY<MATRIX_3X3<T> > U;
	LIST_ARRAY<MATRIX_3X3<T> > De_inverse_hat;
	LIST_ARRAY<DIAGONAL_MATRIX_3X3<T> > Fe_hat;
	LIST_ARRAY<DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D<T> >* dP_dFe;
	LIST_ARRAY<T>* Be_scales_save;
	LIST_ARRAY<MATRIX_3X3<T> >* V;
	T twice_max_strain_per_time_step; // for asynchronous
	LIST_ARRAY<DIAGONALIZED_SEMI_IMPLICIT_ELEMENT_3D<T> >* semi_implicit_data;
	LIST_ARRAY<SYMMETRIC_MATRIX_3X3<T> >* node_stiffness;
	LIST_ARRAY<MATRIX_3X3<T> >* edge_stiffness;
	DIAGONALIZED_FINITE_VOLUME_3D_THREADING_AUXILIARY_STRUCTURES<T>* threading_auxiliary_structures;

	DIAGONALIZED_FINITE_VOLUME_3D (STRAIN_MEASURE_3D<T>& strain_measure_input, DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T>& constitutive_model_input)
		: SOLIDS_FORCES<T, VECTOR_3D<T> > (strain_measure_input.tetrahedralized_volume.particles), strain_measure (strain_measure_input), constitutive_model (constitutive_model_input),
		  dP_dFe (0), Be_scales_save (0), V (0), semi_implicit_data (0), node_stiffness (0), edge_stiffness (0), threading_auxiliary_structures (0)
	{
		Update_Be_Scales();

		if (constitutive_model.anisotropic) Save_V();
	}

	~DIAGONALIZED_FINITE_VOLUME_3D()
	{
		delete dP_dFe;
		delete Be_scales_save;
		delete V;
		delete semi_implicit_data;
		delete node_stiffness;
		delete edge_stiffness;
		delete threading_auxiliary_structures;
	}

	void Save_V()
	{
		if (!V) V = new LIST_ARRAY<MATRIX_3X3<T> >;
	}

	void Save_Stress_Derivative()
	{
		if (!dP_dFe) dP_dFe = new LIST_ARRAY<DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D<T> >;
	}

	void Use_Quasistatics()
	{
		Save_V();
		Save_Stress_Derivative();
	}

	void Use_Stiffness_Matrix()
	{
		node_stiffness = new LIST_ARRAY<SYMMETRIC_MATRIX_3X3<T> >;

		if (!THREAD_DIVISION_PARAMETERS<T>::Thread_Divisions_Enabled()) edge_stiffness = new LIST_ARRAY<MATRIX_3X3<T> >;

		if (!strain_measure.tetrahedron_mesh.segment_mesh) strain_measure.tetrahedron_mesh.Initialize_Segment_Mesh();

		if (!strain_measure.tetrahedron_mesh.tetrahedron_edges) strain_measure.tetrahedron_mesh.Initialize_Tetrahedron_Edges();

		if (THREAD_DIVISION_PARAMETERS<T>::Thread_Divisions_Enabled())
#ifdef READ_THREADING_AUXILIARY_STRUCTURES_FROM_SNAPSHOT
			Read_Threading_Auxiliary_Structures();

#else
			Update_Threading_Auxiliary_Structures();
#endif
	}

	void Enforce_Definiteness (const bool enforce_definiteness_input)
	{
		constitutive_model.enforce_definiteness = enforce_definiteness_input;
	}

	void Update_Be_Scales()
	{
		Be_scales.Resize_Array (strain_measure.Dm_inverse.m);

		for (int t = 1; t <= Be_scales.m; t++) Be_scales (t) = - (T) one_sixth / strain_measure.Dm_inverse (t).Determinant();
	}

	void Initialize_Be_Scales_Save()
	{
		Be_scales_save = new LIST_ARRAY<T> (Be_scales.m);
		LIST_ARRAY<T>::copy (Be_scales, *Be_scales_save);
	}

	void Copy_Be_Scales_Save_Into_Be_Scales (const LIST_ARRAY<int>& map)
	{
		Be_scales.Resize_Array (map.m);

		for (int t = 1; t <= map.m; t++) Be_scales (t) = (*Be_scales_save) (map (t));
	}

//#####################################################################
	void Update_Threading_Auxiliary_Structures();
	void Read_Threading_Auxiliary_Structures();
	void Update_Position_Based_State();
	static void Update_Position_Based_State_Helper (long thread_id, void* helper_raw);
	void Update_Position_Based_State_Parallel();
	void Update_Position_Based_State_Serial();
	void Delete_Position_Based_State();
	void Add_Velocity_Independent_Forces (ARRAY<VECTOR_3D<T> >& F) const;
	static void Add_Velocity_Independent_Forces_Helper (long thread_id, void* helper_raw);
	void Add_Velocity_Independent_Forces_Parallel (ARRAY<VECTOR_3D<T> >& F) const;
	void Add_Velocity_Independent_Forces_Serial (ARRAY<VECTOR_3D<T> >& F) const;
	void Add_Velocity_Dependent_Forces (ARRAY<VECTOR_3D<T> >& F) const;
	void Add_Force_Differential (const ARRAY<VECTOR_3D<T> >& dX, ARRAY<VECTOR_3D<T> >& dF) const;
	static void Add_Force_Differential_Helper (long thread_id, void* helper_raw);
	void Add_Force_Differential_Parallel (const ARRAY<VECTOR_3D<T> >& dX, ARRAY<VECTOR_3D<T> >& dF) const;
	void Add_Force_Differential (const ARRAY<VECTOR_3D<T> >& dX_full, ARRAY<VECTOR_3D<T> >& dF_full, const int partition_id) const;
	void Add_Force_Differential_Serial (const ARRAY<VECTOR_3D<T> >& dX, ARRAY<VECTOR_3D<T> >& dF) const;
	void Initialize_CFL();
	T CFL_Strain_Rate() const;
	void Semi_Implicit_Impulse_Precomputation (const T time, const T cfl, const T max_dt, ARRAY<T>* time_plus_dt, const bool verbose);
	void Semi_Implicit_Recompute_Dt (const int element, T& time_plus_dt);
	void Add_Semi_Implicit_Impulse (const int element, const T dt, T* time_plus_dt);
//#####################################################################
};
}
#endif
