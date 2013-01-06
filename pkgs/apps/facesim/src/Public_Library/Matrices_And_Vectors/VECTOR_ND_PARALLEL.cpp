//#####################################################################
// Copyright 2002-2005, Ronald Fedkiw, Frank Losasso, Igor Neverov, Eftychios Sifakis, Joseph Teran, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class VECTOR_ND_PARALLEL
//#####################################################################
#include "VECTOR_ND_PARALLEL.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;

//#####################################################################
// Function Initialize_Thread_Helper
//#####################################################################/
template<class T> void VECTOR_ND_PARALLEL<T>::
Initialize_Thread_Helper()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Thread_Divisions
//#####################################################################/
template<class T> void VECTOR_ND_PARALLEL<T>::
Initialize_Thread_Divisions()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Double_Precision_Dot_Product
//#####################################################################
template<class T> double VECTOR_ND_PARALLEL<T>::
Double_Precision_Dot_Product (const VECTOR_ND_PARALLEL<T>& v1, const VECTOR_ND_PARALLEL<T>& v2)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function void Add_Scaled_Vector
//#####################################################################
template<class T> void VECTOR_ND_PARALLEL<T>::
Add_Scaled_Vector (const VECTOR_ND_PARALLEL<T>& v1, const T scalar, bool block)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function void Add_Scaled_Vector
//#####################################################################
template<class T> void VECTOR_ND_PARALLEL<T>::
Copy_Vector_Plus_Scaled_Vector (const VECTOR_ND_PARALLEL<T>& v1, const VECTOR_ND_PARALLEL<T>& v2, const T scalar, bool block)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function void Double_Precision_Maxabs
//#####################################################################
template<class T> double VECTOR_ND_PARALLEL<T>::
Double_Precision_Maxabs()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function void operator+=
//#####################################################################
template<class T> VECTOR_ND_PARALLEL<T>& VECTOR_ND_PARALLEL<T>::
operator+= (const T a)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function operator-=
//#####################################################################
template<class T> VECTOR_ND_PARALLEL<T>& VECTOR_ND_PARALLEL<T>::
operator-= (const T a)
{
	NOT_IMPLEMENTED();
}

//#####################################################################

template class VECTOR_ND_PARALLEL<float>;
template class VECTOR_ND_PARALLEL<double>;
template class VECTOR_ND_PARALLEL<int>;
