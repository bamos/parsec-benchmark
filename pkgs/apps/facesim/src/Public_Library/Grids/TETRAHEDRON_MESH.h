//#####################################################################
// Copyright 2002-2004, Zhaosheng Bao, Robert Bridson, Ron Fedkiw, Geoffrey Irving, Sergey Koltakov, Neil Molino, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TETRAHEDRON_MESH
//#####################################################################
#ifndef __TETRAHEDRON_MESH__
#define __TETRAHEDRON_MESH__

#include "../Arrays/LIST_ARRAYS.h"
#include "../Matrices_And_Vectors/VECTOR_2D.h"
namespace PhysBAM
{

class SEGMENT_MESH;
class TRIANGLE_MESH;
template<class T> class LIST_ARRAY;

class TETRAHEDRON_MESH
{
public:
	int number_nodes; // number of nodes in the mesh
	LIST_ARRAYS<int> tetrahedrons; // array of 4 indices for each tetrahedron - tetrahedron(j,i) is j'th index in tetrahedron i
	LIST_ARRAY<LIST_ARRAY<int> >* neighbor_nodes; // for each node, list of neighboring nodes
	LIST_ARRAY<LIST_ARRAY<int> >* incident_tetrahedrons; // for each node, list of neighboring tetrahedrons that contain it
	LIST_ARRAY<LIST_ARRAY<int> >* adjacent_tetrahedrons; // for each tetrahedron, list of (up to 4) adjacent neighboring tetrahedrons
	LIST_ARRAY<LIST_ARRAY<int> >* neighbor_tetrahedrons; // for each tetrahedron, list of tetrahedrons sharing at least one of its node
	SEGMENT_MESH* segment_mesh; // segment mesh consisting of all the edges
	TRIANGLE_MESH* triangle_mesh; // triangle mesh consisting of all the triangles
	LIST_ARRAYS<int>* tetrahedron_edges; // array of 6 indices for each tetrahedron - edges(j,i) is edge j in tetrahedron i
	LIST_ARRAYS<int>* tetrahedron_faces; // array of 4 indices for each tetrahedron - faces(j,i) is the face opposite vertex j in tetrahedron i
	TRIANGLE_MESH* boundary_mesh;
	LIST_ARRAY<bool>* node_on_boundary; // node_on_boundary(i) is true if node i is on the boundary
	LIST_ARRAY<int>* boundary_nodes; // array containing indices of all particles on the boundary
	LIST_ARRAY<LIST_ARRAY<int> >* edge_tetrahedrons; // for each edge, list of tets containing that edge
	LIST_ARRAYS<int>* triangle_tetrahedrons; // for each face, list of incident tets

	TETRAHEDRON_MESH() // simplest constructor - null mesh
		: number_nodes (0), tetrahedrons (4, 0), neighbor_nodes (0), incident_tetrahedrons (0), adjacent_tetrahedrons (0), neighbor_tetrahedrons (0), segment_mesh (0), triangle_mesh (0),
		  tetrahedron_edges (0), tetrahedron_faces (0), boundary_mesh (0), node_on_boundary (0), boundary_nodes (0), edge_tetrahedrons (0), triangle_tetrahedrons (0)
	{}

	TETRAHEDRON_MESH (const int m, const int n, const int p, bool octahedron = true)
		: number_nodes (0), neighbor_nodes (0), incident_tetrahedrons (0), adjacent_tetrahedrons (0), neighbor_tetrahedrons (0), segment_mesh (0), triangle_mesh (0), tetrahedron_edges (0),
		  tetrahedron_faces (0), boundary_mesh (0), node_on_boundary (0), boundary_nodes (0), edge_tetrahedrons (0), triangle_tetrahedrons (0)
	{
		if (octahedron) Initialize_Octahedron_Mesh (m, n, p);
		else Initialize_Cube_Mesh (m, n, p);
	}

	TETRAHEDRON_MESH (const LIST_ARRAYS<int>& tetrahedron_list)
		: number_nodes (0), neighbor_nodes (0), incident_tetrahedrons (0), adjacent_tetrahedrons (0), neighbor_tetrahedrons (0), segment_mesh (0), triangle_mesh (0), tetrahedron_edges (0),
		  tetrahedron_faces (0), boundary_mesh (0), node_on_boundary (0), boundary_nodes (0), edge_tetrahedrons (0), triangle_tetrahedrons (0)
	{
		Initialize_Tetrahedron_Mesh (tetrahedron_list);
	}

