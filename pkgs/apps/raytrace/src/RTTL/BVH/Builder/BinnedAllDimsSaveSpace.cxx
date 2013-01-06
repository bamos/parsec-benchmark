#include "BinnedAllDimsSaveSpace.hxx"

namespace RTTL {

  /* -- Conference Scene Stats --
     numTraversalSteps/(float)numPackets = 57.8138
     numLeafIntersections/(float)numPackets = 3.91992
     numPrimitiveIntersections/(float)numPackets = 12.9996
     numFirstHitTests        52.1669%
     numIntervalPruningTests 37.6962%

     - 32 full bins -
     numTraversalSteps/(float)numPackets = 57.7582
     numLeafIntersections/(float)numPackets = 3.90656
     numPrimitiveIntersections/(float)numPackets = 12.9042
     numFirstHitTests        52.1633%
     numIntervalPruningTests 37.7055%

     - const int numBins = min(maxBins,items) , 32 full bins - 
     numTraversalSteps/(float)numPackets = 57.744
     numLeafIntersections/(float)numPackets = 3.90161
     numPrimitiveIntersections/(float)numPackets = 12.9033
     numFirstHitTests        52.1563%
     numIntervalPruningTests 37.7099%

     numPackets = 16384
     numTraversalSteps/(float)numPackets = 57.7657
     numLeafIntersections/(float)numPackets = 3.90887
     numPrimitiveIntersections/(float)numPackets = 12.9417
     numFirstHitTests        52.1662%
     numIntervalPruningTests 37.7069%
     numLazyBuildSteps = 0

  */

#define CENTROID_DIFF_BUILD

  //#define CHECK_ONLY_LARGEST_DIMENSION

  _ALIGN(DEFAULT_ALIGNMENT) BinnedAllDimsSaveSpace::BinTable BinnedAllDimsSaveSpace::binTable;

  void BinnedAllDimsSaveSpace::build(const RTBoxSSE &sceneAABB,const RTBoxSSE &centroidAABB)
  {
    AABB *aabb = bvh->getAllPrimitiveBounds();
    const int numAABBs = bvh->numPrimitives();
    bvh->reserve(numAABBs);

#if !defined(CENTROID_DIFF_BUILD)
    sse_f *centroid = (sse_f*)malloc_align(sizeof(sse_f)*numAABBs);
    AABB triBounds,centroidBounds;
    triBounds.setEmpty();
    centroidBounds.setEmpty();
    for (int i=0;i<numAABBs;i++)
      {
	bvh->item[i] = i;
	centroid[i] = aabb[i].center();
	triBounds.extend(aabb[i]);
	centroidBounds.extend(centroid[i]);
      }

    int nextFreeNode = 1;
    recursiveBuild(aabb,centroid,bvh->node,bvh->item,0,nextFreeNode,0,numAABBs,triBounds,centroidBounds);
    free_align(centroid);

#else // centroid/diff based
    CentroidDiffAABB *cdAABB = (CentroidDiffAABB*)aabb; //ugly I know

    clearBins3Dim(maxBins,binTable);
    const sse_f centroidBoundsMin = centroidAABB.min_f();
    const sse_f distance = centroidAABB.diameter();
    const sse_f scale = computeScale(distance,maxBins);

#pragma prefetch 
    for (int i=0;i<numAABBs;i++)
      {
	bvh->item[i] = i;
	CentroidDiffAABB cd = aabb[i];
	updateBinAll3Dim(cd,centroidBoundsMin,scale,binTable);
	cdAABB[i] = cd;
      }

    int nextFreeNode = 1;
    recursiveBuildFast(cdAABB,0,nextFreeNode,0,numAABBs,sceneAABB,centroidAABB,true);
#endif

    bvh->doneWithAllPrimitiveBounds(aabb);
  }

