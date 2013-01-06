//#####################################################################
// Copyright 2002-2004, Christopher Allocco, Robert Bridson, Ronald Fedkiw, Geoffrey Irving, Eran Guendelman, Neil Molino, Duc Nguyen, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TRIANGLE_MESH
//#####################################################################
#ifndef __TRIANGLE_MESH__
#define __TRIANGLE_MESH__

#include "../Arrays/LIST_ARRAY.h"
#include "../Arrays/LIST_ARRAYS.h"
#include "../Matrices_And_Vectors/VECTOR_2D.h"
namespace PhysBAM
{

class SEGMENT_MESH;
template<class T> class ARRAYS_2D;
class TRIANGLE_MESH
{
public:
	int number_nodes; // number of nodes in the mesh
	LIST_ARRAYS<int> triangles; // array of 3 indices for each triangle - triangle(j,i) is node j in triangle i
	LIST_ARRAY<LIST_ARRAY<int> >* neighbor_nodes;     // neighbor_nodes(i) points to an array of integer indices of the neighbors of node i
	bool topologically_sorted_neighbor_nodes; // usually false, unless explicitly set with Initialize_Topologically_Sorted_Neighbor_Nodes()
	LIST_ARRAY<LIST_ARRAY<int> >* incident_triangles; // for each node, list of triangles that contain it
	bool topologically_sorted_incident_triangles; // usually false, unless explicitly set with Initialize_Topologically_Sorted_Incident_Triangles()
	LIST_ARRAY<LIST_ARRAY<int> >* adjacent_triangles; // for each triangle, list of (up to 3) adjacent triangles
	SEGMENT_MESH* segment_mesh; // segment mesh consisting of all the edges
	LIST_ARRAYS<int>* triangle_edges; // array of 3 indices for each triangle - edge triangle_edges(j,i) is edge j in triangle i
	LIST_ARRAY<LIST_ARRAY<int> >* edge_triangles; // for each edge, the indices of the incident triangles
	SEGMENT_MESH* boundary_mesh;
	LIST_ARRAY<bool>* node_on_boundary; // whether or not a node is on the boundary
	LIST_ARRAY<int>* boundary_nodes;

	TRIANGLE_MESH() // simplest constructor - null mesh
		: number_nodes (0), triangles (3, 0), neighbor_nodes (0), topologically_sorted_neighbor_nodes (false), incident_triangles (0), topologically_sorted_incident_triangles (false),
		  adjacent_triangles (0), segment_mesh (0), triangle_edges (0), edge_triangles (0), boundary_mesh (0), node_on_boundary (0), boundary_nodes (0)
	{}

	TRIANGLE_MESH (const int m, const int n, const bool herring_bone = false)
		: number_nodes (0), neighbor_nodes (0), topologically_sorted_neighbor_nodes (false), incident_triangles (0), topologically_sorted_incident_triangles (false),
		  adjacent_triangles (0), segment_mesh (0), triangle_edges (0), edge_triangles (0), boundary_mesh (0), node_on_boundary (0), boundary_nodes (0)
	{
		if (!herring_bone) Initialize_Square_Mesh (m, n);
		else Initialize_Herring_Bone_Mesh (m, n);
	}

	TRIANGLE_MESH (const LIST_ARRAYS<int>& triangle_list)
		: number_nodes (0), neighbor_nodes (0), topologically_sorted_neighbor_nodes (false), incident_triangles (0), topologically_sorted_incident_triangles (false),
		  adjacent_triangles (0), segment_mesh (0), triangle_edges (0), edge_triangles (0), boundary_mesh (0), node_on_boundary (0), boundary_nodes (0)
	{
		Initialize_Triangle_Mesh (triangle_list);
	}

	TRIANGLE_MESH (const TRIANGLE_MESH& triangle_mesh)
		: number_nodes (0), neighbor_nodes (0), topologically_sorted_neighbor_nodes (false), incident_triangles (0), topologically_sorted_incident_triangles (false),
		  adjacent_triangles (0), segment_mesh (0), triangle_edges (0), edge_triangles (0), boundary_mesh (0), node_on_boundary (0), boundary_nodes (0)
	{
		Initialize_Triangle_Mesh (triangle_mesh);
	}

	~TRIANGLE_MESH();

