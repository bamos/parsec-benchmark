//#####################################################################
// Copyright 2003, Geoffrey Irving
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class UNION_FIND
//#####################################################################
#ifndef __UNION_FIND__
#define __UNION_FIND__

#include "../Arrays/LIST_ARRAY.h"
namespace PhysBAM
{

class UNION_FIND
{
public:
	LIST_ARRAY<int> parents;

	explicit UNION_FIND (const int entries = 0)
	{
		parents.Resize_Array (entries);
	}

	void Initialize (const int entries)
	{
		parents.Remove_All_Entries();
		parents.Resize_Array (entries);
	}

	int Add_Entry()
	{
		parents.Append_Element (0);
		return parents.m;
	}

	bool Is_Root (const int i)
	{
		return !parents (i);
	}

	int Find (const int i)
	{
		int root = Find_Without_Path_Compression (i);
		Path_Compress (i, root);
		return root;
	}

	int Union (const int i, const int j)
	{
		int root_i = Find (i), root_j = Find_Without_Path_Compression (j);
		Path_Compress (j, root_i);

		if (root_i != root_j) parents (root_j) = root_i;

		return root_i;
	}

	void Compress_All()
	{
		for (int i = 1; i <= parents.m; i++) Path_Compress (i, Find_Without_Path_Compression (i));
	}

private:
	int Find_Without_Path_Compression (const int i) const
	{
		int j = i, k;

		while ( (k = parents (j))) j = k;

		return j;
	}

	void Path_Compress (const int i, const int root)
	{
		int j = i, k;

		while ( (k = parents (j)))
		{
			parents (j) = root;
			j = k;
		}
	}

//#####################################################################
};
}
#endif