	TETRAHEDRON_MESH (const TETRAHEDRON_MESH& tetrahedron_mesh)
		: number_nodes (0), tetrahedrons(), neighbor_nodes (0), incident_tetrahedrons (0), adjacent_tetrahedrons (0), neighbor_tetrahedrons (0), segment_mesh (0), triangle_mesh (0),
		  tetrahedron_edges (0), tetrahedron_faces (0), boundary_mesh (0), node_on_boundary (0), boundary_nodes (0), edge_tetrahedrons (0), triangle_tetrahedrons (0)
	{
		Initialize_Tetrahedron_Mesh (tetrahedron_mesh);
	}

	~TETRAHEDRON_MESH();

	void Clean_Up_Memory()
	{
		number_nodes = 0;
		tetrahedrons.Exact_Resize_Array (4, 0);
		Delete_Auxiliary_Structures();
	}

	void Refresh_Auxiliary_Structures()
	{
		if (neighbor_nodes) Initialize_Neighbor_Nodes();

		if (incident_tetrahedrons) Initialize_Incident_Tetrahedrons();

		if (adjacent_tetrahedrons) Initialize_Adjacent_Tetrahedrons();

		if (neighbor_tetrahedrons) Initialize_Neighbor_Tetrahedrons();

		if (segment_mesh) Initialize_Segment_Mesh();

		if (triangle_mesh) Initialize_Triangle_Mesh();

		if (tetrahedron_edges) Initialize_Tetrahedron_Edges();

		if (tetrahedron_faces) Initialize_Tetrahedron_Faces();

		if (boundary_mesh) Initialize_Boundary_Mesh();

		if (node_on_boundary) Initialize_Node_On_Boundary();

		if (boundary_nodes) Initialize_Boundary_Nodes();

		if (edge_tetrahedrons) Initialize_Edge_Tetrahedrons();

		if (triangle_tetrahedrons) Initialize_Triangle_Tetrahedrons();
	}

	void Initialize_Octahedron_Mesh (const int m, const int n, const int p)
	{
		Clean_Up_Memory();
		number_nodes = m * n * p + (m + 1) * (n + 1) * (p + 1);
		tetrahedrons.Exact_Resize_Array (4, 4 * (m - 1) *n * p + 4 * m * (n - 1) *p + 4 * m * n * (p - 1));
		int t = 0, i, j, k;

		for (i = 1; i <= m; i++) for (j = 1; j <= n; j++) for (k = 1; k <= p - 1; k++) // loop over k-oriented edges in inner cube mesh
				{
					tetrahedrons.Set (++t, Lattice (i, j, k, m, n, p), Lattice (i, j, k + 1, m, n, p), Half_Lattice (i - 1, j - 1, k, m, n, p), Half_Lattice (i, j - 1, k, m, n, p));
					tetrahedrons.Set (++t, Lattice (i, j, k, m, n, p), Lattice (i, j, k + 1, m, n, p), Half_Lattice (i, j - 1, k, m, n, p), Half_Lattice (i, j, k, m, n, p));
					tetrahedrons.Set (++t, Lattice (i, j, k, m, n, p), Lattice (i, j, k + 1, m, n, p), Half_Lattice (i, j, k, m, n, p), Half_Lattice (i - 1, j, k, m, n, p));
					tetrahedrons.Set (++t, Lattice (i, j, k, m, n, p), Lattice (i, j, k + 1, m, n, p), Half_Lattice (i - 1, j, k, m, n, p), Half_Lattice (i - 1, j - 1, k, m, n, p));
				}

		for (i = 1; i <= m; i++) for (j = 1; j <= n - 1; j++) for (k = 1; k <= p; k++) // loop over j-oriented edge in inner cube mesh
				{
					tetrahedrons.Set (++t, Lattice (i, j, k, m, n, p), Lattice (i, j + 1, k, m, n, p), Half_Lattice (i, j, k - 1, m, n, p), Half_Lattice (i - 1, j, k - 1, m, n, p));
					tetrahedrons.Set (++t, Lattice (i, j, k, m, n, p), Lattice (i, j + 1, k, m, n, p), Half_Lattice (i, j, k, m, n, p), Half_Lattice (i, j, k - 1, m, n, p));
					tetrahedrons.Set (++t, Lattice (i, j, k, m, n, p), Lattice (i, j + 1, k, m, n, p), Half_Lattice (i - 1, j, k, m, n, p), Half_Lattice (i, j, k, m, n, p));
					tetrahedrons.Set (++t, Lattice (i, j, k, m, n, p), Lattice (i, j + 1, k, m, n, p), Half_Lattice (i - 1, j, k - 1, m, n, p), Half_Lattice (i - 1, j, k, m, n, p));
				}

		for (i = 1; i <= m - 1; i++) for (j = 1; j <= n; j++) for (k = 1; k <= p; k++) // loop over i-oriented edge in inner cube mesh
				{
					tetrahedrons.Set (++t, Lattice (i, j, k, m, n, p), Lattice (i + 1, j, k, m, n, p), Half_Lattice (i, j - 1, k - 1, m, n, p), Half_Lattice (i, j, k - 1, m, n, p));
					tetrahedrons.Set (++t, Lattice (i, j, k, m, n, p), Lattice (i + 1, j, k, m, n, p), Half_Lattice (i, j, k - 1, m, n, p), Half_Lattice (i, j, k, m, n, p));
					tetrahedrons.Set (++t, Lattice (i, j, k, m, n, p), Lattice (i + 1, j, k, m, n, p), Half_Lattice (i, j, k, m, n, p), Half_Lattice (i, j - 1, k, m, n, p));
					tetrahedrons.Set (++t, Lattice (i, j, k, m, n, p), Lattice (i + 1, j, k, m, n, p), Half_Lattice (i, j - 1, k, m, n, p), Half_Lattice (i, j - 1, k - 1, m, n, p));
				}
	}

