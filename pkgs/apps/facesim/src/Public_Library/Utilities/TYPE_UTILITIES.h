//#####################################################################
// Copyright 2006, Geoffrey Irving, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TYPE_UTILITIES
//#####################################################################
#ifndef __TYPE_UTILITIES__
#define __TYPE_UTILITIES__

namespace PhysBAM
{

template<bool b, class T = void> struct ENABLE_IF {};
template<class T> struct ENABLE_IF<true, T>
{
	typedef T TYPE;
};

template<bool b, class T1, class T2> struct IF
{
	typedef T1 TYPE;
};
template<class T1, class T2> struct IF<false, T1, T2>
{
	typedef T2 TYPE;
};

template<class T> struct IS_PRIMITIVE
{
	static const bool value = false;
};
template<> struct IS_PRIMITIVE<char>
{
	static const bool value = true;
};
template<> struct IS_PRIMITIVE<int>
{
	static const bool value = true;
};
template<> struct IS_PRIMITIVE<short>
{
	static const bool value = true;
};
template<> struct IS_PRIMITIVE<long>
{
	static const bool value = true;
};
template<> struct IS_PRIMITIVE<unsigned char>
{
	static const bool value = true;
};
template<> struct IS_PRIMITIVE<unsigned int>
{
	static const bool value = true;
};
template<> struct IS_PRIMITIVE<unsigned short>
{
	static const bool value = true;
};
template<> struct IS_PRIMITIVE<unsigned long>
{
	static const bool value = true;
};
template<> struct IS_PRIMITIVE<bool>
{
	static const bool value = true;
};
template<> struct IS_PRIMITIVE<float>
{
	static const bool value = true;
};
template<> struct IS_PRIMITIVE<double>
{
	static const bool value = true;
};

template<class T1, class T2> struct IS_SAME
{
	static const bool value = false;
};
template<class T> struct IS_SAME<T, T>
{
	static const bool value = true;
};

template<class T> struct IS_CONST
{
	static const bool value = false;
};
template<class T> struct IS_CONST<const T>
{
	static const bool value = true;
};

}
#endif