	void Refresh_Auxiliary_Structures()
	{
		if (neighbor_nodes)
		{
			if (topologically_sorted_neighbor_nodes) Initialize_Topologically_Sorted_Neighbor_Nodes();
			else Initialize_Neighbor_Nodes();
		}

		if (incident_triangles)
		{
			if (topologically_sorted_incident_triangles) Initialize_Topologically_Sorted_Incident_Triangles();
			else Initialize_Incident_Triangles();
		}

		if (adjacent_triangles) Initialize_Adjacent_Triangles();

		if (segment_mesh) Initialize_Segment_Mesh();

		if (triangle_edges) Initialize_Triangle_Edges();

		if (edge_triangles) Initialize_Edge_Triangles();

		if (boundary_mesh) Initialize_Boundary_Mesh();

		if (node_on_boundary) Initialize_Node_On_Boundary();

		if (boundary_nodes) Initialize_Boundary_Nodes();
	}

	void Initialize_Square_Mesh (const int m, const int n) // construct a regular m-by-n rectangular mesh
	{
		Clean_Up_Memory();
		number_nodes = m * n;
		triangles.Exact_Resize_Array (3, 2 * (m - 1) * (n - 1));
		int t = 0;

		for (int i = 1; i <= m - 1; i++) for (int j = 1; j <= n - 1; j++) // clockwise node ordering
			{
				triangles.Set (++t, i + m * (j - 1), i + 1 + m * j, i + 1 + m * (j - 1));
				triangles.Set (++t, i + m * (j - 1), i + m * j, i + 1 + m * j);
			}
	}

	void Initialize_Equilateral_Mesh (const int m, const int n)
	{
		Clean_Up_Memory();
		number_nodes = m * n;
		triangles.Exact_Resize_Array (3, 2 * (m - 1) * (n - 1));
		int t = 0;

		for (int i = 1; i <= m - 1; i++) for (int j = 1; j <= n - 1; j++) // clockwise node ordering
			{
				if (j % 2)
				{
					triangles.Set (++t, i + m * (j - 1), i + m * j, i + 1 + m * (j - 1));
					triangles.Set (++t, i + 1 + m * (j - 1), i + m * j, i + 1 + m * j);
				}
				else
				{
					triangles.Set (++t, i + m * (j - 1), i + 1 + m * j, i + 1 + m * (j - 1));
					triangles.Set (++t, i + m * (j - 1), i + m * j, i + 1 + m * j);
				}
			}
	}

	void Initialize_Torus_Mesh (const int m, const int n)
	{
		Clean_Up_Memory();
		number_nodes = m * n;
		triangles.Exact_Resize_Array (3, 2 * m * n);
		assert ( (n & 1) == 0);
		int t = 0;

		for (int i = 1; i <= m; i++) for (int j = 1; j <= n; j++) // clockwise node ordering
			{
				int i1 = i == m ? 1 : i + 1, j1 = j == n ? 1 : j + 1;

				if (j & 1)
				{
					triangles.Set (++t, i + m * (j - 1), i + m * (j1 - 1), i1 + m * (j - 1));
					triangles.Set (++t, i1 + m * (j - 1), i + m * (j1 - 1), i1 + m * (j1 - 1));
				}
				else
				{
					triangles.Set (++t, i + m * (j - 1), i1 + m * (j1 - 1), i1 + m * (j - 1));
					triangles.Set (++t, i + m * (j - 1), i + m * (j1 - 1), i1 + m * (j1 - 1));
				}
			}
	}

	void Initialize_Circle_Mesh (const int num_radial, const int num_tangential) // construct a circle
	{
		Clean_Up_Memory();
		number_nodes = num_radial * num_tangential;
		triangles.Exact_Resize_Array (3, 2 * (num_radial - 1) *num_tangential);
		int t = 0, n = num_tangential;

		for (int i = 0; i < num_radial - 1; i++) for (int j = 1; j <= n; j++) // clockwise node ordering
			{
				if (j & 1)
				{
					triangles.Set (++t, i * n + j, (i + 1) *n + j, i * n + (j % n) + 1);
					triangles.Set (++t, (i + 1) *n + j, (i + 1) *n + (j % n) + 1, i * n + (j % n) + 1);
				}
				else
				{
					triangles.Set (++t, i * n + j, (i + 1) *n + j % n + 1, i * n + (j % n) + 1);
					triangles.Set (++t, (i + 1) *n + j, (i + 1) *n + (j % n) + 1, i * n + j);
				}
			}
	}

