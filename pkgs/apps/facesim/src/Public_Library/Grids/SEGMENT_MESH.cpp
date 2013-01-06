//#####################################################################
// Copyright 2002, 2003, Ronald Fedkiw, Geoffrey Irving, Sergey Koltakov, Frank Losasso, Neil Molino, Robert Bridson.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SEGMENT_MESH
//#####################################################################
#include "SEGMENT_MESH.h"
#include "../Arrays/ARRAYS_2D.h"
#include "../Data_Structures/UNION_FIND.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Destructor
//#####################################################################
SEGMENT_MESH::
~SEGMENT_MESH()
{
	delete neighbor_nodes;
	delete incident_segments;
	delete ordered_segments;
}
//#####################################################################
// Function Clean_Up_Memory
//#####################################################################
void SEGMENT_MESH::
Clean_Up_Memory()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Segment
//#####################################################################
// function is much faster if incident_segments is defined
// returns the segment containing these nodes - returns 0 if no segment contains these nodes
int SEGMENT_MESH::
Segment (const int node1, const int node2) const
{
	if (node1 > number_nodes || node2 > number_nodes) return 0;

	if (incident_segments)
	{
		int short_list = node1, check = node2;

		if ( (*incident_segments) (node2).m < (*incident_segments) (short_list).m)
		{
			short_list = node2;
			check = node1;
		}

		for (int k = 1; k <= (*incident_segments) (short_list).m; k++)
		{
			int s = (*incident_segments) (short_list) (k);

			if (segments (1, s) == check || segments (2, s) == check) return s;
		}

		return 0;
	}
	else
	{
		for (int k = 1; k <= segments.m; k++)
		{
			if (segments (1, k) == node1 && segments (2, k) == node2) return k;
			else if (segments (1, k) == node2 && segments (2, k) == node1) return k;
		}

		return 0;
	}
}
//#####################################################################
// Function Initialize_Neighbor_Nodes
//#####################################################################
void SEGMENT_MESH::
Initialize_Neighbor_Nodes()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Incident_Segments
//#####################################################################
void SEGMENT_MESH::
Initialize_Incident_Segments()
{
	if (incident_segments) delete incident_segments;

	incident_segments = new LIST_ARRAY<LIST_ARRAY<int> > (number_nodes);

	for (int t = 1; t <= segments.m; t++)
	{
		int i, j;
		segments.Get (t, i, j);
		(*incident_segments) (i).Append_Unique_Element (t);
		(*incident_segments) (j).Append_Unique_Element (t);
	}

	for (int i = 1; i <= incident_segments->m; i++) (*incident_segments) (i).Compact(); // get rid of extra space
}
//#####################################################################
// Function Get_Connected_Segments
//#####################################################################
void SEGMENT_MESH::
Initialize_Ordered_Segments()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Straight_Mesh
//#####################################################################
void SEGMENT_MESH::
Initialize_Straight_Mesh (const int number_of_points, bool loop)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