  void BinnedAllDimsSaveSpace::recursiveBuildFast(const CentroidDiffAABB *const cdAABB,
						  int nodeID, 
						  int &nextFree,
						  int begin, 
						  int end,
						  const AABB &voxel,
						  const AABB &centroidBounds,
						  const bool skipBinning)
  {
    const int items = end-begin;
    int *const item = bvh->item;
    if (items <= MIN_ITEMS)
      {
      createLeaf:	
	bvh->node[nodeID] = voxel;
	bvh->node[nodeID].createLeaf(begin,end-begin);
	return;
      }
    const int numBins = min(maxBins,2 + 2*(int)sqrtf(items)); 
    const sse_f c_min = centroidBounds.min_f();
    const sse_f distance = centroidBounds.diameter();
    const sse_f scale = computeScale(distance,numBins);

//     DBG_PRINT(voxel);
//     DBG_PRINT(numBins);
//     DBG_PRINT(centroidBounds);
//     DBG_PRINT(c_min);
//     DBG_PRINT(distance);
//     DBG_PRINT(scale);

    /* --------------------------------------------------- */
    if (__builtin_expect(skipBinning == false,1))
      {
	clearBins3Dim(numBins,binTable);
	const sse_f centroidBoundsMin = centroidBounds.min_f();

	for (int i=begin;i<end;i++)
	  updateBinAll3Dim(cdAABB[item[i]],centroidBoundsMin,scale,binTable);
      }
    /* --------------------------------------------------- */

    AABB leftTriAABB,rightTriAABB;


    float voxelArea = voxel.area();

//     DBG_PRINT(voxel);
//     DBG_PRINT(voxelArea);
//     DBG_PRINT(voxel.diameter());

    assert(voxelArea >= 0.0f);
    int bestSplit = -1;
    int bestSplitDim = -1;
    float bestCost = items * voxelArea;

    for (int dim=0;dim<3;dim++)
      {
	const int binDim = dim;
	if (__builtin_expect(M128_FLOAT(distance,dim) == 0.0f,0)) continue;

	AABB rightBounds;
	rightBounds.setEmpty();
	AABB rightCentroidBounds;
	rightCentroidBounds.setEmpty();
	for (int i=numBins-1;i>=0;--i)
	  {
	    rightBounds.extend(binTable.binBounds[binDim][i]);
	    binTable.rightBox[i] = rightBounds;
	  }

	AABB leftBounds;
	leftBounds.setEmpty();

	for (int i=1,count = 0;i<numBins;i++)
	  {
	    leftBounds.extend(binTable.binBounds[binDim][i-1]);
	    count += binTable.binCount[binDim][i-1];

	    const int lnum = count;
	    const int rnum = items-lnum;

	    if (__builtin_expect(lnum == 0 || rnum == 0,0)) continue;

	    const float lsa = leftBounds.area();
	    const float rsa = binTable.rightBox[i].area(); 
	    const float cost =  (lsa * lnum + rsa * rnum + 1.0f * voxelArea); // the '1' is the traversal cost...
	    //float cost =  (lsa * lnum + rsa * rnum) * INTERSECTION_COST + voxel.area() * TRAVERSAL_COST;

	    if (cost < bestCost)
	      {
		bestCost = cost;
		bestSplit = i;
		bestSplitDim = dim;
		leftTriAABB = leftBounds;
		rightTriAABB = binTable.rightBox[i];
	      }

	  }
      }

    if (bestSplit == -1) { goto createLeaf; }

    int *l = item + begin;
    int *r = item + end-1;
    int mid;

    AABB leftCentroidBounds;
    AABB rightCentroidBounds;
    leftCentroidBounds.setEmpty();
    rightCentroidBounds.setEmpty();
    const float c = M128_FLOAT(c_min,bestSplitDim);
    const float s = M128_FLOAT(scale,bestSplitDim);
    while (1)
      {
	//todo: save conversion
	while (l < r && int((cdAABB[*l].centroid(bestSplitDim) - c) * s) < bestSplit)
	  {
	    leftCentroidBounds.extend(cdAABB[*l].centroid());
	    ++l;
	  }
	while (l < r && int((cdAABB[*r].centroid(bestSplitDim) - c) * s) >= bestSplit)
	  {
	    rightCentroidBounds.extend(cdAABB[*r].centroid());
	    --r;
	  }
	if (l == r)
	  {
	    mid = r - (item);
	    rightCentroidBounds.extend(cdAABB[*r].centroid());
	    break;
	  }
	const int h = *l; *l = *r; *r = h;
      }        

    bvh->node[nodeID] = voxel;
    bvh->node[nodeID].createNode(nextFree,bestSplitDim);

    nextFree += 2;
    recursiveBuildFast(cdAABB,bvh->node[nodeID].children()+0,nextFree,begin,mid,leftTriAABB,leftCentroidBounds);
    recursiveBuildFast(cdAABB,bvh->node[nodeID].children()+1,nextFree,mid,end,rightTriAABB,rightCentroidBounds);
  }


