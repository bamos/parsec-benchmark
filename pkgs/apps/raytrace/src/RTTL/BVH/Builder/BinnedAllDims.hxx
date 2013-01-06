#ifndef RTTL__BVH_BUILDER_BINNEDALLDIMS_HXX
#define RTTL__BVH_BUILDER_BINNEDALLDIMS_HXX

#include "Builder.hxx"

namespace RTTL {

  class BinnedAllDims : public BVHBuilder
  {
    void recursiveBuild(const AABB *const triAABB,
            const sse_f *const triCentroid,
            AABB *const bvh,
            int *const item,
            int nodeID, int &nextFree,
            int begin, int end,
            AABB &voxel);

  public:
    BinnedAllDims(BVH *bvh) : BVHBuilder(bvh)
    {
    }

    virtual void build(const RTBoxSSE &sceneAABB,const RTBoxSSE &centroidAABB)
    {
      AABB *aabb = bvh->getAllPrimitiveBounds();
      int prims = bvh->numPrimitives();
      bvh->reserve(prims);
      my_build(aabb,bvh->item,bvh->node,prims);
      bvh->doneWithAllPrimitiveBounds(aabb);
    }

    void my_build(const AABB *const aabb,
          int *const item,
          AABB *const bvh,
          const int numBoxes);

  };

} // end namespace
#endif