	void Initialize_Cube_Mesh (const int m, const int n, const int p) // 5 tetrahedrons per cube
	{
		Clean_Up_Memory();
		number_nodes = m * n * p;
		tetrahedrons.Exact_Resize_Array (4, 5 * (m - 1) * (n - 1) * (p - 1));
		int t = 0;

		for (int i = 1; i <= m - 1; i++) for (int j = 1; j <= n - 1; j++) for (int k = 1; k <= p - 1; k++)
				{
					if ( (i + j + k) % 2 == 0)
					{
						tetrahedrons.Set (++t, i + m * (j - 1) + m * n * (k - 1), i + 1 + m * (j - 1) + m * n * (k - 1), i + m * j + m * n * (k - 1), i + m * (j - 1) + m * n * k);
						tetrahedrons.Set (++t, i + 1 + m * (j - 1) + m * n * (k - 1), i + 1 + m * (j - 1) + m * n * k, i + 1 + m * j + m * n * k, i + m * (j - 1) + m * n * k);
						tetrahedrons.Set (++t, i + m * j + m * n * (k - 1), i + 1 + m * j + m * n * (k - 1), i + 1 + m * j + m * n * k, i + 1 + m * (j - 1) + m * n * (k - 1));
						tetrahedrons.Set (++t, i + m * j + m * n * k, i + 1 + m * j + m * n * k, i + m * (j - 1) + m * n * k, i + m * j + m * n * (k - 1));
						tetrahedrons.Set (++t, i + 1 + m * (j - 1) + m * n * (k - 1), i + m * (j - 1) + m * n * k, i + 1 + m * j + m * n * k, i + m * j + m * n * (k - 1));
					}
					else
					{
						tetrahedrons.Set (++t, i + m * (j - 1) + m * n * (k - 1), i + 1 + m * (j - 1) + m * n * (k - 1), i + 1 + m * j + m * n * (k - 1), i + 1 + m * (j - 1) + m * n * k);
						tetrahedrons.Set (++t, i + m * (j - 1) + m * n * (k - 1), i + m * j + m * n * (k - 1), i + m * j + m * n * k, i + 1 + m * j + m * n * (k - 1));
						tetrahedrons.Set (++t, i + m * j + m * n * k, i + 1 + m * (j - 1) + m * n * k, i + m * (j - 1) + m * n * k, i + m * (j - 1) + m * n * (k - 1));
						tetrahedrons.Set (++t, i + m * j + m * n * k, i + 1 + m * j + m * n * k, i + 1 + m * (j - 1) + m * n * k, i + 1 + m * j + m * n * (k - 1));
						tetrahedrons.Set (++t, i + m * j + m * n * k, i + m * (j - 1) + m * n * (k - 1), i + 1 + m * j + m * n * (k - 1), i + 1 + m * (j - 1) + m * n * k);
					}
				}
	}

