//#####################################################################
// Copyright 2004, Geoffrey Irving, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SPLAY_TREE
//#####################################################################
#include "SPLAY_TREE.h"
#include "../Utilities/DEBUG_UTILITIES.h"

using namespace PhysBAM;
template<class T, class TV> const T* SPLAY_TREE<T, TV>::goal_key;
//#####################################################################
// Function Splay
//#####################################################################
// Top-down splay routine.  Reference: "Self-adjusting Binary Search Trees", Sleator and Tarjan, JACM Volume 32, No 3, July 1985, pp 652-686. See page 668 for specific splay transformations.
template<class T, class TV> int SPLAY_TREE<T, TV>::
Splay (int (*cut) (const T&), SPLAY_TREE_NODE<T, TV>*& tree) const
{
	if (!tree) return -1;

	SPLAY_TREE_NODE<T, TV> *left = 0, *right = 0;
	SPLAY_TREE_NODE<T, TV> **left_right = &left, **right_left = &right;
	SPLAY_TREE_NODE<T, TV> *x = tree;
	int cut_x;

	for (;;)
	{
		cut_x = cut (x->key);

		if (cut_x == 0) break;                                                                      // success
		else if (cut_x < 0)
		{
			SPLAY_TREE_NODE<T, TV> *y = x->left;

			if (!y) break;                                         // trivial

			int cut_y = cut (y->key);

			if (cut_y == 0)
			{
				*right_left = x;        // zig
				right_left = &x->left;
				x = y;
				cut_x = cut_y;
				break;
			}
			else if (cut_y < 0)
			{
				SPLAY_TREE_NODE<T, TV> *z = y->left;

				if (!z)
				{
					*right_left = x;        // zig
					right_left = &x->left;
					x = y;
					cut_x = cut_y;
					break;
				}
				else
				{
					x->left = y->right;        // zig-zig
					y->right = x;
					*right_left = y;
					right_left = &y->left;
					x = z;
				}
			}
			else
			{
				SPLAY_TREE_NODE<T, TV> *z = y->right;

				if (!z)
				{
					*right_left = x;        // zig
					right_left = &x->left;
					x = y;
					cut_x = cut_y;
					break;
				}
				else
				{
					*left_right = y;        // zig-zag
					left_right = &y->right;
					*right_left = x;
					right_left = &x->left;
					x = z;
				}
			}
		}
		else
		{
			SPLAY_TREE_NODE<T, TV> *y = x->right;

			if (!y) break;                                        // trivial

			int cut_y = cut (y->key);

			if (cut_y == 0)
			{
				*left_right = x;        // zig
				left_right = &x->right;
				x = y;
				cut_x = cut_y;
				break;
			}
			else if (cut_y > 0)
			{
				SPLAY_TREE_NODE<T, TV> *z = y->right;

				if (!z)
				{
					*left_right = x;        // zig
					left_right = &x->right;
					x = y;
					cut_x = cut_y;
					break;
				}
				else
				{
					x->right = y->left;        // zig-zig
					y->left = x;
					*left_right = y;
					left_right = &y->right;
					x = z;
				}
			}
			else
			{
				SPLAY_TREE_NODE<T, TV> *z = y->left;

				if (!z)
				{
					*left_right = x;        // zig
					left_right = &x->right;
					x = y;
					cut_x = cut_y;
					break;
				}
				else
				{
					*right_left = y;        // zig-zag
					right_left = &y->left;
					*left_right = x;
					left_right = &x->right;
					x = z;
				}
			}
		}
	}

	*left_right = x->left;
	x->left = left;
	*right_left = x->right;
	x->right = right;
	tree = x;
	return cut_x;
}
//#####################################################################
// Function Splay_Min
//#####################################################################
// Equivalent to Splay with k -1
template<class T, class TV> void SPLAY_TREE<T, TV>::
Splay_Min (SPLAY_TREE_NODE<T, TV>*& tree) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Splay_Max
//#####################################################################
// Equivalent to Splay with k 1
template<class T, class TV> void SPLAY_TREE<T, TV>::
Splay_Max (SPLAY_TREE_NODE<T, TV>*& tree) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class SPLAY_TREE<std::string, int>;
template class SPLAY_TREE<std::string, float>;
template class SPLAY_TREE<std::string, double>;
template class SPLAY_TREE<std::string, std::string>;
