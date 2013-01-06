//#####################################################################
// Copyright 2002-2004, Ronald Fedkiw, Geoffrey Irving, Sergey Koltakov, Neil Molino, Robert Bridson.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SEGMENT_MESH
//#####################################################################
#ifndef __SEGMENT_MESH__
#define __SEGMENT_MESH__

#include "../Arrays/LIST_ARRAYS.h"
namespace PhysBAM
{

template<class T> class LIST_ARRAY;

class SEGMENT_MESH
{
public:
	int number_nodes; // number of nodes in the mesh
	LIST_ARRAYS<int> segments;    // array of 2 indices for each segment - segment(j,i) is node j in segment i
	LIST_ARRAY<LIST_ARRAY<int> >* neighbor_nodes;       // neighbor_nodes(i) points to an array of integer indices of the neighbors of node i
	LIST_ARRAY<LIST_ARRAY<int> >* incident_segments;  // for each node, list of segments that contain it
	LIST_ARRAY<LIST_ARRAYS<int> >* ordered_segments;   // a list of connected segment components

	SEGMENT_MESH()
		: number_nodes (0), segments (2, 0), neighbor_nodes (0), incident_segments (0), ordered_segments (0)
	{}

	SEGMENT_MESH (const int m) // construct a regular m node linear mesh
		: number_nodes (0), neighbor_nodes (0), incident_segments (0), ordered_segments (0)
	{
		Initialize_Straight_Mesh (m);
	}

	SEGMENT_MESH (const LIST_ARRAYS<int>& segment_list)
		: neighbor_nodes (0), incident_segments (0), ordered_segments (0)
	{
		Initialize_Segment_Mesh (segment_list);
	}

	SEGMENT_MESH (const SEGMENT_MESH& segment_mesh)
		: segments (segment_mesh.segments), neighbor_nodes (0), incident_segments (0), ordered_segments (0)
	{
		Initialize_Segment_Mesh (segment_mesh);
	}

	~SEGMENT_MESH();

	void Initialize_Segment_Mesh (const LIST_ARRAYS<int>& segment_list) // construct a mesh given a list of segments
	{
		Clean_Up_Memory();
		segments.Exact_Resize_Array (2, segment_list.m);
		LIST_ARRAYS<int>::copy (segment_list, segments);

		for (int t = 1; t <= segments.m; t++) number_nodes = max (number_nodes, segments (1, t), segments (2, t));
	}

	void Initialize_Segment_Mesh (const SEGMENT_MESH& segment_mesh) // works with the copy constructor
	{
		Initialize_Segment_Mesh (segment_mesh.segments);
	}

	int Get_Opposite_Endpoint (const int segment, const int node)
	{
		assert (Node_In_Segment (node, segment));

		if (segments (1, segment) == node) return segments (2, segment);
		else return segments (1, segment);
	}

	void Refresh_Auxiliary_Structures()
	{
		if (neighbor_nodes) Initialize_Neighbor_Nodes();

		if (incident_segments) Initialize_Incident_Segments();

		if (ordered_segments) Initialize_Ordered_Segments();
	}

	bool Node_In_Segment (const int node, const int segment) const
	{
		return segments (1, segment) == node || segments (2, segment) == node;
	}

	bool Segments_Adjacent (const int segment1, const int segment2) const
	{
		int i, j;
		segments.Get (segment1, i, j);
		return Node_In_Segment (i, segment2) || Node_In_Segment (j, segment2);
	}

	void Replace_Node_In_Segment (const int segment, const int old_node, const int new_node)
	{
		assert (Node_In_Segment (old_node, segment));

		for (int i = 1; i <= 2; i++) if (segments (i, segment) == old_node)
			{
				segments (i, segment) = new_node;
				return;
			}
	}

	template <class RW>
	void Read (std::istream &input_stream)
	{
		Clean_Up_Memory();
		Read_Binary<RW> (input_stream, number_nodes);
		segments.template Read<RW> (input_stream);
	}

	template <class RW>
	void Write (std::ostream &output_stream) const
	{
		Write_Binary<RW> (output_stream, number_nodes);
		segments.template Write<RW> (output_stream);
	}

//#####################################################################
	void Clean_Up_Memory();
	int Segment (const int node1, const int node2) const;
	void Initialize_Neighbor_Nodes();
	void Initialize_Incident_Segments();
	void Initialize_Ordered_Segments();
	void Initialize_Straight_Mesh (const int number_of_points, bool loop = false);
//#####################################################################
};
}
#endif