  void BinnedAllDimsSaveSpace::recursiveBuild(const AABB *const triAABB,
					      const sse_f *const triCentroid,
					      AABB *const bvh,
					      int *const item,
					      int nodeID, 
					      int &nextFree,
					      int begin, 
					      int end,
					      AABB &voxel,
					      const AABB &centroidBounds)
  {
    const int items = end-begin;
    const int numBins = min(maxBins,2*items); //const int numBins = maxBins;

    if (end - begin <= MIN_ITEMS)
      {
      createLeaf:
	bvh[nodeID] = voxel;
	bvh[nodeID].createLeaf(begin,end-begin);
	return;
      }

    const sse_f c_min = centroidBounds.min_f();
    const sse_f distance = centroidBounds.diameter();
    // set scale to zero if 1.0f/zero would be inf
    const sse_f scale = _mm_blendv_ps(_mm_div_ps(_mm_set_ps1(numBins * 0.999f),distance),
				      _mm_setzero_ps(),
				      _mm_cmpneq_ps(distance,_mm_setzero_ps()));
    const float voxelArea = voxel.area();
    int bestSplit = -1;
    int bestSplitDim = -1;
    float bestCost = items * voxelArea;


#if defined(CHECK_ONLY_LARGEST_DIMENSION)
    /* --------------------------------------------------- */
    int dim = ((RTVec3f*)&distance)->maxIndex();
    {
      if (__builtin_expect(M128_FLOAT(distance,dim) == 0.0f,0)) goto createLeaf;

#pragma unroll(8)
      for (int i=0;i<numBins;i++)
	binBounds[0][i].setEmpty();

#pragma unroll(8)
      for (int i=0;i<numBins;i++)
	binCount[0][i] = 0;

      const float c = centroidBounds.min3f()[dim];
      const float s = M128_FLOAT(scale,dim);
#pragma unroll(8)
      for (int i=begin;i<end;i++)
	{
	  const int t = item[i];
	  const int bin = int((M128_FLOAT(triCentroid[t],dim) - c) * s);
	  //assert(bin >= 0 && bin < numBins);
	  binBounds[0][bin].extend(triAABB[t]);
	  binCount[0][bin]++;
	}
    }
    /* --------------------------------------------------- */
#else
    /* --------------------------------------------------- */
    {
      for (int dim=0;dim<3;dim++)
	{
	  for (int i=0;i<numBins;i++)
	    binTable.binBounds[dim][i].setEmpty();

	  for (int i=0;i<numBins;i++)
	    binTable.binCount[dim][i] = 0;
	}

      const sse_f c = centroidBounds.min_f();
#pragma unroll(4)
      for (int i=begin;i<end;i++)
	{
	  const int t = item[i];
	  const sse_i bin = _mm_cvttps_epi32((triCentroid[t] - c) * scale);
	  const int bin0 = M128_INT(bin,0);
	  const int bin1 = M128_INT(bin,1);
	  const int bin2 = M128_INT(bin,2);

	  binTable.binBounds[0][bin0].extend(triAABB[t]);
	  binTable.binBounds[1][bin1].extend(triAABB[t]);
	  binTable.binBounds[2][bin2].extend(triAABB[t]);
	  binTable.binCount[0][bin0]++;
	  binTable.binCount[1][bin1]++;
	  binTable.binCount[2][bin2]++;
	}
    }
    /* --------------------------------------------------- */
#endif

    AABB leftTriAABB,rightTriAABB;

#if defined(CHECK_ONLY_LARGEST_DIMENSION)
    for (int d=0;d<1;d++)
      {
	const int binDim = 0;
#else
        for (int dim=0;dim<3;dim++)
	  {
            const int binDim = dim;
#endif
            if (__builtin_expect(M128_FLOAT(distance,dim) == 0.0f,0)) continue;

            AABB rightBounds;
            rightBounds.setEmpty();
            for (int i=numBins-1;i>=0;--i)
	      {
                rightBounds.extend(binTable.binBounds[binDim][i]);
                binTable.rightBox[i] = rightBounds;
	      }

            AABB leftBounds;
            leftBounds.setEmpty();
            for (int i=1,count = 0;i<numBins;i++)
	      {
                leftBounds.extend(binTable.binBounds[binDim][i-1]);
                count += binTable.binCount[binDim][i-1];

                const int lnum = count;
                const int rnum = items-lnum;

                if (__builtin_expect(lnum == 0 || rnum == 0,0)) continue;

                const float lsa = leftBounds.area();
                const float rsa = binTable.rightBox[i].area(); 
                const float cost =  (lsa * lnum + rsa * rnum + 1.0f * voxelArea); // the '1' is the traversal cost...
                //float cost =  (lsa * lnum + rsa * rnum) * INTERSECTION_COST + voxel.area() * TRAVERSAL_COST;

                if (cost < bestCost)
		  {
                    bestCost = cost;
                    bestSplit = i;
                    bestSplitDim = dim;
                    leftTriAABB = leftBounds;
                    rightTriAABB = binTable.rightBox[i];
		  }

	      }
	  }

        if (bestSplit == -1) goto createLeaf;

        int *l = item + begin;
        int *r = item + end-1;
        int mid;

        AABB leftCentroidBounds;
        AABB rightCentroidBounds;
        leftCentroidBounds.setEmpty();
        rightCentroidBounds.setEmpty();
        const float c = M128_FLOAT(c_min,bestSplitDim);
        const float s = M128_FLOAT(scale,bestSplitDim);
        while (1)
	  {
            //todo: save conversion
            while (l < r && int((M128_FLOAT(triCentroid[*l],bestSplitDim) - c) * s) < bestSplit)
	      {
                leftCentroidBounds.extend(triCentroid[*l]);
                ++l;
	      }
            while (l < r && int((M128_FLOAT(triCentroid[*r],bestSplitDim) - c) * s) >= bestSplit)
	      {
                rightCentroidBounds.extend(triCentroid[*r]);
                --r;
	      }
            if (l == r)
	      {
                mid = r - (item);// + 1;
                rightCentroidBounds.extend(triCentroid[*r]);
                break;
	      }
            const int h = *l; *l = *r; *r = h;
	  }

        bvh[nodeID] = voxel;
        bvh[nodeID].createNode(nextFree,bestSplitDim);

        nextFree += 2;
        recursiveBuild(triAABB,triCentroid,bvh,item,bvh[nodeID].children()+0,nextFree,begin,mid,leftTriAABB,leftCentroidBounds);
        recursiveBuild(triAABB,triCentroid,bvh,item,bvh[nodeID].children()+1,nextFree,mid,end,rightTriAABB,rightCentroidBounds);
      }
  };
