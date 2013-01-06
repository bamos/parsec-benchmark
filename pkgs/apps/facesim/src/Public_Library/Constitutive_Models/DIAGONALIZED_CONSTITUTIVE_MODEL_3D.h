//#####################################################################
// Copyright 2003-2004, Ron Fedkiw, Geoffrey Irving, Igor Neverov, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DIAGONALIZED_CONSTITUTIVE_MODEL_3D
//#####################################################################
#ifndef __DIAGONALIZED_CONSTITUTIVE_MODEL_3D__
#define __DIAGONALIZED_CONSTITUTIVE_MODEL_3D__

#include "DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D.h"
namespace PhysBAM
{

template<class T> class MATRIX_3X3;
template<class T> class DIAGONAL_MATRIX_3X3;
template<class T> class SYMMETRIC_MATRIX_3X3;

template<class T>
class DIAGONALIZED_CONSTITUTIVE_MODEL_3D
{
public:
	bool anisotropic, enforce_definiteness;
	T lambda, mu; // Lame coefficients (used by almost all derived models)
	T alpha, beta; // isotropic damping parameters (used by all current derived models)

	DIAGONALIZED_CONSTITUTIVE_MODEL_3D()
		: anisotropic (false), enforce_definiteness (false), lambda (0), mu (0), alpha (0), beta (0)
	{}

	virtual ~DIAGONALIZED_CONSTITUTIVE_MODEL_3D()
	{}

	virtual MATRIX_3X3<T> dP_From_dF (const MATRIX_3X3<T>& dF, const DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D<T>& dP_dF, const T scale, const int tetrahedron_index) const
	{
		return scale * dP_dF.Differential (dF);
	}

	virtual T CFL_Elastic (const T density, const T minimum_altitude) const
	{
		T elastic_sound_speed_inverse = sqrt (density / (lambda + 2 * mu));
		return minimum_altitude * elastic_sound_speed_inverse;
	}

	virtual T CFL_Damping (const T density, const T minimum_altitude) const
	{
		T damping_sound_speed_inverse = density * minimum_altitude / (alpha + 2 * beta);
		return minimum_altitude * damping_sound_speed_inverse;
	}

	void Default() const
	{
		std::cout << "THIS DIAGONALIZED_CONSTITUTIVE_MODEL_3D FUNCTION IS NOT DEFINED!" << std::endl;
	}

//#####################################################################
	virtual DIAGONAL_MATRIX_3X3<T> P_From_Strain (const DIAGONAL_MATRIX_3X3<T>& F, const T scale) const
	{
		Default();
		exit (1);
	}
	virtual MATRIX_3X3<T> P_From_Strain (const DIAGONAL_MATRIX_3X3<T>& F, const MATRIX_3X3<T>& V, const T scale, const int tetrahedron_index = 0) const
	{
		Default();
		exit (1);
	}
	virtual MATRIX_3X3<T> P_From_Strain_Rate (const DIAGONAL_MATRIX_3X3<T>& F, const MATRIX_3X3<T>& F_dot, const T scale) const
	{
		Default();
		exit (1);
	}
	virtual MATRIX_3X3<T> dP_From_dF (const MATRIX_3X3<T>& dF, const DIAGONAL_MATRIX_3X3<T>& F, const MATRIX_3X3<T>& V, const DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D<T>& dP_dF,
					  const T scale, const int tetrahedron_index = 0) const
	{
		Default();
		exit (1);
	}
	virtual void Isotropic_Stress_Derivative (const DIAGONAL_MATRIX_3X3<T>& F, DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D<T>& dP_dF, const int tetrahedron_index = 0) const
	{
		Default();
		exit (1);
	}
	virtual void Update_State_Dependent_Auxiliary_Variables (const DIAGONAL_MATRIX_3X3<T>& F, const MATRIX_3X3<T>& V, const int tetrahedron_index = 0) {}
//#####################################################################
};
}
#endif
