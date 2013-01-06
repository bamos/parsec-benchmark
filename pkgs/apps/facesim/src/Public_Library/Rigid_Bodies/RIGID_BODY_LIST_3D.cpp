//#####################################################################
// Copyright 2003-2004, Zhaosheng Bao, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Sergey Koltakov, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "RIGID_BODY_LIST_3D.h"
#include "../Geometry/LEVELSET_IMPLICIT_SURFACE.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
template int RIGID_BODY_LIST_3D<float>::Add_Rigid_Body<float> (const std::string& basename, float scaling_factor, const bool read_triangulated_surface, const bool read_implicit_surface, const bool read_tetrahedralized_volume, const bool read_rgd_file);
template int RIGID_BODY_LIST_3D<float>::Add_Rigid_Body<double> (const std::string& basename, float scaling_factor, const bool read_triangulated_surface, const bool read_implicit_surface, const bool read_tetrahedralized_volume, const bool read_rgd_file);
template int RIGID_BODY_LIST_3D<double>::Add_Rigid_Body<double> (const std::string& basename, double scaling_factor, const bool read_triangulated_surface, const bool read_implicit_surface, const bool read_tetrahedralized_volume, const bool read_rgd_file);
template int RIGID_BODY_LIST_3D<double>::Add_Rigid_Body<float> (const std::string& basename, double scaling_factor, const bool read_triangulated_surface, const bool read_implicit_surface, const bool read_tetrahedralized_volume, const bool read_rgd_file);
template void RIGID_BODY_LIST_3D<float>::Read<float> (const std::string& directory, const int frame, const bool read_triangulated_surface_list, const bool read_implicit_surface_list, const bool read_tetrahedralized_volume_list, LIST_ARRAY<int>* needs_init);
template void RIGID_BODY_LIST_3D<double>::Read<double> (const std::string& directory, const int frame, const bool read_triangulated_surface_list, const bool read_implicit_surface_list, const bool read_tetrahedralized_volume_list, LIST_ARRAY<int>* needs_init);
template void RIGID_BODY_LIST_3D<double>::Read<float> (const std::string& directory, const int frame, const bool read_triangulated_surface_list, const bool read_implicit_surface_list, const bool read_tetrahedralized_volume_list, LIST_ARRAY<int>* needs_init);
template void RIGID_BODY_LIST_3D<float>::Write<float> (const std::string& directory, const int frame, const bool write_triangulated_surface_list, const bool write_implicit_surface_list, const bool write_tetrahedralized_volume_list) const;
template void RIGID_BODY_LIST_3D<double>::Write<double> (const std::string& directory, const int frame, const bool write_triangulated_surface_list, const bool write_implicit_surface_list, const bool write_tetrahedralized_volume_list) const;
template void RIGID_BODY_LIST_3D<double>::Write<float> (const std::string& directory, const int frame, const bool write_triangulated_surface_list, const bool write_implicit_surface_list, const bool write_tetrahedralized_volume_list) const;
//#####################################################################
// Function Add_Rigid_Body
//#####################################################################
// segmented curve and implicit curve already added to their respective lists
template<class T> int RIGID_BODY_LIST_3D<T>::
Add_Rigid_Body (RIGID_BODY_3D<T>* const& rigid_body, const int triangulated_surface_id, const int implicit_surface_id, const int tetrahedralized_volume_id)
{
	int id = Add_Element (rigid_body);
	rigid_body->Set_Id_Number (id);
	assert (!triangulated_surface_id || triangulated_surface_list.Element (triangulated_surface_id));
	assert (!implicit_surface_id || implicit_surface_list.Element (implicit_surface_id));
	assert (!tetrahedralized_volume_id || tetrahedralized_volume_list.Element (tetrahedralized_volume_id));
	rigid_body_id_to_triangulated_surface_id.Append_Element (triangulated_surface_id);
	assert (id == rigid_body_id_to_triangulated_surface_id.m);
	rigid_body_id_to_implicit_surface_id.Append_Element (implicit_surface_id);
	assert (id == rigid_body_id_to_implicit_surface_id.m);
	rigid_body_id_to_tetrahedralized_volume_id.Append_Element (tetrahedralized_volume_id);
	assert (id == rigid_body_id_to_tetrahedralized_volume_id.m);
	return id;
}

