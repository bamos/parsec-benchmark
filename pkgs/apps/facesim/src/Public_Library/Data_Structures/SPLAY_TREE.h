//#####################################################################
// Copyright 2004, Geoffrey Irving, Frank Losasso, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SPLAY_TREE
//#####################################################################
#ifndef __SPLAY_TREE__
#define __SPLAY_TREE__

#include <string>
#include <iostream>
#include <assert.h>
#include "SPLAY_TREE_NODE.h"
namespace PhysBAM
{

template<class T, class TV>
class SPLAY_TREE
{
public:
	int m;
	mutable SPLAY_TREE_NODE<T, TV>* root;
	static const T* goal_key;

	SPLAY_TREE()
		: m (0), root (0)
	{}

	~SPLAY_TREE()
	{
		delete root;
	}

	void Clean_Up_Memory()
	{
		delete root;
		root = 0;
		m = 0;
	}

	bool Find (const T& key) const
	{
		return !Splay_Equal (key);
	}

	bool Find (const T& key, TV& value) const
	{
		if (Splay_Equal (key)) return false;

		value = root->value;
		return true;
	}

	void Set (const T& key, const TV& value)
	{
		if (!root)
		{
			root = new SPLAY_TREE_NODE<T, TV> (0, 0, key, value);
			m++;
		}
		else
		{
			int cut = Splay_Equal (key);

			if (cut == 0) root->value = value;
			else if (cut < 0)
			{
				SPLAY_TREE_NODE<T, TV>* old = root;
				root = new SPLAY_TREE_NODE<T, TV> (old->left, old, key, value);
				old->left = 0;
				m++;
			}
			else
			{
				SPLAY_TREE_NODE<T, TV>* old = root;
				root = new SPLAY_TREE_NODE<T, TV> (old, old->right, key, value);
				old->right = 0;
				m++;
			}
		}
	}

	bool Remove (const T& key, TV& value)
	{
		if (Splay_Equal (key)) return false;

		value = root->value;
		Remove_Root();
		return true;
	}

	static void Iterator_Print (std::ostream* output_stream, SPLAY_TREE_NODE<T, TV>* node)
	{
		*output_stream << node->key << " (with value '" << node->value << "')\n";
	}

	template<class T2> void Iterate (T2 data, void (*f) (T2, SPLAY_TREE_NODE<T, TV>*))
	{
		if (root) root->Iterate (data, f);
	}

private:
	static int Cut_Equal (const T& key)
	{
		return *goal_key < key ? -1 : *goal_key == key ? 0 : 1;
	}

	int Splay_Equal (const T& key) const
	{
		goal_key = &key;
		return Splay (Cut_Equal, root);
	}

	void Remove_Root()
	{
		assert (root);
		SPLAY_TREE_NODE<T, TV> *left = root->left, *old = root;

		if (!left) root = old->right;
		else
		{
			Splay_Max (left);
			left->right = old->right;
			root = left;
		}

		old->left = old->right = 0;
		delete old;
		m--;
	}

//#####################################################################
	int Splay (int (*cut) (const T&), SPLAY_TREE_NODE<T, TV>*& tree) const;
	void Splay_Min (SPLAY_TREE_NODE<T, TV>*& tree) const;
	void Splay_Max (SPLAY_TREE_NODE<T, TV>*& tree) const;
//#####################################################################
};
template<> inline int SPLAY_TREE<std::string, int>::Cut_Equal (const std::string& key)
{
	return goal_key->compare (key);
}
template<> inline int SPLAY_TREE<std::string, std::string>::Cut_Equal (const std::string& key)
{
	return goal_key->compare (key);
}

template<class T, class TV>
std::ostream& operator<< (std::ostream& output_stream, SPLAY_TREE<T, TV>& tree)
{
	tree.Iterate (&output_stream, SPLAY_TREE<T, TV>::Iterator_Print);
	return output_stream;
}
}
#endif
