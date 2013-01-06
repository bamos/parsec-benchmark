#ifndef RTTL__BVH_BUILDER_BINNEDALLDIMS_SAVESPACE_HXX
#define RTTL__BVH_BUILDER_BINNEDALLDIMS_SAVESPACE_HXX

#include "Builder.hxx"

namespace RTTL {

  static const int maxBins = 32; // must be a multiple of 4

  class BinnedAllDimsSaveSpace : public BVHBuilder
  {
  protected:

    struct BinTable {
      int  binCount[3][maxBins];
      AABB binBounds[3][maxBins];
      AABB rightBox[maxBins+1];
    };

    _ALIGN(DEFAULT_ALIGNMENT) static BinTable binTable; // for multiple threads these arrays have to allocated per thread

    _INLINE sse_f computeScale(const sse_f distance, const int numBins)
    {
      // set scale to zero if 1.0f/zero would be inf
      return _mm_blendv_ps(_mm_div_ps(_mm_set_ps1(numBins * 0.999f),distance),
               _mm_setzero_ps(),_mm_cmpneq_ps(distance,_mm_setzero_ps()));
    }

    _INLINE void clearBins3Dim(const int numBins, BinTable &bin)
    {
      for (int dim=0;dim<3;dim++)
    {
      for (int i=0;i<numBins;i++)
        bin.binBounds[dim][i].setEmpty();

      for (int i=0;i<numBins;i++)
        bin.binCount[dim][i] = 0;
    }
    }

    _INLINE void updateBinAll3Dim(const CentroidDiffAABB& cdAABB,
                  const sse_f &c,
                  const sse_f &scale,
                  BinTable &bin)
    {
      const sse_i binID = _mm_cvttps_epi32((cdAABB.centroid() - c) * scale);
      const int bin0 = M128_INT(binID,0);
      const int bin1 = M128_INT(binID,1);
      const int bin2 = M128_INT(binID,2);
      const AABB aabb = cdAABB.convert();
      bin.binBounds[0][bin0].extend(aabb);
      bin.binBounds[1][bin1].extend(aabb);
      bin.binBounds[2][bin2].extend(aabb);
      bin.binCount[0][bin0]++;
      bin.binCount[1][bin1]++;
      bin.binCount[2][bin2]++;
    }
    
    void recursiveBuild(const AABB *const triAABB,
            const sse_f *const triCentroid,
            AABB *const bvh,
            int *const item,
            int nodeID, int &nextFree,
            int begin, int end,
            AABB &voxel,
            const AABB &centroidBounds);

    void recursiveBuildFast(const CentroidDiffAABB *const cdAABB,
                int nodeID,
                int &nextFree,
                int begin,
                int end,
                const AABB &voxel,
                const AABB &centroidBounds,
                const bool skipBinning = false);


  public:
    BinnedAllDimsSaveSpace(BVH *bvh) : BVHBuilder(bvh)
    {
    }

    virtual void build(const RTBoxSSE &sceneAABB,const RTBoxSSE &centroidAABB);

  };

} // end namespace
#endif
