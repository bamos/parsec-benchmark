#ifndef RTTL__BVH_BUILDER_HXX
#define RTTL__BVH_BUILDER_HXX

#include "../BVH.hxx"

namespace RTTL {

#define MAX_DEPTH 63
#define MIN_ITEMS 1
#define TRAVERSAL_COST 13.0f
#define INTERSECTION_COST 20.0f


  struct BVH;

  /*! abstract base class for bvh builders. should even support
    build-from-hierarchy type builders, even though we don't have
    any,yet .. */
  class BVHBuilder
  {
  public:
    /*! bvh that this builder is supposed to build. */
    BVH *bvh;

    struct Options {
      /*! string specifying which builder to use by default ... (i.e.,
    if no other one is specified) */
      static const char *defaultBuilder;
    };

    BVHBuilder(BVH *bvhToBeBuilt) : bvh(bvhToBeBuilt) {};

    /*! build the BVH -- assume there's interfaces for getting what
      the builder needs (the builder is attached to a specific BVH,
      anyway, as passed by its constructor) */
    virtual void build(const RTBoxSSE &sceneAABB,const RTBoxSSE &centroidAABB) = 0;

    /*! look up builder type in a registry, return that type
      (hardcoded for now */
    static BVHBuilder *get(const char *builderType, BVH *bvh);
  };


} // end namespace
#endif