	void Initialize_Herring_Bone_Mesh (const int m, const int n) // construct a regular m-by-n herring bone rectangular mesh
	{
		Clean_Up_Memory();
		number_nodes = m * n;
		triangles.Exact_Resize_Array (3, 2 * (m - 1) * (n - 1));
		int t = 0;

		for (int i = 1; i <= m - 1; i++) for (int j = 1; j <= n - 1; j++) // clockwise node ordering
			{
				if (i % 2)
				{
					triangles.Set (++t, i + m * (j - 1), i + m * j, i + 1 + m * (j - 1));
					triangles.Set (++t, i + 1 + m * (j - 1), i + m * j, i + 1 + m * j);
				}
				else
				{
					triangles.Set (++t, i + m * (j - 1), i + 1 + m * j, i + 1 + m * (j - 1));
					triangles.Set (++t, i + m * (j - 1), i + m * j, i + 1 + m * j);
				}
			}
	}

	// construct a regular m-by-n herring bone rectangular mesh, but order the indices to block it...
	void Initialize_Herring_Bone_Mesh_Blocked (const int m, const int n, const int blocks_m, const int blocks_n, ARRAYS_2D<int>& grid_to_particle, ARRAY<VECTOR_2D<int> >* particle_ranges);
	void Initialize_Herring_Bone_Mesh_Blocked_With_Equal_Extents (const int m, const int n, const int blocks_m, const int blocks_n, ARRAYS_2D<int>& grid_to_particle, ARRAY<VECTOR_2D<int> >* particle_ranges);

	void Initialize_Triangle_Mesh (const LIST_ARRAYS<int>& triangle_list) // construct a mesh given a list of triangles
	{
		Clean_Up_Memory();
		triangles.Exact_Resize_Array (3, triangle_list.m);
		LIST_ARRAYS<int>::copy (triangle_list, triangles);

		for (int t = 1; t <= triangles.m; t++)
		{
			int i, j, k;
			triangles.Get (t, i, j, k);
			number_nodes = max (number_nodes, i, j, k);
		}
	}

	void Initialize_Triangle_Mesh (const TRIANGLE_MESH& triangle_mesh) // works with the copy constructor
	{
		Initialize_Triangle_Mesh (triangle_mesh.triangles);
	}

	bool Node_In_Triangle (const int node, const int triangle_index) const
	{
		int i, j, k;
		triangles.Get (triangle_index, i, j, k);
		return i == node || j == node || k == node;
	}

	bool Segment_In_Triangle (const int segment_node_1, const int segment_node_2, const int triangle_index)
	{
		int node1, node2, node3;
		triangles.Get (triangle_index, node1, node2, node3);

		if (segment_node_1 == node1)
		{
			if (segment_node_2 == node2 || segment_node_2 == node3) return true;
			else return false;
		}
		else if (segment_node_1 == node2)
		{
			if (segment_node_2 == node1 || segment_node_2 == node3) return true;
			else return false;
		}
		else if (segment_node_1 == node3)
		{
			if (segment_node_2 == node1 || segment_node_2 == node2) return true;
			else return false;
		}
		else return false;
	}

	void Replace_Node_In_Triangle (const int triangle, const int old_node, const int new_node)
	{
		assert (Node_In_Triangle (old_node, triangle));

		for (int i = 1; i <= 3; i++) if (triangles (i, triangle) == old_node)
			{
				triangles (i, triangle) = new_node;
				return;
			}
	}

	bool Edge_Neighbors (const int triangle1, const int triangle2) const
	{
		int i, j, k;
		triangles.Get (triangle1, i, j, k);
		return Node_In_Triangle (i, triangle2) + Node_In_Triangle (j, triangle2) + Node_In_Triangle (k, triangle2) == 2;
	}

	int Other_Node (const int node1, const int node2, const int triangle) const
	{
		assert (Node_In_Triangle (node1, triangle) && Node_In_Triangle (node2, triangle));
		int i, j, k;
		triangles.Get (triangle, i, j, k);
		return i ^ j ^ k ^ node1 ^ node2;
	}

	void Other_Two_Nodes (const int node, const int triangle, int& other_node1, int& other_node2) const
	{
		assert (Node_In_Triangle (node, triangle));
		int i, j, k;
		triangles.Get (triangle, i, j, k);

		if (i == node)
		{
			other_node1 = j;
			other_node2 = k;
		}
		else if (j == node)
		{
			other_node1 = k;
			other_node2 = i;
		}
		else if (k == node)
		{
			other_node1 = i;
			other_node2 = j;
		}
	}

