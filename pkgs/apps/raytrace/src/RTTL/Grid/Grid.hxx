/*! \file Grid.hxx defines a recursive grid (should probably rename to RecursiveGrid.hxx ... */

#ifndef RTTL_RECURSIVEGRID_HXX
#define RTTL_RECURSIVEGRID_HXX

#include "../common/RTInclude.hxx"
#include "../common/RTBox.hxx"
#include "../common/RTRay.hxx"
#include "../common/RTIntervalArith.hxx"
#include "../Mesh/Mesh.hxx"
#include "../Triangle/Triangle.hxx"
#include "Builder/Builder.hxx"


namespace RTTL {

  typedef RTBox3f box3f;
  typedef RTBox3i box3i;


  struct AABBPrimList
  {
    box3f4 getBounds4(int primID)
  };




  /*! a recursive grid, that is build on-demand 

    \warning can not have more than 1k prims per cell

    \warning can not have more than 1m cells

   */
  struct LazyRecursiveGrid
  {
    box3f primBounds; /*! bounds of all primitives inside this grid */
    box3f cellBounds; /*! bounds of all *cells* of this grid. may be larger than primbounds */
    bool is_built; /*! true if grid is already built, false otherwise */

    // -------------------------------------------------------
    // while not initialized:
    //
    PrimitiveList *primitive;
    int *primID;
    int primIDs;

    vec3i res; /*! resolution of the grid. i.e., the number of cells in x, y, and z dimension */
    vec3f cellWidth; /*! width of a cell, in world space coords */

    /*! describes each cell in 32 bits: if cell is not built, yet, its
        "is_built" flag is zeroed, and the cell can still be either a
        grid or a leaf. in this case, the list of primitives can be
        found in 'cellPrim', and a 'buildCell' has to be called for this
        cell */
    struct CommonCellInfo
    {
      unsigned int is_leaf  :  1;
      unsigned int unused   : 31;
    };
    struct LeafCellInfo
    {
      unsigned int is_leaf  :  1;
      unsigned int begin    : 21; /*!< can NOT have more than 2 million cells !!! */
      unsigned int size     : 10; /*!< can NOT have more than 1024 items per cell !!! */
    };
    struct SubgridCellInfo
    {
      unsigned int is_leaf   :  1;
      unsigned int subgridID : 31;
    };
    /*! cell info per cell ID. */
    CommonCellInfo *cell;

    vector<LazyRecursiveGrid *> subgrid; /*!< list of subgrids, referenced by SubgridCellInfo.subgridID */

    int cellID(vec3i coord) const
    { return coord.x + res.x * (coord.y + res.y * (coord.z)); };
    
//     /*! build the given cell. */
//     void buildCell(int cellID)
//     {
//       if (cell[cellID].is_built) return;
//       cout << "buliding cell " << cellID << endl;
      
//       static const int MAKE_LEAF_THRESHOLD = 8;
//       assert(MAKE_LEAF_THRESHOLD < 1024);
      
//       // check if we'll make a leaf out of it 
//       if (cellPrim[cellID].end - cellPrim[cellID].begin <= MAKE_LEAF_THRESHOLD)
// 	{
// 	  cell[cellID].is_built = true;
// 	  cell[cellID].is_leaf  = true;
// 	  cell[cellID].ID       = leafCell.size();
// 	  leafCell[cell[cellID].ID] = cellPrim[cellID];
// 	}
//       else
// 	{
// 	  cell[cellID].is_built = true;
// 	  cell[cellID].is_leaf  = false;
// 	  cell[cellID].subgridID= subgrid.size();
// 	  subgrid[cell[cellID].ID] = new RecursiveGrid
// 	}
//     }

    LazyRecursiveGrid()
      : is_built = false;
    {
    }


