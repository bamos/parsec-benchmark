//#####################################################################
// Copyright 2004, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D
//#####################################################################
#ifndef __DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D__
#define __DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D__

namespace PhysBAM
{

template<class T> class MATRIX_3X3;

template<class T>
class DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D
{
public:
	T x1111, x2211, x3311, x2222, x3322, x3333; // 3x3 block
	T x2121, x2112; // 2x2 block
	T x3131, x3113; // 2x2 block
	T x3223, x3232; // 2x2 block

	MATRIX_3X3<T> Differential (const MATRIX_3X3<T>& dF) const
	{
		return MATRIX_3X3<T> (x1111 * dF.x[0] + x2211 * dF.x[4] + x3311 * dF.x[8], x2121 * dF.x[1] + x2112 * dF.x[3], x3131 * dF.x[2] + x3113 * dF.x[6],
				      x2112 * dF.x[1] + x2121 * dF.x[3], x2211 * dF.x[0] + x2222 * dF.x[4] + x3322 * dF.x[8], x3232 * dF.x[5] + x3223 * dF.x[7],
				      x3113 * dF.x[2] + x3131 * dF.x[6], x3223 * dF.x[5] + x3232 * dF.x[7], x3311 * dF.x[0] + x3322 * dF.x[4] + x3333 * dF.x[8]);
	}

	void Fix_Indefinite_Blocks()
	{
		SYMMETRIC_MATRIX_3X3<T> A1 (x1111, x2211, x3311, x2222, x3322, x3333);
		DIAGONAL_MATRIX_3X3<T> D1;
		MATRIX_3X3<T> V1;
		A1.Fast_Solve_Eigenproblem (D1, V1);
		D1 = D1.Max (0);
		A1 = SYMMETRIC_MATRIX_3X3<T>::Conjugate (V1, D1);
		x1111 = A1.x11;
		x2211 = A1.x21;
		x3311 = A1.x31;
		x2222 = A1.x22;
		x3322 = A1.x32;
		x3333 = A1.x33;
		SYMMETRIC_MATRIX_2X2<T> A2 (x2121, x2112, x2121);
		DIAGONAL_MATRIX_2X2<T> D2;
		MATRIX_2X2<T> V2;
		A2.Solve_Eigenproblem (D2, V2);
		D2 = D2.Max (0);
		A2 = SYMMETRIC_MATRIX_2X2<T>::Conjugate (V2, D2);
		x2121 = A2.x11;
		x2112 = A2.x21;
		SYMMETRIC_MATRIX_2X2<T> A3 (x3131, x3113, x3131);
		DIAGONAL_MATRIX_2X2<T> D3;
		MATRIX_2X2<T> V3;
		A3.Solve_Eigenproblem (D3, V3);
		D3 = D3.Max (0);
		A3 = SYMMETRIC_MATRIX_2X2<T>::Conjugate (V3, D3);
		x3131 = A3.x11;
		x3113 = A3.x21;
		SYMMETRIC_MATRIX_2X2<T> A4 (x3232, x3223, x3232);
		DIAGONAL_MATRIX_2X2<T> D4;
		MATRIX_2X2<T> V4;
		A4.Solve_Eigenproblem (D4, V4);
		D4 = D4.Max (0);
		A4 = SYMMETRIC_MATRIX_2X2<T>::Conjugate (V4, D4);
		x3232 = A4.x11;
		x3223 = A4.x21;
	}

//#####################################################################
};
}
#endif
