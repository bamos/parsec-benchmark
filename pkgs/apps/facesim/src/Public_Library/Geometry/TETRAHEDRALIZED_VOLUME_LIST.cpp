//#########################################################################################################################################
// Copyright 2004, Zhaosheng Bao.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#########################################################################################################################################
#include "TETRAHEDRALIZED_VOLUME_LIST.h"
#include "TETRAHEDRALIZED_VOLUME.h"
using namespace PhysBAM;
template void TETRAHEDRALIZED_VOLUME_LIST<float>::Read<float> (const std::string& prefix, const int frame);
template void TETRAHEDRALIZED_VOLUME_LIST<double>::Read<double> (const std::string& prefix, const int frame);
template void TETRAHEDRALIZED_VOLUME_LIST<double>::Read<float> (const std::string& prefix, const int frame);
template void TETRAHEDRALIZED_VOLUME_LIST<float>::Write<float> (const std::string& prefix, const int frame) const;
template void TETRAHEDRALIZED_VOLUME_LIST<double>::Write<double> (const std::string& prefix, const int frame) const;
template void TETRAHEDRALIZED_VOLUME_LIST<double>::Write<float> (const std::string& prefix, const int frame) const;
//#####################################################################
// Function Read
//#####################################################################
template<class T> template<class RW> void TETRAHEDRALIZED_VOLUME_LIST<T>::
Read (const std::string& prefix, const int frame)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Write
//#####################################################################
template<class T> template <class RW> void TETRAHEDRALIZED_VOLUME_LIST<T>::
Write (const std::string& prefix, const int frame) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Destroy_Element
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME_LIST<T>::
Destroy_Element (TETRAHEDRALIZED_VOLUME<T>*& tetrahedralized_volume, const int id)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class TETRAHEDRALIZED_VOLUME_LIST<float>;
template class TETRAHEDRALIZED_VOLUME_LIST<double>;
