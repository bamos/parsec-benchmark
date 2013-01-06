//#####################################################################
// Copyright 2005-2006, Andrew Selle, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#ifndef __THREAD_ARRAY_LOCK__
#define __THREAD_ARRAY_LOCK__
#include "THREAD_LOCK.h"
#include "../Arrays/ARRAY.h"
#include "../Math_Tools/hash_function.h"

namespace PhysBAM
{

template<class KT>
class THREAD_ARRAY_LOCK
{
private:
	ARRAY<THREAD_LOCK> locks;
public:
	THREAD_ARRAY_LOCK (const int number_of_locks = 16381); // was 1023
	void Lock (const KT key);
	void Unlock (const KT key);
	void Lock_Ascending (const KT key1, const KT key2, const KT key3, const KT key4);
	void Unlock_Descending (const KT key1, const KT key2, const KT key3, const KT key4);

	int Index (const KT& v) const
	{
		return (unsigned int) Hash (v) % (unsigned int) locks.m + 1;
	}

//#####################################################################
};
}
#endif