//#####################################################################
// Function Add_Rigid_Body
//#####################################################################
template<class T> template<class RW> int RIGID_BODY_LIST_3D<T>::
Add_Rigid_Body (const std::string& basename, T scaling_factor, const bool read_triangulated_surface, const bool read_implicit_surface, const bool read_tetrahedralized_volume, const bool read_rgd_file)
{
	std::string filename, hashname;
	RIGID_BODY_3D<T>* rigid_body = new RIGID_BODY_3D<T>();

	// triangulated surface
	TRIANGULATED_SURFACE<T>* triangulated_surface = 0;
	int triangulated_surface_id = 0;

	if (read_triangulated_surface)
	{
		filename = basename + ".tri";

		if (!FILE_UTILITIES::File_Exists (filename)) std::cout << "Note: No tri file for " << basename.c_str() << std::endl;
		else
		{
			if (scaling_factor != 1) hashname = STRING_UTILITIES::string_sprintf ("%s@%.6f", filename.c_str(), scaling_factor); // mangle hash name if we're rescaling it
			else hashname = filename;

			if (triangulated_surface_hash.Find (hashname, triangulated_surface_id)) // already read in segmented curve
			{
				triangulated_surface = triangulated_surface_list.Element (triangulated_surface_id);
				assert (triangulated_surface);
			} // only works if the referenced geometry is still in memory
			else  // read in segmented curve for the first time
			{
				FILE_UTILITIES::Create_From_File<RW> (filename, triangulated_surface);
				triangulated_surface_id = triangulated_surface_list.Add_Element (triangulated_surface);

				if (scaling_factor != 1) triangulated_surface->Rescale (scaling_factor);

				triangulated_surface_hash.Set (hashname, triangulated_surface_id);
			}
		}
	}

	if (triangulated_surface) rigid_body->Initialize_Triangulated_Surface (*triangulated_surface);

	// implicit surface
	IMPLICIT_SURFACE<T>* implicit_surface = 0;
	int implicit_surface_id = 0;

	if (read_implicit_surface)
	{
		if (FILE_UTILITIES::File_Exists (filename = basename + ".phi"))
		{
			if (scaling_factor != 1) hashname = STRING_UTILITIES::string_sprintf ("%s@%.6f", filename.c_str(), scaling_factor); // mangle hash name if we're rescaling it
			else hashname = filename;

			if (implicit_surface_hash.Find (hashname, implicit_surface_id)) // already read in implicit curve
			{
				implicit_surface = implicit_surface_list.Element (implicit_surface_id);
				assert (implicit_surface);
			} // only works if the referenced geometry is still in memory
			else  // read in implicit curve for the first time
			{
				LEVELSET_IMPLICIT_SURFACE<T>* levelset_implicit_surface = 0;
				FILE_UTILITIES::template Create_From_File<RW> (filename, levelset_implicit_surface);
				implicit_surface = levelset_implicit_surface;
				implicit_surface_id = implicit_surface_list.Add_Element (implicit_surface);

				if (scaling_factor != 1) implicit_surface->Rescale (scaling_factor);

				implicit_surface_hash.Set (hashname, implicit_surface_id);
			}
		}
		else if (FILE_UTILITIES::File_Exists (filename = basename + ".oct"))
		{
			if (scaling_factor != 1) hashname = STRING_UTILITIES::string_sprintf ("%s@%.6f", filename.c_str(), scaling_factor); // mangle hash name if we're rescaling it
			else hashname = filename;

			if (implicit_surface_hash.Find (hashname, implicit_surface_id)) // already read in implicit curve
			{
				implicit_surface = implicit_surface_list.Element (implicit_surface_id);
				assert (implicit_surface);
			}
		} // only works if the referenced geometry is still in memory
		else std::cout << "Note: No phi or oct file for " << basename.c_str() << std::endl;
	}

	if (implicit_surface) rigid_body->Initialize_Implicit_Surface (*implicit_surface);

	// tetrahedralized_volume
	TETRAHEDRALIZED_VOLUME<T>* tetrahedralized_volume = 0;
	int tetrahedralized_volume_id = 0;

	if (read_tetrahedralized_volume)
	{
		filename = basename + ".tet";

		if (!FILE_UTILITIES::File_Exists (filename)) std::cout << "Note: No tet file for " << basename.c_str() << std::endl;
		else
		{
			if (scaling_factor != 1) hashname = STRING_UTILITIES::string_sprintf ("%s@%.6f", filename.c_str(), scaling_factor); // mangle hash name if we're rescaling it
			else hashname = filename;

			if (tetrahedralized_volume_hash.Find (hashname, tetrahedralized_volume_id)) // already read in triangulated area
			{
				tetrahedralized_volume = tetrahedralized_volume_list.Element (tetrahedralized_volume_id);
				assert (tetrahedralized_volume);
			} // only works if the referenced geometry is still in memory
			else  // read in triangulated area for the first time
			{
				FILE_UTILITIES::Create_From_File<RW> (filename, tetrahedralized_volume);
				tetrahedralized_volume_id = tetrahedralized_volume_list.Add_Element (tetrahedralized_volume);

				if (scaling_factor != 1) tetrahedralized_volume->Rescale (scaling_factor);

				tetrahedralized_volume_hash.Set (hashname, tetrahedralized_volume_id);
			}
		}
	}

	if (tetrahedralized_volume) rigid_body->Initialize_Tetrahedralized_Volume (*tetrahedralized_volume);

	// rigid body
	if (read_rgd_file)
	{
		std::istream* input = FILE_UTILITIES::Safe_Open_Input (basename + ".rgd", true, false);

		if (!input)
		{
			std::cout << "Note: No rgd file for " << basename << " (using default values)" << std::endl;
		}
		else
		{
			rigid_body->template Read<RW> (*input);
			delete input;
		}
	}

	if (scaling_factor != 1) rigid_body->Rescale (scaling_factor);

	rigid_body->Update_Angular_Velocity();

	return Add_Rigid_Body (rigid_body, triangulated_surface_id, implicit_surface_id, tetrahedralized_volume_id);
}