	void Initialize_Tetrahedron_Mesh (const LIST_ARRAYS<int>& tetrahedron_list) // construct a mesh given a list of tetrahedrons
	{
		Clean_Up_Memory();
		tetrahedrons.Exact_Resize_Array (4, tetrahedron_list.m);
		LIST_ARRAYS<int>::copy (tetrahedron_list, tetrahedrons);

		for (int t = 1; t <= tetrahedrons.m; t++)
		{
			int i, j, k, l;
			tetrahedrons.Get (t, i, j, k, l);
			number_nodes = max (number_nodes, i, j, k, l);
		}
	}

	void Initialize_Tetrahedron_Mesh (const TETRAHEDRON_MESH& tetrahedron_mesh) // works with the copy constructor
	{
		Initialize_Tetrahedron_Mesh (tetrahedron_mesh.tetrahedrons);
	}

	bool Node_In_Tetrahedron (const int node, const int tetrahedron) const
	{
		int i, j, k, l;
		tetrahedrons.Get (tetrahedron, i, j, k, l);
		return i == node || j == node || k == node || l == node;
	}

	bool Triangle_In_Tetrahedron (const int node1, const int node2, const int node3, const int tetrahedron) const
	{
		return Node_In_Tetrahedron (node1, tetrahedron) && Node_In_Tetrahedron (node2, tetrahedron) && Node_In_Tetrahedron (node3, tetrahedron);
	}

	bool Face_Neighbors (const int tetrahedron1, const int tetrahedron2) const
	{
		int i, j, k, l;
		tetrahedrons.Get (tetrahedron1, i, j, k, l);
		return Triangle_In_Tetrahedron (j, k, l, tetrahedron2) || Triangle_In_Tetrahedron (i, k, l, tetrahedron2) ||
		       Triangle_In_Tetrahedron (i, j, l, tetrahedron2) || Triangle_In_Tetrahedron (i, j, k, tetrahedron2);
	}

	void Other_Three_Nodes (const int node, const int tetrahedron, int& other_node1, int& other_node2, int& other_node3) const
	{
		assert (Node_In_Tetrahedron (node, tetrahedron));
		int i, j, k, l;
		tetrahedrons.Get (tetrahedron, i, j, k, l);

		if (i == node)
		{
			other_node1 = j;
			other_node2 = k;
			other_node3 = l;
		}
		else if (j == node)
		{
			other_node1 = i;
			other_node2 = l;
			other_node3 = k;
		}
		else if (k == node)
		{
			other_node1 = l;
			other_node2 = i;
			other_node3 = j;
		}
		else if (l == node)
		{
			other_node1 = k;
			other_node2 = j;
			other_node3 = i;
		}
	}

	void Other_Two_Nodes (const int node1, const int node2, const int tetrahedron, int& other_node1, int& other_node2) const
	{
		assert (Node_In_Tetrahedron (node1, tetrahedron) && Node_In_Tetrahedron (node2, tetrahedron));
		int i, j, k, l;
		tetrahedrons.Get (tetrahedron, i, j, k, l);

		if (i == node1)
		{
			if (j == node2)
			{
				other_node1 = k;
				other_node2 = l;
			}
			else if (k == node2)
			{
				other_node1 = l;
				other_node2 = j;
			}
			else
			{
				other_node1 = j;
				other_node2 = k;
			}
		}
		else if (j == node1)
		{
			if (i == node2)
			{
				other_node1 = l;
				other_node2 = k;
			}
			else if (k == node2)
			{
				other_node1 = i;
				other_node2 = l;
			}
			else
			{
				other_node1 = k;
				other_node2 = i;
			}
		}
		else if (k == node1)
		{
			if (i == node2)
			{
				other_node1 = j;
				other_node2 = l;
			}
			else if (l == node2)
			{
				other_node1 = i;
				other_node2 = j;
			}
			else
			{
				other_node1 = l;
				other_node2 = i;
			}
		}
		else if (l == node1)
		{
			if (i == node2)
			{
				other_node1 = k;
				other_node2 = j;
			}
			else if (k == node2)
			{
				other_node1 = j;
				other_node2 = i;
			}
			else
			{
				other_node1 = i;
				other_node2 = k;
			}
		}
	}

private:
	int Lattice (const int i, const int j, const int k, const int m, const int n, const int p) const
	{
		return i + m * (j - 1) + m * n * (k - 1);
	}

