//#####################################################################
// Copyright 2006, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Header STATIC_ASSERT
//#####################################################################
// Simplified version of BOOST_STATIC_ASSERT (see http://www.boost.org/doc/html/boost_staticassert.html)
//#####################################################################
#ifndef __STATIC_ASSERT__
#define __STATIC_ASSERT__

namespace PhysBAM
{

template<bool b> struct STATIC_ASSERTION_FAILURE;
template<> struct STATIC_ASSERTION_FAILURE<true>
{
	typedef void TYPE;
};

#define STATIC_ASSERT(c) \
    typedef typename STATIC_ASSERTION_FAILURE<(c)>::TYPE static_assert##__LINE__

}
#endif