    /*! single threaded build */
    void build_singleThreaded(PrimitiveList &prim, const box3f &bounds, int *primBegin, int *primEnd)
    {
      // first of all: determine res...
#if 1
      int targetNumCells;

      int numPrims = primEnd - primBegin;
      static const int linearNumCellsThreshold = 1000;
      if (numPrims <= linearNumCellsThreshold)
	/* assume that if less than linearNumCellsThreshold we'll
	   probably NOT have an additional recursion level, and that
	   then, we'll pick the num cells as what is ideal for a regular
	   grid */
	targetNumCells = numPrims;
      else
	/*! otherwise, we asssume one more recursion level, in which
	  O(N^2/3) of the cells would actually be filled -- so let's
	  take that as num primitives ... */
	targetNumCells = 1+ powf(numPrims,2./3.);
#else
      int targetNumCells = 1 + (primEnd - primBegin) / 16; // make grid rather coarse...
#endif
      
      res.x = (int)ceilf(diam.x * powf(targetNumCells/diam.volume(),1./3.));
      res.y = (int)ceilf(diam.y * powf(targetNumCells/diam.volume(),1./3.));
      res.z = (int)ceilf(diam.z * powf(targetNumCells/diam.volume(),1./3.));
      assert(res.x >= 1);
      assert(res.y >= 1);
      assert(res.z >= 1);
      numCells = res.volume();
      assert(numCells < 1000000);
      
      // other inits:
      cellBounds = primBounds;
      cellBounds.min -= (diam * 0.0001f);
      cellBounds.max += (diam * 0.0001f);
      
      // temporary arrays during building:
      int *cell_size; /*! number of items per cell. only used temporarily during building */
      int *cell_begin; /*! pointer to start of this cell's item list */
    

      // first stage: clear all counters
      cell_size = new int[numCells+4];
      for (int i=0;i<=numCells;i++)
	cell_size[i] = 0;
      
      // second stage: compute all counters (incrementally)
      for (int i=primBegin; i<primEnd; i++)
	{
	  const box3f4 bounds_i = prim.getBounds4(primID[i]);
	  const box3i4 coords_i = worldToGrid(bounds_i);
	  for (int z=0;z<coords_i.z;z++)
	    for (int y=0;y<coords_i.y;y++)
	      for (int x=0;x<coords_i.x;x++)
		++cell_size[cellID(vec3i(x,y,z))];
	}
      
      // third stage: prefix sum to compute cell_begin pointers 
      int ctr = 0;
      cell_begin = new int[numCells+4];
      for (int i=0;i<=numCells;i++)
	{
	  ctr += cell_size[i];
	  cell_begin[i] = ctr;
	}

      // fourth stage: alloc item list(s), and perform actual engridding...
      int totalReferences = cell_begin[numCells];
      cellIDlist = new int[totalReferences];

      for (int i=primBegin; i<primEnd; i++)
	{
	  const box3f4 bounds_i = prim.getBounds4(primID[i]);
	  const box3i4 coords_i = worldToGrid(bounds_i);
	  for (int z=0;z<coords_i.z;z++)
	    for (int y=0;y<coords_i.y;y++)
	      for (int x=0;x<coords_i.x;x++)
		{
		  int cID = cellID(vec3i(x,y,z));
		  --cell_size[cID];
		  cellIDlist[cell_begin[ID]+cell_size[ID]] = i;
		}
	}
      delete[] cell_size;
      

      // final stage: compute actual cell_infos, and (lazily) create recursive grids where required
      for (int i=0;i<numCells;i++)
	{
	  int numInCell = cell_begin[i+1] - cell_begin[i];
	  static const int makeLeafThreshold = 8;
	  assert(makeLeafThreshold > 0);
	  if (numInCell <= makeLeafThreshold)
	    {
	      cell[i].is_leaf = true;
	      cell[i].begin   = cell_begin[i];
	      cell[i].size    = numInCell;
	      assert(cell[i].size    == numInCell); // to make sure nothing got lost during discretization
	      assert(cell[i].begin   == cell_begin[i]);// to make sure nothing got lost during discretization
	    }
	  else
	    {
	      cell[i].is_leaf = false;
	      RecursiveGrid *grid = new RecursiveGrid(prim,cellIDlist,cell_begin[i],cell_begin[i+1]);
	      cell[i].subgridID = subgrid.size();
	      subgrid.push_back(grid);
	    }
	}
      delete[] cell_begin;
    };
  };
};
