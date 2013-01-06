#include "BVH.hxx"
#include "Builder/Builder.hxx"

/* NOTE (iw) : all builder codes moved to 'Builder/' subdir */

#define DBG(a) a

namespace RTTL
{
  BVHStatCollector BVHStatCollector::global;

  void BVH::build(const RTBoxSSE &sceneAABB,const RTBoxSSE &centroidAABB)
  {
    if (builder == NULL)
      setBuilder("default");
    builder->build(sceneAABB,centroidAABB);
  }

  void BVH::setBuilder(const char *builderType)
  {
    if (builder) delete builder;
    builder = BVHBuilder::get(builderType,this);
  };

  void BVH::reserve(int numPrimitives)
  {
    if (numPrimitives > reservedSize)
      {
    if (node) aligned_free(node);
    if (item) aligned_free(node);

    node = aligned_malloc<AABB>(2*numPrimitives);
    item = aligned_malloc<int>(numPrimitives);

    reservedSize = numPrimitives;
      }
  }

};
