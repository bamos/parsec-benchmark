#include "OnDemandBuilder.hxx"

 _ALIGN(DEFAULT_ALIGNMENT) int  OnDemandBuilder::binCount[3][maxBins];
 _ALIGN(DEFAULT_ALIGNMENT) AABB OnDemandBuilder::binBounds[3][maxBins];
 _ALIGN(DEFAULT_ALIGNMENT) AABB OnDemandBuilder::rightBox[maxBins+1];
 _ALIGN(DEFAULT_ALIGNMENT) int nextFree = 1;

void OnDemandBuilder::build(const RTBoxSSE &sceneAABB,const RTBoxSSE &centroidAABB)
{
  PING;
  AABB *aabb = bvh->getAllPrimitiveBounds();
  const int numAABBs = bvh->numPrimitives();
  bvh->reserve(numAABBs);

  if (cdAABB)
   aligned_free(cdAABB);

  cdAABB = aligned_malloc<CentroidDiffAABB>(numAABBs);

  AABB triBounds;
#pragma prefetch
  for (int i=0;i<numAABBs;i++)
    {
      bvh->item[i] = i;
      CentroidDiffAABB cd = aabb[i];
      triBounds.extend(aabb[i]);
      cdAABB[i] = cd;
    }
  //recursiveBuild(cdAABB,bvh->node,bvh->item,0,nextFreeNode,0,numAABBs,triBounds,centroidBounds);

  DBG_PRINT(numAABBs);
  bvh->node[0] = triBounds;
  bvh->node[0].createLazyNode(0,numAABBs);

  /* don't delete centroid/diff table */

  bvh->doneWithAllPrimitiveBounds(aabb);
}

void OnDemandBuilder::createNode(AABB *const bvh,
                     int *const item,
                     int nodeID)
{
  assert(bvh[nodeID].isLazy());
  BVH_STAT_COLLECTOR(BVHStatCollector::global.numLazyBuildSteps++);
  AABB voxel = bvh[nodeID];
  const int begin = bvh[nodeID].itemOffset();
  const int end = begin+bvh[nodeID].lazyItems();
  const int items = end-begin;
  //DBG_PRINT(begin);
  //DBG_PRINT(end);
  //DBG_PRINT(items);
  if (items <= MIN_ITEMS)
    {
     //todo: don't need to recreate leaf, simple set signs to zero
    createLeaf:
      bvh[nodeID] = voxel;
      bvh[nodeID].createLeaf(begin,end-begin);
      return;
    }
  AABB centroidBounds;
  centroidBounds.setEmpty();
  const sse_f c_min = centroidBounds.min_f();

  for (int i=begin;i<end;i++)
    centroidBounds.extend(cdAABB[item[i]].centroid());

      const int numBins = min(maxBins,2 + 2*(int)sqrtf(items));
      const sse_f distance = centroidBounds.diameter();
      // set scale to zero if 1.0f/zero would be inf
      const sse_f scale = _mm_blendv_ps(_mm_div_ps(_mm_set_ps1(numBins * 0.999f),distance),_mm_setzero_ps(),_mm_cmpneq_ps(distance,_mm_setzero_ps()));
      const float voxelArea = voxel.area();
      int bestSplit = -1;
      int bestSplitDim = -1;
      float bestCost = items * voxelArea;

      /* --------------------------------------------------- */
      clearBins3Dim(numBins);
      const sse_f centroidBoundsMin = centroidBounds.min_f();
      for (int i=begin;i<end;i++)
    updateBinAll3Dim(cdAABB[item[i]],centroidBoundsMin,scale);
      /* --------------------------------------------------- */

      AABB leftTriAABB,rightTriAABB;

      for (int dim=0;dim<3;dim++)
      {
          const int binDim = dim;
          if (__builtin_expect(M128_FLOAT(distance,dim) == 0.0f,0)) continue;

          AABB rightBounds;
          rightBounds.setEmpty();
          for (int i=numBins-1;i>=0;--i)
          {
              rightBounds.extend(binBounds[binDim][i]);
              rightBox[i] = rightBounds;
          }

          AABB leftBounds;
          leftBounds.setEmpty();
          for (int i=1,count = 0;i<numBins;i++)
          {
              leftBounds.extend(binBounds[binDim][i-1]);
              count += binCount[binDim][i-1];

              const int lnum = count;
              const int rnum = items-lnum;

              if (__builtin_expect(lnum == 0 || rnum == 0,0)) continue;

              const float lsa = leftBounds.area();
              const float rsa = rightBox[i].area();
              const float cost =  (lsa * lnum + rsa * rnum + 1.0f * voxelArea); // the '1' is the traversal cost...
              //float cost =  (lsa * lnum + rsa * rnum) * INTERSECTION_COST + voxel.area() * TRAVERSAL_COST;

              if (cost < bestCost)
              {
                  bestCost = cost;
                  bestSplit = i;
                  bestSplitDim = dim;
                  leftTriAABB = leftBounds;
                  rightTriAABB = rightBox[i];
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
              mid = r - (item);// + 1;
              rightCentroidBounds.extend(cdAABB[*r].centroid());
              break;
          }
          const int h = *l; *l = *r; *r = h;
      }

      bvh[nodeID] = voxel;
      bvh[nodeID].createNode(nextFree,bestSplitDim);

      bvh[nextFree+0] = leftTriAABB;
      bvh[nextFree+0].createLazyNode(begin,mid-begin);
      bvh[nextFree+1] = rightTriAABB;
      bvh[nextFree+1].createLazyNode(mid,end-mid);
      nextFree += 2;
}