	static bool Equivalent_Triangles (const int tri1_a, const int tri1_b, const int tri1_c, const int tri2_a, const int tri2_b, const int tri2_c)
	{
		if (tri1_a == tri2_a && ( (tri1_b == tri2_b && tri1_c == tri2_c) || (tri1_b == tri2_c && tri1_c == tri2_b))) return true;

		if (tri1_a == tri2_b && ( (tri1_b == tri2_c && tri1_c == tri2_a) || (tri1_b == tri2_a && tri1_c == tri2_c))) return true;

		if (tri1_a == tri2_c && ( (tri1_b == tri2_a && tri1_c == tri2_b) || (tri1_b == tri2_b && tri1_c == tri2_a))) return true;

		return false;
	}

	static bool Equivalent_Oriented_Triangles (const int tri1_a, const int tri1_b, const int tri1_c, const int tri2_a, const int tri2_b, const int tri2_c)
	{
		return ( (tri1_a == tri2_a && tri1_b == tri2_b && tri1_c == tri2_c) || (tri1_a == tri2_b && tri1_b == tri2_c && tri1_c == tri2_a) || (tri1_a == tri2_c && tri1_b == tri2_a && tri1_c == tri2_b));
	}

	static void Add_Ordered_Neighbors (LIST_ARRAY<int>& nodes, LIST_ARRAY<int>& links, const int neighbor1, const int neighbor2)
	{
		int index1, index2;

		if (!nodes.Find (neighbor1, index1))
		{
			nodes.Append_Element (neighbor1);
			links.Append_Element (0);
			index1 = nodes.m;
		}

		if (!nodes.Find (neighbor2, index2))
		{
			nodes.Append_Element (neighbor2);
			links.Append_Element (0);
			index2 = nodes.m;
		}

		links (index1) = index2;
	}

	template <class RW>
	void Read (std::istream& input_stream)
	{
		Clean_Up_Memory();
		Read_Binary<RW> (input_stream, number_nodes);
		triangles.template Read<RW> (input_stream);
	}

	template <class RW>
	void Write (std::ostream& output_stream) const
	{
		Write_Binary<RW> (output_stream, number_nodes);
		triangles.template Write<RW> (output_stream);
	}

//#####################################################################
	void Clean_Up_Memory();
	int Triangle (const int node1, const int node2, const int node3) const;
	void Initialize_Neighbor_Nodes();
	void Initialize_Topologically_Sorted_Neighbor_Nodes();
	void Initialize_Incident_Triangles();
	void Initialize_Topologically_Sorted_Incident_Triangles();
	void Initialize_Adjacent_Triangles();
private: // helper function for Initialize_Adjacent_Triangles()
	void Find_And_Append_Adjacent_Triangles (const int triangle, const int node1, const int node2);
public:
	void Initialize_Segment_Mesh();
	void Initialize_Triangle_Edges();
	void Initialize_Edge_Triangles();
	void Initialize_Boundary_Mesh();
	void Initialize_Node_On_Boundary();
	void Initialize_Boundary_Nodes();
	int Delete_Triangles_With_Missing_Nodes(); // returns the number deleted
	void Delete_Triangles (const LIST_ARRAY<int>& deletion_list);
	void Non_Manifold_Nodes (LIST_ARRAY<int>& node_list);
	int Minimum_Degree_Node (int* index = 0) const;
	int Maximum_Degree_Node (int* index = 0) const;
	void Update_Adjacent_Triangles_From_Incident_Triangles (const int node);
	void Update_Neighbor_Nodes_From_Incident_Triangles (const int node);
	void Mark_Edge_Connected_Component_Incident_On_A_Node (const int node, const int triangle_index_in_incident_triangles, ARRAY<bool>& marked) const;
	int Adjacent_Triangle (const int triangle_index, const int node1, const int node2) const;
	int Triangles_On_Edge (const int node1, const int node2, LIST_ARRAY<int>* triangles_on_edge) const;
	bool Triangle_On_Edge (const int node1, const int node2) const;
	int Triangles_Across_Edge (const int triangle, const int node1, const int node2, LIST_ARRAY<int>& triangles_across_edge) const;
	void Make_Orientations_Consistent();
	bool Orientations_Consistent();
	void Identify_Connected_Components (ARRAY<int>& label);
	void Label_Connected_Component_With_ID (ARRAY<int>& label, const int triangle, const int id) const;
//#####################################################################
};
}
#endif
