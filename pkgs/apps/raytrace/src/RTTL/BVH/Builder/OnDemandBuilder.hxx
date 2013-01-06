#ifndef RTTL__BVH_BUILDER_ON_DEMAND_HXX
#define RTTL__BVH_BUILDER_ON_DEMAND_HXX

#include "BinnedAllDimsSaveSpace.hxx"
#include "RTTL/Mesh/Mesh.hxx"

namespace RTTL {

    class OnDemandBuilder :public BVHBuilder {
    protected:
        CentroidDiffAABB *cdAABB;

    public:

        // for multiple threads these arrays have to allocated per thread
        _ALIGN(DEFAULT_ALIGNMENT) static int  binCount[3][maxBins];
        _ALIGN(DEFAULT_ALIGNMENT) static AABB binBounds[3][maxBins];
        _ALIGN(DEFAULT_ALIGNMENT) static AABB rightBox[maxBins+1];

        _INLINE void clearBins3Dim(const int numBins)
        {
            for (int dim=0;dim<3;dim++)
            {
                for (int i=0;i<numBins;i++)
                    binBounds[dim][i].setEmpty();

                for (int i=0;i<numBins;i++)
                    binCount[dim][i] = 0;
            }
        }

        _INLINE void updateBinAll3Dim(const CentroidDiffAABB& cdAABB,
            const sse_f &c,
            const sse_f &scale)
        {
            const sse_i bin = _mm_cvttps_epi32((cdAABB.centroid() - c) * scale);
            const int bin0 = M128_INT(bin,0);
            const int bin1 = M128_INT(bin,1);
            const int bin2 = M128_INT(bin,2);
            const AABB aabb = cdAABB.convert();
            binBounds[0][bin0].extend(aabb);
            binBounds[1][bin1].extend(aabb);
            binBounds[2][bin2].extend(aabb);
            binCount[0][bin0]++;
            binCount[1][bin1]++;
            binCount[2][bin2]++;
        }

        OnDemandBuilder(BVH *bvh) : BVHBuilder(bvh), cdAABB(NULL)
        {
        }

        void createNode(AABB *const bvh,
            int *const item,
            int nodeID);

        virtual void build(const RTBoxSSE &sceneAABB,const RTBoxSSE &centroidAABB);

    };

