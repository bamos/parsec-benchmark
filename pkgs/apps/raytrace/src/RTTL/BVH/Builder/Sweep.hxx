#ifndef RTTL__BVH_BUILDER_SWEEP_HXX
#define RTTL__BVH_BUILDER_SWEEP_HXX

#include "Builder.hxx"

namespace RTTL {

  class SweepBVHBuilder : public BVHBuilder
  {
  private:
    void recursiveBuild(const int start,
            const int end,
            const int nodeIndex,
            const AABB& bounds,
            const sse_f *const centroid,
            int *const item,
            AABB *const bvh,
            int &numNodes,
            const int depth);

    unsigned int adjustBounds(AABB *const bvh,
                  const AABB *const aabb,
                  const int *const item,
                  const unsigned int index,
                  const unsigned int start);

    void checkTree(AABB *const bvh,
           const int *const item,
           const unsigned int bvhIndex=0);

  public:
    SweepBVHBuilder(BVH *bvh) : BVHBuilder(bvh) {};

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
