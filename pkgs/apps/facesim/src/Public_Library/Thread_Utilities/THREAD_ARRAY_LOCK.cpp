//#####################################################################
// Copyright 2005-2006, Andrew Selle, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "THREAD_ARRAY_LOCK.h"
#include "../Math_Tools/exchange_sort.h"
using namespace PhysBAM;
//#####################################################################
// Function THREAD_ARRAY_LOCK
//#####################################################################
template <class KT> THREAD_ARRAY_LOCK<KT>::
THREAD_ARRAY_LOCK (const int number_of_locks)
	: locks (number_of_locks)
{}
//#####################################################################
// Function Lock
//#####################################################################
template <class KT> void THREAD_ARRAY_LOCK<KT>::
Lock (const KT key)
{
	locks (Index (key)).Lock();
}
//#####################################################################
// Function Unlock
//#####################################################################
template <class KT> void THREAD_ARRAY_LOCK<KT>::
Unlock (const KT key)
{
	locks (Index (key)).Unlock();
}
//#####################################################################
// Function Lock_Ascending
//#####################################################################
template <class KT> void THREAD_ARRAY_LOCK<KT>::
Lock_Ascending (const KT key1, const KT key2, const KT key3, const KT key4)
{
	int index1 = Index (key1), index2 = Index (key2), index3 = Index (key3), index4 = Index (key4);
	exchange_sort (index1, index2, index3, index4);
	locks (index1).Lock();

	if (index2 != index1) locks (index2).Lock();

	if (index3 != index2) locks (index3).Lock();

	if (index4 != index3) locks (index4).Lock();
}
//#####################################################################
// Function Unlock_Descending
//#####################################################################
template <class KT> void THREAD_ARRAY_LOCK<KT>::
Unlock_Descending (const KT key1, const KT key2, const KT key3, const KT key4)
{
	int index1 = Index (key1), index2 = Index (key2), index3 = Index (key3), index4 = Index (key4);
	exchange_sort (index1, index2, index3, index4);
	locks (index4).Unlock();

	if (index3 != index4) locks (index3).Unlock();

	if (index2 != index3) locks (index2).Unlock();

	if (index1 != index2) locks (index1).Unlock();
}
//#####################################################################


template class THREAD_ARRAY_LOCK<int>;
template class THREAD_ARRAY_LOCK<VECTOR_2D<int> >;
template class THREAD_ARRAY_LOCK<VECTOR_3D<int> >;
