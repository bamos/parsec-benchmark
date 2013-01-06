//#####################################################################
// Copyright 2002-2005, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Eilene Hao, Geoffrey Irving, Neil Molino, Duc Nguyen, Andrew Selle, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class PARTICLE
//#####################################################################
#include "PARTICLE.h"
#include "PARTICLE_ATTRIBUTE_COLLECTION.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function ~PARTICE
//#####################################################################
PARTICLE::
~PARTICLE()
{
	delete deletion_list;
	delete attribute_collection;
	delete particle_ranges;
}
//#####################################################################
// Function Set_Array_Buffer_Size
//#####################################################################
void PARTICLE::
Set_Array_Buffer_Size (const int array_buffer_size_input)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Particles
//#####################################################################
void PARTICLE::
Initialize_Particles (const PARTICLE& particles)
{
	Clean_Up_Memory();
	Copy_Particle_State (particles);
	Increase_Array_Size (particles.number); // speeds this up by allocating all the needed memory at once

	for (int k = 1; k <= particles.number; k++) Copy_Particle (particles, k, Add_Particle());
}
//#####################################################################
// Function Initialize_Attributes_Collection
//#####################################################################
void PARTICLE::
Initialize_Attributes_Collection()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Increase_Array_Size
//#####################################################################
void PARTICLE::
Increase_Array_Size (const int number_of_new_indices)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Clean_Up_Memory
//#####################################################################
void PARTICLE::
Clean_Up_Memory()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Copy_Particle
//#####################################################################
void PARTICLE::
Copy_Particle (const int from, const int to)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Copy_Particle
//#####################################################################
void PARTICLE::
Copy_Particle (const PARTICLE& from_particles, const int from, const int to)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Particle
//#####################################################################
int PARTICLE::
Add_Particle()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Particles
//#####################################################################
void PARTICLE::
Add_Particles (const int new_particles)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Delete_Particle
//#####################################################################
void PARTICLE::
Delete_Particle (const int index)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Delete_All_Particles
//#####################################################################
void PARTICLE::
Delete_All_Particles()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Deletion_List
//#####################################################################
void PARTICLE::
Add_To_Deletion_List (const int index)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Delete_Particles_On_Deletion_List
//#####################################################################
void PARTICLE::
Delete_Particles_On_Deletion_List (const bool preserve_order, const bool already_sorted, const bool use_heap_sort)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Copy_Particle_State
//#####################################################################
void PARTICLE::
Copy_Particle_State (const PARTICLE& from_particles)
{}
//#####################################################################
// Function Optimize_Storage
//#####################################################################
void PARTICLE::
Optimize_Storage()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Print
//#####################################################################
void PARTICLE::
Print (std::ostream &output_stream, const int particle_index) const
{}
//#####################################################################
// Function Default
//#####################################################################
void PARTICLE::
Default()
{
	std::cout << "THIS PARTICLE FUNCTION IS NOT DEFINED!" << std::endl;
}
//#####################################################################

