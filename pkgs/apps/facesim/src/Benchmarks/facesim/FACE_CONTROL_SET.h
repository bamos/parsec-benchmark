//#####################################################################
// Copyright 2004-2005, Igor Neverov, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class FACE_CONTROL_SET
//#####################################################################
#ifndef __FACE_CONTROL_SET__
#define __FACE_CONTROL_SET__
namespace PhysBAM
{

template <class T>
class FACE_CONTROL_SET
{
public:

	FACE_CONTROL_SET()
	{}

	virtual ~FACE_CONTROL_SET()
	{}

	void Default() const
	{
		std::cout << "THIS FACE_CONTROL_SET FUNCTION IS NOT DEFINED!" << std::endl;
	}

//#####################################################################
	virtual int Size() const
	{
		Default();
		exit (1);
	}
	virtual T operator() (const int control_id) const
	{
		Default();
		exit (1);
	}
	virtual T& operator() (const int control_id)
	{
		Default();
		exit (1);
	}
	virtual bool Control_Active (const int control_id) const
	{
		Default();
		exit (1);
	}
	virtual bool& Control_Active (const int control_id)
	{
		Default();
		exit (1);
	}
	virtual bool Positions_Determined_Kinematically (const int control_id) const
	{
		Default();
		exit (1);
	}
	virtual void Force_Derivative (ARRAY<VECTOR_3D<T> >& dFdl, const int control_id) const
	{
		Default();
		exit (1);
	}
	virtual void Position_Derivative (ARRAY<VECTOR_3D<T> >& dXdl, const int control_id) const
	{
		Default();
		exit (1);
	}
	virtual void Set_Attachment_Positions (ARRAY<VECTOR_3D<T> >&X) const
	{
		Default();
		exit (1);
	}
	virtual void Save_Controls()
	{
		Default();
		exit (1);
	}
	virtual void Kinematically_Update_Positions (ARRAY<VECTOR_3D<T> >&X) const
	{
		Default();
		exit (1);
	}
	virtual void Kinematically_Update_Jacobian (ARRAY<VECTOR_3D<T> >&dX) const
	{
		Default();
		exit (1);
	}
	virtual T Penalty() const
	{
		Default();
		exit (1);
	}
	virtual T Penalty_Gradient (const int control_id) const
	{
		Default();
		exit (1);
	}
	virtual T Penalty_Hessian (const int control_id1, const int control_id2) const
	{
		Default();
		exit (1);
	}
	virtual void Print_Diagnostics (std::ostream& output = std::cout) const
	{
		Default();
		exit (1);
	}
	virtual void Project_Parameters_To_Allowable_Range (const bool active_controls_only = false)
	{
		Default();
		exit (1);
	}
	virtual void Interpolate (const T interpolation_fraction)
	{
		Default();
		exit (1);
	}
//#####################################################################
};
}
#endif