    template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS>
    _INLINE void TraverseLazyBVH(RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> &packet,
        AABB *bvh,
        int *item,
        const StandardTriangleMesh &mesh,
        OnDemandBuilder &builder)
    {
        BVH_STAT_COLLECTOR(BVHStatCollector::global.numPackets++);

        struct BVHStack {
            int bvhIndex;
            int fastHitID;
        } bvhStack[MAX_BVH_STACK_DEPTH];
        BVHStack *sptr = bvhStack;

        int raySigns[3];
        int hitID = 0;

        raySigns[0] = _mm_movemask_ps(packet.directionX(0)) == 0 ? 0 : 1;
        raySigns[1] = _mm_movemask_ps(packet.directionY(0)) == 0 ? 0 : 1;
        raySigns[2] = _mm_movemask_ps(packet.directionZ(0)) == 0 ? 0 : 1;

        sptr->bvhIndex = 0;
        sptr->fastHitID = hitID;
        sptr++;

        const unsigned int signsMinX = _mm_movemask_ps(packet.reciprocalMin(0));
        const unsigned int signsMinY = _mm_movemask_ps(packet.reciprocalMin(1));
        const unsigned int signsMinZ = _mm_movemask_ps(packet.reciprocalMin(2));
        const unsigned int signsMaxX = _mm_movemask_ps(packet.reciprocalMax(0));
        const unsigned int signsMaxY = _mm_movemask_ps(packet.reciprocalMax(1));
        const unsigned int signsMaxZ = _mm_movemask_ps(packet.reciprocalMax(2));

        const bool sameSigns =
            (signsMaxX == 0xf || signsMinX == 0x0) &&
            (signsMaxY == 0xf || signsMinY == 0x0) &&
            (signsMaxZ == 0xf || signsMinZ == 0x0);

        /* --------------------------------------------------------- */
        /* -- different ray directions signs -> skip IA traversal -- */
        /* --------------------------------------------------------- */

        if (__builtin_expect(sameSigns == false,0))
            while(1)
            {
start_traverse_diff_signs:
                if (__builtin_expect(sptr == bvhStack,0)) return;
                sptr--;

                unsigned int index = sptr->bvhIndex;
                hitID = sptr->fastHitID;

                while(1) {
                    BVH_STAT_COLLECTOR(BVHStatCollector::global.numTraversalSteps++);
                    if (__builtin_expect(bvh[index].isLazy(),0)) builder.createNode(bvh,item,index);

                    const AABB &entry = bvh[index];
                    const sse_f m_min = entry.m_min[0];
                    const sse_f m_max = entry.m_max[0];
                    sse_f min_x = _mm_shuffle_ps(m_min,m_min,_MM_SHUFFLE(0, 0, 0, 0));
                    sse_f min_y = _mm_shuffle_ps(m_min,m_min,_MM_SHUFFLE(1, 1, 1, 1));
                    sse_f min_z = _mm_shuffle_ps(m_min,m_min,_MM_SHUFFLE(2, 2, 2, 2));
                    sse_f max_x = _mm_shuffle_ps(m_max,m_max,_MM_SHUFFLE(0, 0, 0, 0));
                    sse_f max_y = _mm_shuffle_ps(m_max,m_max,_MM_SHUFFLE(1, 1, 1, 1));
                    sse_f max_z = _mm_shuffle_ps(m_max,m_max,_MM_SHUFFLE(2, 2, 2, 2));
                    if (!MULTIPLE_ORIGINS)
                    {
                        min_x = min_x - packet.originX(0);
                        min_y = min_y - packet.originY(0);
                        min_z = min_z - packet.originZ(0);
                        max_x = max_x - packet.originX(0);
                        max_y = max_y - packet.originY(0);
                        max_z = max_z - packet.originZ(0);
                    }
                    if (RayPacketIntersectAABB<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS, false>
                        (packet,hitID,min_x,max_x,min_y,max_y,min_z,max_z) == 0) // early hit
                    {
                        unsigned int intersectRays = 0;
                        for(int i=hitID+1;i<N;i++)
                        {
                            intersectRays |= RayPacketIntersectAABB<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS, false>(packet,i,min_x,max_x,min_y,max_y,min_z,max_z);
                            hitID = i;
                            if (intersectRays) break;
                        }
                        if (!intersectRays) { goto start_traverse_diff_signs; }
                    }
                    BVH_STAT_COLLECTOR(else BVHStatCollector::global.numFirstHitTests++);
                    if (entry.isLeaf()) break;

                    const unsigned int axis = entry.axis();
                    const unsigned int children = entry.children();
                    const unsigned int rayDir = raySigns[axis] & 1;
                    const unsigned int firstChild = children + rayDir;
                    const unsigned int lastChild  = children + (1^rayDir);
                    index  = firstChild;

                    sptr->bvhIndex  = lastChild;
                    sptr->fastHitID = hitID;
                    sptr++;
                }
                BVH_STAT_COLLECTOR(BVHStatCollector::global.numLeafIntersections++);

                const AABB &entry = bvh[index];
                const unsigned int items = entry.items();
                int *start = (int*)item + entry.itemOffset();
                for (unsigned int i=0;i<items;i++)
                {
                    BVH_STAT_COLLECTOR(BVHStatCollector::global.numPrimitiveIntersections++);
                    const int triID = start[i];
                    const RTVec3f &v0 = mesh.getTriangleVertex(triID,0);
                    const RTVec3f &v1 = mesh.getTriangleVertex(triID,1);
                    const RTVec3f &v2 = mesh.getTriangleVertex(triID,2);
                    IntersectTriangleMoellerTrumbore
                        <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>
                        (v0,v1,v2,triID,packet,hitID,N);
                }
            }

            /* ------------------------------------------------------- */
            /* -- equal ray directions signs -> enable IA traversal -- */
            /* ------------------------------------------------------- */

            const sse_f min_rcp[3] = {
                raySigns[0] ? packet.reciprocalMin(0) : packet.reciprocalMax(0),
                raySigns[1] ? packet.reciprocalMin(1) : packet.reciprocalMax(1),
                raySigns[2] ? packet.reciprocalMin(2) : packet.reciprocalMax(2)
            };

            const sse_f max_rcp[3] = {
                raySigns[0] ? packet.reciprocalMax(0) : packet.reciprocalMin(0),
                raySigns[1] ? packet.reciprocalMax(1) : packet.reciprocalMin(1),
                raySigns[2] ? packet.reciprocalMax(2) : packet.reciprocalMin(2)
            };

            while(1)
            {
start_traverse_same_signs:
                if (__builtin_expect(sptr == bvhStack,0)) return;
                sptr--;

                unsigned int index = sptr->bvhIndex;
                hitID = sptr->fastHitID;

                while(1) {
                    BVH_STAT_COLLECTOR(BVHStatCollector::global.numTraversalSteps++);
                    if (__builtin_expect(bvh[index].isLazy(),0)) builder.createNode(bvh,item,index);

                    const AABB &entry = bvh[index];

                    const sse_f *const t = (sse_f*)&bvh[index];
                    sse_f min_x = _mm_shuffle_ps(t[raySigns[0]],t[raySigns[0]],
                        _MM_SHUFFLE(0, 0, 0, 0));
                    sse_f min_y = _mm_shuffle_ps(t[raySigns[1]],t[raySigns[1]],
                        _MM_SHUFFLE(1, 1, 1, 1));
                    sse_f min_z = _mm_shuffle_ps(t[raySigns[2]],t[raySigns[2]],
                        _MM_SHUFFLE(2, 2, 2, 2));
                    sse_f max_x = _mm_shuffle_ps(t[raySigns[0]^1],t[raySigns[0]^1],
                        _MM_SHUFFLE(0, 0, 0, 0));
                    sse_f max_y = _mm_shuffle_ps(t[raySigns[1]^1],t[raySigns[1]^1],
                        _MM_SHUFFLE(1, 1, 1, 1));
                    sse_f max_z = _mm_shuffle_ps(t[raySigns[2]^1],t[raySigns[2]^1],
                        _MM_SHUFFLE(2, 2, 2, 2));
                    if (!MULTIPLE_ORIGINS)
                    {
                        min_x = min_x - packet.originX(0);
                        min_y = min_y - packet.originY(0);
                        min_z = min_z - packet.originZ(0);
                        max_x = max_x - packet.originX(0);
                        max_y = max_y - packet.originY(0);
                        max_z = max_z - packet.originZ(0);
                    }

                    if (RayPacketIntersectAABB
                        <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS, true>
                        (packet,hitID,min_x,max_x,min_y,max_y,min_z,max_z) == 0) // early hit
                    {
                        if (!MULTIPLE_ORIGINS)
                            if (RayIntervalIntersectAABB
                                <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>
                                (min_rcp,max_rcp,min_x,max_x,min_y,max_y,min_z,max_z) == 0)
                            {
                                BVH_STAT_COLLECTOR(BVHStatCollector::global.numIntervalPruningTests++);
                                goto start_traverse_same_signs;
                            }


                            unsigned int intersectRays = 0;
                            while (++hitID<N)
                                if (RayPacketIntersectAABB
                                    <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS, true>
                                    (packet,hitID,min_x,max_x,min_y,max_y,min_z,max_z))
                                    break;
                            if (hitID == N)
                            { goto start_traverse_same_signs; }
                    }
                    BVH_STAT_COLLECTOR(else BVHStatCollector::global.numFirstHitTests++);

                    if (entry.isLeaf()) break;

                    const unsigned int axis = entry.axis();
                    const unsigned int children = entry.children();
                    const unsigned int rayDir = raySigns[axis] & 1;
                    const unsigned int firstChild = children + rayDir;
                    const unsigned int lastChild  = children + (1^rayDir);
                    index  = firstChild;

                    sptr->bvhIndex  = lastChild;
                    sptr->fastHitID = hitID;
                    sptr++;
                }
                BVH_STAT_COLLECTOR(BVHStatCollector::global.numLeafIntersections++);
                const AABB &entry = bvh[index];
                const unsigned int items = entry.items();
                int *start = (int*)item + entry.itemOffset();
                for (unsigned int i=0;i<items;i++)
                {
                    BVH_STAT_COLLECTOR(BVHStatCollector::global.numPrimitiveIntersections++);
                    const int triID = start[i];
                    const RTVec3f &v0 = mesh.getTriangleVertex(triID,0);
                    const RTVec3f &v1 = mesh.getTriangleVertex(triID,1);
                    const RTVec3f &v2 = mesh.getTriangleVertex(triID,2);
                    IntersectTriangleMoellerTrumbore
                        <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>
                        (v0,v1,v2,triID,packet,hitID,N);
                }
            }
    }

};

#endif
