//#####################################################################
// Copyright 2004, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SPLAY_TREE_NODE
//#####################################################################
#ifndef __SPLAY_TREE_NODE__
#define __SPLAY_TREE_NODE__

namespace PhysBAM
{

template<class T, class TV>
class SPLAY_TREE_NODE
{
public:
	SPLAY_TREE_NODE<T, TV> *left, *right;
	T key;
	TV value;

	SPLAY_TREE_NODE (SPLAY_TREE_NODE<T, TV>*const left_input, SPLAY_TREE_NODE<T, TV>*const right_input, const T& key_input, const TV& value_input)
		: left (left_input), right (right_input), key (key_input), value (value_input)
	{}

	~SPLAY_TREE_NODE()
	{
		delete left;
		delete right;
	}

	template<class T2> void Iterate (T2 data, void (*f) (T2, SPLAY_TREE_NODE<T, TV>*))
	{
		if (left) left->Iterate (data, f);

		f (data, this);

		if (right) right->Iterate (data, f);
	}

//#####################################################################
};
}
#endif