//#####################################################################
// Function Set_External_Forces_And_Velocities
//#####################################################################
template<class T> void RIGID_BODY_LIST_3D<T>::
Set_External_Forces_And_Velocities (EXTERNAL_FORCES_AND_VELOCITIES<T, VECTOR_3D<T> >& external_forces_and_velocities)
{
	for (int i = 1; i <= rigid_bodies.m; i++) rigid_bodies (i)->Set_External_Forces_And_Velocities (external_forces_and_velocities);
}
//#####################################################################
// Function Read
//#####################################################################
template<class T> template<class RW> void RIGID_BODY_LIST_3D<T>::
Read (const std::string& directory, const int frame, const bool read_triangulated_surface_list, const bool read_implicit_surface_list, const bool read_tetrahedralized_volume_list, LIST_ARRAY<int>* needs_init)
{
	std::string prefix = directory + "/rigid_body_";

	if (read_triangulated_surface_list) triangulated_surface_list.template Read<RW> (prefix + "triangulated_surface_", frame);

	if (read_implicit_surface_list) implicit_surface_list.template Read<RW> (prefix + "implicit_surface_", frame);

	if (read_tetrahedralized_volume_list) tetrahedralized_volume_list.template Read<RW> (prefix + "tetrahedralized_volume_", frame);

	LIST_ARRAY<int> needs_init_default;

	if (!needs_init) needs_init = &needs_init_default;

	DYNAMIC_LIST<RIGID_BODY_3D<T>*>::template Read<RW> (prefix, frame, *needs_init);

	if (needs_init->m) // don't need to re-read these things if we will not be initializing any newly-active bodies
	{
		char version;
		FILE_UTILITIES::Read_From_File<RW> (STRING_UTILITIES::string_sprintf ("%skey", prefix.c_str()), version, rigid_body_id_to_triangulated_surface_id, rigid_body_id_to_implicit_surface_id, rigid_body_id_to_tetrahedralized_volume_id);
		assert (version == 1);
		Read_Rigid_Body_Names (directory);
	}

	// TODO: fix this ridiculous hack
	rigid_body_id_to_triangulated_surface_id.Resize_Array (last_unique_id);
	rigid_body_id_to_implicit_surface_id.Resize_Array (last_unique_id);
	rigid_body_id_to_tetrahedralized_volume_id.Resize_Array (last_unique_id);

	// end ridiculous hack
	for (int i = 1; i <= needs_init->m; i++)
	{
		int id = (*needs_init) (i), index = id_to_index_map (id);
		assert (index);
		// initialize new rigid body with given id, and initialize geometry
		RIGID_BODY_3D<T>* rigid_body = new RIGID_BODY_3D<T>();
		rigid_body->Set_Id_Number (id);

		if (id <= rigid_body_names.m) rigid_body->Set_Name (rigid_body_names (id));

		int triangulated_surface_id = rigid_body_id_to_triangulated_surface_id (id), implicit_surface_id = rigid_body_id_to_implicit_surface_id (id), tetrahedralized_volume_id = rigid_body_id_to_tetrahedralized_volume_id (id);

		if (read_triangulated_surface_list && triangulated_surface_id)
		{
			assert (triangulated_surface_list.Element (triangulated_surface_id));
			rigid_body->Initialize_Triangulated_Surface (*triangulated_surface_list.Element (triangulated_surface_id));
		}

		if (read_implicit_surface_list && implicit_surface_id)
		{
			assert (implicit_surface_list.Element (implicit_surface_id));
			rigid_body->Initialize_Implicit_Surface (*implicit_surface_list.Element (implicit_surface_id));
		}

		if (read_tetrahedralized_volume_list && tetrahedralized_volume_id)
		{
			assert (tetrahedralized_volume_list.Element (tetrahedralized_volume_id));
			rigid_body->Initialize_Tetrahedralized_Volume (*tetrahedralized_volume_list.Element (tetrahedralized_volume_id));
		}

		array (index) = rigid_body;
	}

	// now that all active rigid bodies are initialized we can read in their updated values
	std::istream* input = FILE_UTILITIES::Safe_Open_Input (STRING_UTILITIES::string_sprintf ("%s/rigid_bodies.%d", directory.c_str(), frame));

	for (int i = 1; i <= rigid_bodies.m; i++) rigid_bodies (i)->template Read<RW> (*input);

	delete input;
}

