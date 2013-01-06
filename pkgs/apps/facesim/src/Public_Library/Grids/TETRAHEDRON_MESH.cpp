//#####################################################################
// Copyright 2002-2004, Zhaosheng Bao, Robert Bridson, Ron Fedkiw, Geoffrey Irving, Sergey Koltakov, Neil Molino, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TETRAHEDRON_MESH
//#####################################################################
#include "TETRAHEDRON_MESH.h"
#include "SEGMENT_MESH.h"
#include "TRIANGLE_MESH.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Destructor
//#####################################################################
TETRAHEDRON_MESH::
~TETRAHEDRON_MESH()
{
	delete neighbor_nodes;
	delete incident_tetrahedrons;
	delete adjacent_tetrahedrons;
	delete neighbor_tetrahedrons;
	delete segment_mesh;
	delete triangle_mesh;
	delete tetrahedron_edges;
	delete tetrahedron_faces;
	delete boundary_mesh;
	delete node_on_boundary;
	delete boundary_nodes;
	delete edge_tetrahedrons;
	delete triangle_tetrahedrons;
}
//#####################################################################
// Function Delete_Auxiliary_Structures
//#####################################################################
void TETRAHEDRON_MESH::
Delete_Auxiliary_Structures()
{
	delete neighbor_nodes;
	neighbor_nodes = 0;
	delete incident_tetrahedrons;
	incident_tetrahedrons = 0;
	delete adjacent_tetrahedrons;
	adjacent_tetrahedrons = 0;
	delete neighbor_tetrahedrons;
	neighbor_tetrahedrons = 0;
	delete segment_mesh;
	segment_mesh = 0;
	delete triangle_mesh;
	triangle_mesh = 0;
	delete tetrahedron_edges;
	tetrahedron_edges = 0;
	delete tetrahedron_faces;
	tetrahedron_faces = 0;
	delete boundary_mesh;
	boundary_mesh = 0;
	delete node_on_boundary;
	node_on_boundary = 0;
	delete boundary_nodes;
	boundary_nodes = 0;
	delete edge_tetrahedrons;
	edge_tetrahedrons = 0;
	delete triangle_tetrahedrons;
	triangle_tetrahedrons = 0;
}
//#####################################################################
// Function Tetrahedron
//#####################################################################
// faster if incident_tetrahedrons is defined
// returns the tetrahedron containing these nodes - returns 0 if no tetrahedron contains these nodes
int TETRAHEDRON_MESH::
Tetrahedron (const int node1, const int node2, const int node3, const int node4) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Neighbor_Nodes
//#####################################################################
void TETRAHEDRON_MESH::
Initialize_Neighbor_Nodes()
{
	delete neighbor_nodes;
	neighbor_nodes = new LIST_ARRAY<LIST_ARRAY<int> > (number_nodes);

	for (int t = 1; t <= tetrahedrons.m; t++)
	{
		int i, j, k, l;
		tetrahedrons.Get (t, i, j, k, l);
		(*neighbor_nodes) (i).Append_Unique_Element (j);
		(*neighbor_nodes) (i).Append_Unique_Element (k);
		(*neighbor_nodes) (i).Append_Unique_Element (l);
		(*neighbor_nodes) (j).Append_Unique_Element (i);
		(*neighbor_nodes) (j).Append_Unique_Element (k);
		(*neighbor_nodes) (j).Append_Unique_Element (l);
		(*neighbor_nodes) (k).Append_Unique_Element (i);
		(*neighbor_nodes) (k).Append_Unique_Element (j);
		(*neighbor_nodes) (k).Append_Unique_Element (l);
		(*neighbor_nodes) (l).Append_Unique_Element (i);
		(*neighbor_nodes) (l).Append_Unique_Element (j);
		(*neighbor_nodes) (l).Append_Unique_Element (k);
	}

	for (int i = 1; i <= number_nodes; i++) (*neighbor_nodes) (i).Compact(); // remove extra space
}
//#####################################################################
// Function Initialize_Incident_Tetrahedrons
//#####################################################################
void TETRAHEDRON_MESH::
Initialize_Incident_Tetrahedrons()
{
	delete incident_tetrahedrons;
	incident_tetrahedrons = new LIST_ARRAY<LIST_ARRAY<int> > (number_nodes);

	for (int t = 1; t <= tetrahedrons.m; t++) // for each tetrahedron, put it on each of its nodes lists of tetrahedrons
	{
		int i, j, k, l;
		tetrahedrons.Get (t, i, j, k, l);
		(*incident_tetrahedrons) (i).Append_Element (t);
		(*incident_tetrahedrons) (j).Append_Element (t);
		(*incident_tetrahedrons) (k).Append_Element (t);
		(*incident_tetrahedrons) (l).Append_Element (t);
	}

	for (int i = 1; i <= number_nodes; i++) (*incident_tetrahedrons) (i).Compact(); // remove extra space
}
//#####################################################################
// Function Initialize_Adjacent_Tetrahedrons
//#####################################################################
void TETRAHEDRON_MESH::
Initialize_Adjacent_Tetrahedrons()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Find_And_Append_Adjacent_Tetrahedrons
//#####################################################################
// check for an adjacent tetrahedron that contains node1, node2, and node3 - append to the adjaceny list
void TETRAHEDRON_MESH::
Find_And_Append_Adjacent_Tetrahedrons (const int tetrahedron, const int node1, const int node2, const int node3)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Neighbor_Tetrahedrons
//#####################################################################
void TETRAHEDRON_MESH::
Initialize_Neighbor_Tetrahedrons()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Segment_Mesh
//#####################################################################
void TETRAHEDRON_MESH::
Initialize_Segment_Mesh()
{
	delete segment_mesh;
	bool neighbor_nodes_defined = neighbor_nodes != 0;

	if (!neighbor_nodes) Initialize_Neighbor_Nodes();

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
				segment_mesh->segments.Set (++edge, node, (*neighbor_nodes) (node) (k));

	if (!neighbor_nodes_defined)
	{
		delete neighbor_nodes;
		neighbor_nodes = 0;
	}
}
//#####################################################################
// Function Initialize_Triangle_Mesh
//#####################################################################
void TETRAHEDRON_MESH::
Initialize_Triangle_Mesh()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Tetrahedron_Edges
//#####################################################################
void TETRAHEDRON_MESH::
Initialize_Tetrahedron_Edges()
{
	delete tetrahedron_edges;
	tetrahedron_edges = new LIST_ARRAYS<int> (6, tetrahedrons.m);

	if (!segment_mesh) Initialize_Segment_Mesh(); // edges only makes sense when referring to a segment mesh

	// incident_segments make the SEGMENT_MESH::Segment() function faster
	bool incident_segments_defined = segment_mesh->incident_segments != 0;

	if (!segment_mesh->incident_segments) segment_mesh->Initialize_Incident_Segments();

	// for each tetrahedron, make the lists of edges
	for (int t = 1; t <= tetrahedrons.m; t++)
	{
		int i, j, k, l;
		tetrahedrons.Get (t, i, j, k, l);
		tetrahedron_edges->Set (t, segment_mesh->Segment (i, j), segment_mesh->Segment (j, k), segment_mesh->Segment (k, i),
					segment_mesh->Segment (i, l), segment_mesh->Segment (j, l), segment_mesh->Segment (k, l));
	}

	if (!incident_segments_defined)
	{
		delete segment_mesh->incident_segments;
		segment_mesh->incident_segments = 0;
	}
}
//#####################################################################
// Function Initialize_Tetrahedron_Faces
//#####################################################################
void TETRAHEDRON_MESH::
Initialize_Tetrahedron_Faces()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Number_Of_Tetrahedrons_Across_Face
//#####################################################################
int TETRAHEDRON_MESH::
Number_Of_Tetrahedrons_Across_Face (const int tetrahedron, const int node1, const int node2, const int node3) const
{
	int count = 0;

	for (int t = 1; t <= (*incident_tetrahedrons) (node1).m; t++)
	{
		int tetrahedron2 = (*incident_tetrahedrons) (node1) (t); // tetrahedron in question
		int i, j, k, l;
		tetrahedrons.Get (tetrahedron2, i, j, k, l);

		if (tetrahedron2 != tetrahedron)
		{
			if (i == node2 && (j == node3 || k == node3 || l == node3)) count++;
			else if (j == node2 && (i == node3 || k == node3 || l == node3)) count++;
			else if (k == node2 && (i == node3 || j == node3 || l == node3)) count++;
			else if (l == node2 && (i == node3 || j == node3 || k == node3)) count++;
		}
	}

	return count;
}
//#####################################################################
// Function Initialize_Boundary_Mesh
//#####################################################################
void TETRAHEDRON_MESH::
Initialize_Boundary_Mesh()
{
	delete boundary_mesh;
	boundary_mesh = new TRIANGLE_MESH();
	bool incident_tetrahedrons_defined = incident_tetrahedrons != 0;

	if (!incident_tetrahedrons) Initialize_Incident_Tetrahedrons();

	for (int t = 1; t <= tetrahedrons.m; t++)
	{
		int i, j, k, l;
		tetrahedrons.Get (t, i, j, k, l);

		if (Number_Of_Tetrahedrons_Across_Face (t, i, k, j) == 0) boundary_mesh->triangles.Append_Element (i, k, j);

		if (Number_Of_Tetrahedrons_Across_Face (t, i, j, l) == 0) boundary_mesh->triangles.Append_Element (i, j, l);

		if (Number_Of_Tetrahedrons_Across_Face (t, i, l, k) == 0) boundary_mesh->triangles.Append_Element (i, l, k);

		if (Number_Of_Tetrahedrons_Across_Face (t, j, k, l) == 0) boundary_mesh->triangles.Append_Element (j, k, l);
	}

	if (!incident_tetrahedrons_defined)
	{
		delete incident_tetrahedrons;
		incident_tetrahedrons = 0;
	}

	boundary_mesh->number_nodes = number_nodes; // use the same number of nodes as in the tetrahedron mesh
}
//#####################################################################
// Function Initialize_Node_On_Boundary
//#####################################################################
void TETRAHEDRON_MESH::
Initialize_Node_On_Boundary()
{
	delete node_on_boundary;
	node_on_boundary = new LIST_ARRAY<bool> (number_nodes);
	bool boundary_mesh_defined = boundary_mesh != 0;

	if (!boundary_mesh_defined) Initialize_Boundary_Mesh();

	for (int t = 1; t <= boundary_mesh->triangles.m; t++)
	{
		int i, j, k;
		boundary_mesh->triangles.Get (t, i, j, k);
		(*node_on_boundary) (i) = (*node_on_boundary) (j) = (*node_on_boundary) (k) = true;
	}

	if (!boundary_mesh_defined)
	{
		delete boundary_mesh;
		boundary_mesh = 0;
	}
}
//#####################################################################
// Function Initialize_Boundary_Nodes
//#####################################################################
void TETRAHEDRON_MESH::
Initialize_Boundary_Nodes()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Edge_Tetrahedrons
//#####################################################################
void TETRAHEDRON_MESH::
Initialize_Edge_Tetrahedrons()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Triangle_Tetrahedrons
//#####################################################################
void TETRAHEDRON_MESH::
Initialize_Triangle_Tetrahedrons()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Delete_Tetrahedrons_With_Missing_Nodes
//#####################################################################
int TETRAHEDRON_MESH::
Delete_Tetrahedrons_With_Missing_Nodes()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Delete_Tetrahedrons
//#####################################################################
void TETRAHEDRON_MESH::
Delete_Tetrahedrons (const LIST_ARRAY<int>& deletion_list)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Number_Of_Boundary_Tetrahedrons
//#####################################################################
int TETRAHEDRON_MESH::
Number_Of_Boundary_Tetrahedrons()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Number_Of_Interior_Tetrahedrons
//#####################################################################
int TETRAHEDRON_MESH::
Number_Of_Interior_Tetrahedrons()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Number_Of_Nodes_With_Minimum_Valence
//#####################################################################
int TETRAHEDRON_MESH::
Number_Of_Nodes_With_Minimum_Valence()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Minimum_Valence
//#####################################################################
int TETRAHEDRON_MESH::
Minimum_Valence (int* index)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Edge_Neighbours
//#####################################################################
bool TETRAHEDRON_MESH::
Edge_Neighbors (const int tet1, const int tet2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Number_Of_Edge_Neighbors
//#####################################################################
int TETRAHEDRON_MESH::
Number_Of_Edge_Neighbors (const int segment) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Segment_Mesh_Of_Subset
//#####################################################################
void TETRAHEDRON_MESH::
Initialize_Segment_Mesh_Of_Subset (SEGMENT_MESH& segment_mesh_of_subset, const LIST_ARRAY<bool>& subset) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Boundary_Mesh_Of_Subset
//#####################################################################
void TETRAHEDRON_MESH::
Initialize_Boundary_Mesh_Of_Subset (TRIANGLE_MESH& boundary_mesh_of_subset, const LIST_ARRAY<bool>& subset)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Adjacent_Tetrahedrons_From_Incident_Tetrahedrons
//#####################################################################
void TETRAHEDRON_MESH::
Update_Adjacent_Tetrahedrons_From_Incident_Tetrahedrons (const int node)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Neighbor_Nodes_From_Incident_Tetrahedrons
//#####################################################################
void TETRAHEDRON_MESH::
Update_Neighbor_Nodes_From_Incident_Tetrahedrons (const int node)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Mark_Face_Connected_Component_Incident_On_A_Node
//#####################################################################
// implements a depth first search to find marked connected component
void TETRAHEDRON_MESH::
Mark_Face_Connected_Component_Incident_On_A_Node (const int node, const int tetrahedron_index_in_incident_tetrahedrons, ARRAY<bool>& marked) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Tetrahedrons_On_Edge
//#####################################################################
int TETRAHEDRON_MESH::
Tetrahedrons_On_Edge (const int node1, const int node2, LIST_ARRAY<int>* tetrahedrons_on_edge) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Tetrahedrons_Across_Face
//#####################################################################
int TETRAHEDRON_MESH::
Tetrahedrons_Across_Face (const int tetrahedron, const int node1, const int node2, const int node3, LIST_ARRAY<int>& tetrahedrons_across_face) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Identify_Connected_Components
//#####################################################################
// labels each face connected components with a unique id
void TETRAHEDRON_MESH::
Identify_Face_Connected_Components (ARRAY<int>& label)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Identify_Edge_Node_Connected_Components
//#####################################################################
// labels each node connected components with a unique id
void TETRAHEDRON_MESH::
Identify_Edge_Connected_Components (ARRAY<int>& label)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
