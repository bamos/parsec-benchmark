//#####################################################################
// Copyright 2002-2004, Ronald Fedkiw, Geoffrey Irving, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class CONSTITUTIVE_MODEL_3D
//#####################################################################
#ifndef __CONSTITUTIVE_MODEL_3D__
#define __CONSTITUTIVE_MODEL_3D__

namespace PhysBAM
{

template<class T> class MATRIX_3X3;
template<class T> class STRAIN_MEASURE_3D;

template<class T>
class CONSTITUTIVE_MODEL_3D
{
public:
	STRAIN_MEASURE_3D<T>& strain_measure;

	CONSTITUTIVE_MODEL_3D (STRAIN_MEASURE_3D<T>& strain_measure_input)
		: strain_measure (strain_measure_input)
	{}

	virtual ~CONSTITUTIVE_MODEL_3D()
	{}

	void Default() const
	{
		std::cout << "THIS CONSTITUTIVE_MODEL_3D FUNCTION IS NOT DEFINED!" << std::endl;
	}

//#####################################################################
	virtual MATRIX_3X3<T> P_From_Strain (const int tetrahedron_index) const
	{
		Default();
		exit (1);
	}
	virtual SYMMETRIC_MATRIX_3X3<T> S_From_Strain (const int tetrahedron_index) const
	{
		Default();
		exit (1);
	}
	virtual SYMMETRIC_MATRIX_3X3<T> Sigma_From_Strain (const int tetrahedron_index) const
	{
		Default();
		exit (1);
	}
	virtual MATRIX_3X3<T> P_From_Strain_Rate (const int tetrahedron_index, const MATRIX_3X3<T>& F) const
	{
		Default();
		exit (1);
	}
	virtual SYMMETRIC_MATRIX_3X3<T> Sigma_From_Strain_Rate (const int tetrahedron_index) const
	{
		Default();
		exit (1);
	}
	virtual SYMMETRIC_MATRIX_3X3<T> Sigma_From_Strain_Rate (const int tetrahedron_index, const MATRIX_3X3<T>& F, const T& one_over_J) const
	{
		Default();
		exit (1);
	}
	virtual T CFL_Elastic() const
	{
		Default();
		exit (1);
	}
	virtual T CFL_Damping() const
	{
		Default();
		exit (1);
	}
//#####################################################################
};
}
#endif