//#####################################################################
// Function Write
//#####################################################################
template<class T> template<class RW> void RIGID_BODY_LIST_3D<T>::
Write (const std::string& directory, const int frame, const bool write_triangulated_surface_list, const bool write_implicit_surface_list, const bool write_tetrahedralized_volume_list) const
{
	std::string prefix = directory + "/rigid_body_";

	if (write_triangulated_surface_list) triangulated_surface_list.template Write<RW> (prefix + "triangulated_surface_", frame);

	if (write_implicit_surface_list) implicit_surface_list.template Write<RW> (prefix + "implicit_surface_", frame);

	if (write_tetrahedralized_volume_list) tetrahedralized_volume_list.template Write<RW> (prefix + "tetrahedralized_volume_", frame);

	DYNAMIC_LIST<RIGID_BODY_3D<T>*>::template Write<RW> (prefix, frame);
	// update names
	rigid_body_names.Resize_Array (last_unique_id);

	for (int i = 1; i <= needs_write.m; i++)
	{
		int id = needs_write (i), index = id_to_index_map (id);
		assert (index);
		rigid_body_names (id) = rigid_bodies (index)->name;
	}

	if (needs_write.m) // don't need to re-write these things if we do not have any newly-active bodies
	{
		Write_Rigid_Body_Names (directory);
		char version = 1;
		FILE_UTILITIES::Write_To_File<RW> (STRING_UTILITIES::string_sprintf ("%skey", prefix.c_str()), version, rigid_body_id_to_triangulated_surface_id, rigid_body_id_to_implicit_surface_id, rigid_body_id_to_tetrahedralized_volume_id);
	}

	// write rigid body values for all active rigid bodies
	std::ostream* output = FILE_UTILITIES::Safe_Open_Output (STRING_UTILITIES::string_sprintf ("%s/rigid_bodies.%d", directory.c_str(), frame));

	for (int i = 1; i <= rigid_bodies.m; i++) rigid_bodies (i)->template Write<RW> (*output);

	delete output;
	needs_write.Reset_Current_Size_To_Zero();
}
//#####################################################################
// Function Write_Rigid_Body_Names
//#####################################################################
template<class T> void RIGID_BODY_LIST_3D<T>::
Write_Rigid_Body_Names (const std::string& output_directory) const
{
	std::ostream* output = FILE_UTILITIES::Safe_Open_Output (output_directory + "/rigid_body_names", false);
	*output << rigid_body_names.m << std::endl;

	for (int i = 1; i <= rigid_body_names.m; i++) *output << rigid_body_names (i) << std::endl;

	delete output;
}
//#####################################################################
// Function Read_Rigid_Body_Names
//#####################################################################
template<class T> bool RIGID_BODY_LIST_3D<T>::
Read_Rigid_Body_Names (const std::string& output_directory)
{
	std::istream* input = FILE_UTILITIES::Safe_Open_Input (output_directory + "/rigid_body_names", false, false);

	if (!input)
	{
		std::cerr << "Did not find rigid body names." << std::endl;
		rigid_body_names.Clean_Up_Memory();
		return false;
	}

	int num;
	*input >> num;
	input->ignore (256, '\n');
	rigid_body_names.Resize_Array (num);

	for (int i = 1; i <= rigid_body_names.m; i++)
	{
		std::getline (*input, rigid_body_names (i));
	}

	delete input;
	return true;
}
//#####################################################################
// Function Destroy_Element
//#####################################################################
template<class T> void RIGID_BODY_LIST_3D<T>::
Destroy_Element (RIGID_BODY_3D<T>*& rigid_body, const int id)
{
	delete rigid_body;
	rigid_body = 0;
}
//#####################################################################
template class RIGID_BODY_LIST_3D<float>;
template class RIGID_BODY_LIST_3D<double>;