	int Half_Lattice (const int i, const int j, const int k, const int m, const int n, const int p) const
	{
		return m * n * p + i + 1 + (m + 1) * j + (m + 1) * (n + 1) * k;
	}

public:
	template <class RW>
	void Read (std::istream& input_stream)
	{
		Clean_Up_Memory();
		Read_Binary<RW> (input_stream, number_nodes);
		tetrahedrons.template Read<RW> (input_stream);
	}

	template <class RW>
	void Write (std::ostream& output_stream) const
	{
		Write_Binary<RW> (output_stream, number_nodes);
		tetrahedrons.template Write<RW> (output_stream);
	}

//#####################################################################
	void Delete_Auxiliary_Structures();
	int Tetrahedron (const int node1, const int node2, const int node3, const int node4) const;
	void Initialize_Neighbor_Nodes();
	void Initialize_Incident_Tetrahedrons();
	void Initialize_Adjacent_Tetrahedrons();
private: // helper function for Initialize_Adjacent_Tetrahedrons
	void Find_And_Append_Adjacent_Tetrahedrons (const int tetrahedron, const int node1, const int node2, const int node3);
	int Number_Of_Tetrahedrons_Across_Face (const int tetrahedron, const int node1, const int node2, const int node3) const;
public:
	void Initialize_Neighbor_Tetrahedrons();
	void Initialize_Segment_Mesh();
	void Initialize_Triangle_Mesh();
	void Initialize_Tetrahedron_Edges();
	void Initialize_Tetrahedron_Faces();
	void Initialize_Boundary_Mesh();
	void Initialize_Node_On_Boundary();
	void Initialize_Boundary_Nodes();
	void Initialize_Edge_Tetrahedrons();
	void Initialize_Triangle_Tetrahedrons();
	int Delete_Tetrahedrons_With_Missing_Nodes(); // returns the number deleted
	void Delete_Tetrahedrons (const LIST_ARRAY<int>& deletion_list);
	int Number_Of_Boundary_Tetrahedrons();
	int Number_Of_Interior_Tetrahedrons();
	int Number_Of_Nodes_With_Minimum_Valence();
	int Minimum_Valence (int* index = 0);
	bool Edge_Neighbors (const int tet1, const int tet2) const;
	int Number_Of_Edge_Neighbors (const int segment) const;
	void Initialize_Segment_Mesh_Of_Subset (SEGMENT_MESH& segment_mesh_of_subset, const LIST_ARRAY<bool>& subset) const;
	void Initialize_Boundary_Mesh_Of_Subset (TRIANGLE_MESH& boundary_mesh_of_subset, const LIST_ARRAY<bool>& subset);
	void Update_Adjacent_Tetrahedrons_From_Incident_Tetrahedrons (const int node);
	void Update_Neighbor_Nodes_From_Incident_Tetrahedrons (const int node);
	void Mark_Face_Connected_Component_Incident_On_A_Node (const int node, const int tetrahedron_index_in_incident_tetrahedrons, ARRAY<bool>& marked) const;
	int Tetrahedrons_On_Edge (const int node1, const int node2, LIST_ARRAY<int>* tetrahedrons_on_edge) const;
	int Tetrahedrons_Across_Face (const int tetrahedron, const int node1, const int node2, const int node3, LIST_ARRAY<int>& tetrahedrons_across_face) const;
	void Identify_Face_Connected_Components (ARRAY<int>& label);
	void Identify_Edge_Connected_Components (ARRAY<int>& label);
//#####################################################################
};
}
#endif
