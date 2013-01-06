//#####################################################################
// Copyright 2002-2005, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Frank Losasso, Neil Molino, Duc Nguyen, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "TRIANGLE_MESH.h"
#include "SEGMENT_MESH.h"
#include "../Arrays/ARRAYS_2D.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Destructor
//#####################################################################
TRIANGLE_MESH::
~TRIANGLE_MESH()
{
	delete neighbor_nodes;
	delete incident_triangles;
	delete adjacent_triangles;
	delete segment_mesh;
	delete triangle_edges;
	delete edge_triangles;
	delete boundary_mesh;
	delete node_on_boundary;
	delete boundary_nodes;
}
//#####################################################################
// Function Clean_Up_Memory
//#####################################################################
void TRIANGLE_MESH::
Clean_Up_Memory()
{
	number_nodes = 0;
	triangles.Exact_Resize_Array (3, 0);
	delete neighbor_nodes;
	neighbor_nodes = 0;
	delete incident_triangles;
	incident_triangles = 0;
	delete adjacent_triangles;
	adjacent_triangles = 0;
	delete segment_mesh;
	segment_mesh = 0;
	delete triangle_edges;
	triangle_edges = 0;
	delete edge_triangles;
	edge_triangles = 0;
	delete boundary_mesh;
	boundary_mesh = 0;
	delete node_on_boundary;
	node_on_boundary = 0;
	delete boundary_nodes;
	boundary_nodes = 0;
}
//#####################################################################
// Function Triangle
//#####################################################################
// function is much faster if incident_triangles is defined
// returns the triangle containing these nodes - returns 0 if no triangle contains these nodes
int TRIANGLE_MESH::
Triangle (const int node1, const int node2, const int node3) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Neighbor_Nodes
//#####################################################################
void TRIANGLE_MESH::
Initialize_Neighbor_Nodes()
{
	delete neighbor_nodes;
	neighbor_nodes = new LIST_ARRAY<LIST_ARRAY<int> > (number_nodes);
	topologically_sorted_neighbor_nodes = false; // this function will not enforce the topological sorting

	for (int t = 1; t <= triangles.m; t++)
	{
		int i, j, k;
		triangles.Get (t, i, j, k);
		(*neighbor_nodes) (i).Append_Unique_Element (j);
		(*neighbor_nodes) (i).Append_Unique_Element (k);
		(*neighbor_nodes) (j).Append_Unique_Element (i);
		(*neighbor_nodes) (j).Append_Unique_Element (k);
		(*neighbor_nodes) (k).Append_Unique_Element (i);
		(*neighbor_nodes) (k).Append_Unique_Element (j);
	}

	for (int i = 1; i <= number_nodes; i++) (*neighbor_nodes) (i).Compact(); // remove extra space
}
//#####################################################################
// Function Initialize_Topologically_Sorted_Neighbor_Nodes
//#####################################################################
// for each node, sort its list of neighbors in the same order that triangle corners are listed
// we assume triangles corners are listed in a topologically consistent fashion, e.g. all counterclockwise
void TRIANGLE_MESH::
Initialize_Topologically_Sorted_Neighbor_Nodes()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Incident_Triangles
//#####################################################################
void TRIANGLE_MESH::
Initialize_Incident_Triangles()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Topologically_Sorted_Incident_Triangles
//#####################################################################
// we assume triangles corners are listed in a topologically consistent fashion, e.g. all counterclockwise
void TRIANGLE_MESH::
Initialize_Topologically_Sorted_Incident_Triangles()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Adjacent_Triangles
//#####################################################################
void TRIANGLE_MESH::
Initialize_Adjacent_Triangles()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Find_And_Append_Adjacent_Triangles
//#####################################################################
// check for an adjacent triangle that contains both node1 and node2 - append to the adjacency list
void TRIANGLE_MESH::
Find_And_Append_Adjacent_Triangles (const int triangle, const int node1, const int node2)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Segment_Mesh
//#####################################################################
void TRIANGLE_MESH::
Initialize_Segment_Mesh()
{
	delete segment_mesh;
	bool neighbor_nodes_defined = neighbor_nodes != 0;

	if (!neighbor_nodes_defined) Initialize_Neighbor_Nodes();

	// calculate number of  edges -> half the sum of the degree
	int number_of_edges = 0;

	for (int i = 1; i <= number_nodes; i++) number_of_edges += (*neighbor_nodes) (i).m;

	number_of_edges /= 2;
	segment_mesh = new SEGMENT_MESH();
	segment_mesh->number_nodes = number_nodes;
	segment_mesh->segments.Exact_Resize_Array (2, number_of_edges);
	int edge = 0;

	for (int node = 1; node <= neighbor_nodes->m; node++) for (int k = 1; k <= (*neighbor_nodes) (node).m; k++)
			if (node < (*neighbor_nodes) (node) (k)) // do nodes in ascending order so that no edges are counted more than once
			{
				edge++;
				segment_mesh->segments (1, edge) = node;
				segment_mesh->segments (2, edge) = (*neighbor_nodes) (node) (k);
			}

	if (!neighbor_nodes_defined)
	{
		delete neighbor_nodes;
		neighbor_nodes = 0;
	}
}
//#####################################################################
// Function Initialize_Triangle_Edges
//#####################################################################
void TRIANGLE_MESH::
Initialize_Triangle_Edges()
{
	delete triangle_edges;
	triangle_edges = new LIST_ARRAYS<int> (3, triangles.m);

	if (!segment_mesh) Initialize_Segment_Mesh(); // edges only makes sense when referring to a segment mesh

	bool incident_segments_defined = segment_mesh->incident_segments != 0;

	if (!incident_segments_defined) segment_mesh->Initialize_Incident_Segments();

	for (int t = 1; t <= triangles.m; t++)
	{
		int i, j, k;
		triangles.Get (t, i, j, k);
		triangle_edges->Set (t, segment_mesh->Segment (i, j), segment_mesh->Segment (j, k), segment_mesh->Segment (k, i));
	}

	if (!incident_segments_defined)
	{
		delete segment_mesh->incident_segments;
		segment_mesh->incident_segments = 0;
	}
}
//#####################################################################
// Function Initialize_Triangles_On_Edges
//#####################################################################
void TRIANGLE_MESH::
Initialize_Edge_Triangles()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Boundary_Mesh
//#####################################################################
// The direction of each boundary segment matches that of the triangle it lives on
void TRIANGLE_MESH::
Initialize_Boundary_Mesh()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Node_On_Boundary
//#####################################################################
void TRIANGLE_MESH::
Initialize_Node_On_Boundary()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Boundary_Nodes
//#####################################################################
void TRIANGLE_MESH::
Initialize_Boundary_Nodes()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Delete_Triangles_With_Missing_Nodes
//#####################################################################
int TRIANGLE_MESH::
Delete_Triangles_With_Missing_Nodes()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Delete_Triangles
//#####################################################################
void TRIANGLE_MESH::
Delete_Triangles (const LIST_ARRAY<int>& deletion_list)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Non_Manifold_Nodes
//#####################################################################
void TRIANGLE_MESH::
Non_Manifold_Nodes (LIST_ARRAY<int>& node_list)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Minimum_Degree_Node
//#####################################################################
int TRIANGLE_MESH::
Minimum_Degree_Node (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Maximum_Degree_Node
//#####################################################################
int TRIANGLE_MESH::
Maximum_Degree_Node (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Adjacent_Triangles_From_Incident_Triangles
//#####################################################################
void TRIANGLE_MESH::
Update_Adjacent_Triangles_From_Incident_Triangles (const int node)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Neighbor_Nodes_From_Incident_Triangles
//#####################################################################
void TRIANGLE_MESH::
Update_Neighbor_Nodes_From_Incident_Triangles (const int node)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Mark_Face_Connected_Component_Incident_On_A_Node
//#####################################################################
// implements a depth first search to find marked connected component
void TRIANGLE_MESH::
Mark_Edge_Connected_Component_Incident_On_A_Node (const int node, const int triangle_index_in_incident_triangles, ARRAY<bool>& marked) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Adjacent_Triangle
//#####################################################################
int TRIANGLE_MESH::
Adjacent_Triangle (const int triangle, const int node1, const int node2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Triangles_On_Edge
//#####################################################################
int TRIANGLE_MESH::
Triangles_On_Edge (const int node1, const int node2, LIST_ARRAY<int>* triangles_on_edge) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Triangle_On_Edge
//#####################################################################
bool TRIANGLE_MESH::
Triangle_On_Edge (const int node1, const int node2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Triangles_Across_Edge
//#####################################################################
int TRIANGLE_MESH::
Triangles_Across_Edge (const int triangle, const int node1, const int node2, LIST_ARRAY<int>& triangles_across_edge) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Make_Orientations_Consistent
//#####################################################################
void TRIANGLE_MESH::
Make_Orientations_Consistent()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Make_Orientations_Consistent
//#####################################################################
bool TRIANGLE_MESH::
Orientations_Consistent()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Identify_Connected_Components
//#####################################################################
// Labels each connected components with a unique id
void TRIANGLE_MESH::
Identify_Connected_Components (ARRAY<int>& label)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Label_Connected_Component_With_ID
//#####################################################################
// Labels connected components to a given triangle with a unique id
void TRIANGLE_MESH::
Label_Connected_Component_With_ID (ARRAY<int>& label, const int triangle, const int id) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Herring_Bone_Mesh_Blocked
//#####################################################################
void TRIANGLE_MESH::
Initialize_Herring_Bone_Mesh_Blocked (const int m, const int n, const int blocks_m, const int blocks_n, ARRAYS_2D<int>& particle, ARRAY<VECTOR_2D<int> >* particle_ranges)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Herring_Bone_Mesh_Blocked_With_Equal_Extents
//#####################################################################
void TRIANGLE_MESH::
Initialize_Herring_Bone_Mesh_Blocked_With_Equal_Extents (const int m, const int n, const int blocks_m, const int blocks_n, ARRAYS_2D<int>& particle, ARRAY<VECTOR_2D<int> >* particle_ranges)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
