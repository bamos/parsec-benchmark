
/*! \file BVH.hxx defines the most typical kind of BVH - binary tree,
AABBs. Put specialized BVH variants (N-way branching factor, speres
or non-AA-box BVHs etcpp) into separate files, and give reasonable
names ... */

#ifndef RTTL_BVH_HXX
#define RTTL_BVH_HXX

#include "../common/RTInclude.hxx"
#include "../common/RTBox.hxx"
#include "../common/RTRay.hxx"
#include "../common/RTIntervalArith.hxx"
#include "../Triangle/Triangle.hxx"
#include "Builder/Builder.hxx"
#include "../Mesh/Mesh.hxx"

#define MAX_BVH_STACK_DEPTH 64

// see config.h
#if DO_STATS_BVH
#  define BVH_STAT_COLLECTOR(x) x
#else
#  define BVH_STAT_COLLECTOR(x) 
#endif

namespace RTTL {

    // just a few forward definitions ...
    class AABB;
    class BVHBuilder;


    /*! just the data container for a plain BVH. might lateron want to
    use templated base classes for various different BVH
    representations (for how nodes are stored internally (precision,
    incremental, ...), for how primitives are passed to the builder,
    whatever...) --- but for starters, this'll do just fine ...

    note that this is simply a *container*, where leaves store lists
    of primitive IDs -- we do not make any assumptions about what
    primitives there are, how they are intersected, or anything alike
    */
    struct BVH
    {
        /*! array of BVH nodes. root node is node '0'. */
        AABB *node;
        /*! array containing the BVH's leaves' item lists, one after
        another. BVH leaves point into this array */
        int  *item;
        /*! number of primitives we have already reserved memory for */
        int reservedSize;

        /*! constructor -- just initialize memory, everything else will
        later on be done by the builder or traverser classes */
        BVH() : node(NULL), item(NULL), reservedSize(0), builder(NULL)
        {};

        /*! use builder of given type for this BVH. if builderType is
        NULL, use default builder type */
        void setBuilder(const char *builderType);


        /*! call the current builder to build a BVH */
        virtual void build(const RTBoxSSE &sceneAABB,const RTBoxSSE &centroidAABB);
        /*! the builder for this particular BVH */
        BVHBuilder *builder;

        /*! allocate memory, nothing else */
        void reserve(int numPrimitives);


        // -------------------------------------------------------
        /* this is the interface for non-hierarchical BVH builders, that
        when they start buildeing a bvh first query the number of
        primitives all all bounding boxes ... */
        // -------------------------------------------------------

        /*! for nasty builders that need access to all primitive bounds ... */
        virtual AABB *getAllPrimitiveBounds() = 0;
        /*! in case the BVH had had to allocate memory when returning the
        primitive bounds */
        virtual void doneWithAllPrimitiveBounds(AABB *memory) = 0;
        /*! for nasty BVH builders that want to query all primitives
        before they do anything else .. */
        virtual int numPrimitives() = 0;
    };

    struct AABBListBVH : public BVH
    {
        AABB *primBounds;
        int primitives;

        AABBListBVH(AABB *primBounds, int primitives)
            : BVH(), primBounds(primBounds), primitives(primitives)
        {};
        /*! nothing to do -- we have had everything pre-allocated, anyway */
        virtual void doneWithAllPrimitiveBounds(AABB * /*ignore*/)
        { /* nothing to do */ };

        /*! not a lot to do -- we have had everything pre-allocated,
        anyway, so just return it */
        virtual AABB *getAllPrimitiveBounds()
        { return primBounds; }
        virtual int numPrimitives()
        { return primitives; }
    };

    class BVHStatCollector
    {
    public:

        static BVHStatCollector global;

        int numPackets;
        int numTraversalSteps;
        int numLeafIntersections;
        int numPrimitiveIntersections;
        int numFirstHitTests;
        int numIntervalPruningTests;
        int numLazyBuildSteps;

        _INLINE void reset()
        {
            numPackets = 0;
            numTraversalSteps = 0;
            numLeafIntersections = 0;
            numPrimitiveIntersections = 0;
            numFirstHitTests = 0;
            numIntervalPruningTests = 0;
            numLazyBuildSteps = 0;
        }

        _INLINE void print()
        {
            DBG_PRINT(numPackets);
            DBG_PRINT(numTraversalSteps/(float)numPackets);
            DBG_PRINT(numLeafIntersections/(float)numPackets);
            DBG_PRINT(numPrimitiveIntersections/(float)numPackets);
            cout << "numFirstHitTests        " << 100.0f*numFirstHitTests/numTraversalSteps << "%" << endl;
            cout << "numIntervalPruningTests " << 100.0f*numIntervalPruningTests/numTraversalSteps << "%" << endl;
            DBG_PRINT(numLazyBuildSteps);
        }

    };

    struct BVHExtData // 32bits
    {
        union {
            struct {
                unsigned char axis;
                unsigned char sign;
                unsigned short items;
            };
            union {
                unsigned int children;
                unsigned int t;
            };
        };
        BVHExtData() {}

        BVHExtData(unsigned char _axis, unsigned char _sign, short _items)
        {
            axis = _axis;
            sign = _sign;
            items = _items;
        }

        BVHExtData(unsigned int _t)
        {
            t = _t;
        }
    };

    class AABB: public RTBoxSSE  {
    public:

        AABB() {
        }

      AABB(const sse_f& _min, const sse_f& _max)
      {
    min_f() = _min;
    max_f() = _max;
      }

        AABB(const RTBoxSSE &b) {
      min_f() = b.min_f();
      max_f() = b.max_f();
        }

        _INLINE void createLeaf(const unsigned int offset, const unsigned int entries)
        {
            children() = offset | (unsigned int)(1<<31);
            items() = entries;
            axis() = 0;
            sign() = 0;
        }

        _INLINE void createNode(const unsigned int index, const unsigned char dim)
        {
            children() = index;
            axis() = dim;
            sign() = 0;
            items() = 0;
        }

        _INLINE void createLazyNode(const unsigned int offset, const unsigned int entries)
        {
            children() = offset | (unsigned int)(1<<31);
            extMax() = entries |  (unsigned int)(1<<31);
        }

//         _INLINE sse_f distance() const
//         {
//             return _mm_sub_ps(m_max[0],m_min[0]);
//         }

        _INLINE void setMin(int i, float f)   { m_min.entry<float>(i) = f; };
        _INLINE void setMax(int i, float f)   { m_max.entry<float>(i) = f; };

        _INLINE unsigned char& axis() const { return ((BVHExtData*)&m_max)[3].axis; }
        _INLINE unsigned char& sign() const { return ((BVHExtData*)&m_max)[3].sign; }
        _INLINE unsigned short& items() const { return ((BVHExtData*)&m_max)[3].items; }
        _INLINE unsigned int& children() const { return ((BVHExtData*)&m_min)[3].children; }
        _INLINE unsigned int itemOffset() const { return ((BVHExtData*)&m_min)[3].children & ~(unsigned int)(1<<31); }
        _INLINE bool isLeaf() const { return ((BVHExtData*)&m_min)[3].children & (unsigned int)(1<<31); }
        _INLINE bool isLazy() const { return extMax() & (unsigned int)(1<<31);}
        _INLINE unsigned int lazyItems() const { return extMax() & ~(unsigned int)(1<<31); }


        _INLINE unsigned int& extMin() const { return ((BVHExtData*)&m_min)[3].t; }
        _INLINE unsigned int& extMax() const { return ((BVHExtData*)&m_max)[3].t; }
    };

    class CentroidDiffAABB : public RTBoxSSE  {
    public:

        // min -> centroid
        // max -> diff

        CentroidDiffAABB() {
        }

      sse_f centroid() const { return min_f(); }
      float centroid(const int i) const { return ((float*)&m_min)[i]; }
      sse_f diff() const { return max_f(); }

        CentroidDiffAABB(const RTBoxSSE &b) {
      max_f() = b.diameter() * 0.5f;
          min_f() = b.min_f() + max_f();
        }

        AABB convert() const
        {
            AABB aabb;
            aabb.m_min = m_min - m_max;
            aabb.m_max = m_min + m_max;
            return aabb;
        }

    };


    template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS,int SAME_SIGNS>
    _INLINE unsigned int RayPacketIntersectAABB(const RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> &packet,
        const int i,
        const sse_f& bMinX,
        const sse_f& bMaxX,
        const sse_f& bMinY,
        const sse_f& bMaxY,
        const sse_f& bMinZ,
        const sse_f& bMaxZ)
    {
        const sse_f clipMinX = (MULTIPLE_ORIGINS ? (bMinX - packet.originX(i)) : bMinX) * packet.reciprocalX(i);
        const sse_f clipMaxX = (MULTIPLE_ORIGINS ? (bMaxX - packet.originX(i)) : bMaxX) * packet.reciprocalX(i);
        const sse_f clipMinY = (MULTIPLE_ORIGINS ? (bMinY - packet.originY(i)) : bMinY) * packet.reciprocalY(i);
        const sse_f clipMaxY = (MULTIPLE_ORIGINS ? (bMaxY - packet.originY(i)) : bMaxY) * packet.reciprocalY(i);
        const sse_f clipMinZ = (MULTIPLE_ORIGINS ? (bMinZ - packet.originZ(i)) : bMinZ) * packet.reciprocalZ(i);
        const sse_f clipMaxZ = (MULTIPLE_ORIGINS ? (bMaxZ - packet.originZ(i)) : bMaxZ) * packet.reciprocalZ(i);

        if (SAME_SIGNS)
        {
            const sse_f near4 = _mm_max_ps(_mm_max_ps(clipMinX,clipMinY),clipMinZ);
            const sse_f far4  = _mm_min_ps(_mm_min_ps(clipMaxX,clipMaxY),clipMaxZ);
            return  _mm_movemask_ps(_mm_cmple_ps(_mm_max_ps(packet.minDistance(i),near4),
                _mm_min_ps(packet.maxDistance(i),far4)));
        }
        else
        {
            const sse_f near4 = _mm_max_ps(_mm_max_ps(_mm_min_ps(clipMinX,clipMaxX),
                _mm_min_ps(clipMinY,clipMaxY)),
                _mm_min_ps(clipMinZ,clipMaxZ));
            const sse_f far4  = _mm_min_ps(_mm_min_ps(_mm_max_ps(clipMinX,clipMaxX),
                _mm_max_ps(clipMinY,clipMaxY)),
                _mm_max_ps(clipMinZ,clipMaxZ));
            return  _mm_movemask_ps(_mm_cmple_ps(_mm_max_ps(packet.minDistance(i),near4),
                _mm_min_ps(packet.maxDistance(i),far4)));
        }

    }

    /* requires that all ray directions have the same sign (per coordinate) */
    template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS>
    _INLINE unsigned int RayIntervalIntersectAABB(const sse_f *const min_rcp,
        const sse_f *const max_rcp,
        const sse_f& bMinX,
        const sse_f& bMaxX,
        const sse_f& bMinY,
        const sse_f& bMaxY,
        const sse_f& bMinZ,
        const sse_f& bMaxZ)
    {
        if (!MULTIPLE_ORIGINS)
        {
            // uuuuuuhohhhh..... bad naming of variables -- you're
            // assuming the origin has been subtracted from min/max
            // already ...
            const sse_f nearX = max_rcp[0] * bMinX;
            const sse_f nearY = max_rcp[1] * bMinY;
            const sse_f nearZ = max_rcp[2] * bMinZ;
            const sse_f nearAll  = max(nearX,max(nearY,nearZ));

            const sse_f farX = min_rcp[0] * bMaxX;
            const sse_f farY = min_rcp[1] * bMaxY;
            const sse_f farZ = min_rcp[2] * bMaxZ;
            const sse_f farAll  = min(farX,min(farY,farZ));

            return _mm_movemask_ps(_mm_cmple_ps(nearAll,farAll));
        }
        else
        {
            cout << "NOT IMPLEMENTED" << endl;
            exit(1);
        }
    }

    /* requires that all ray directions have the same sign (per coordinate) */
    template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS>
    _INLINE unsigned int RayIntervalIntersectAABB(const sse_f& min_rcp,
        const sse_f& max_rcp,
        const sse_f& min_origin,
        const sse_f& max_origin,
        const sse_f& min_aabb,
        const sse_f& max_aabb)
    {
        // todo: include distance
        const RTIntervalVec aabb(min_aabb,max_aabb);
        const RTIntervalVec origin(min_origin,max_origin);
        const RTIntervalVec rcp(min_rcp,max_rcp);
        const RTIntervalVec aabb_origin = aabb-origin;
        const RTIntervalVec t = aabb_origin*rcp;
        return !t.empty();
    }

    template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS, class Mesh>
   /* _INLINE*/ void TraverseBVH(RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> &packet,
                 const AABB *const bvh,
                 const int *const item,
                 const Mesh &mesh)
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
                    const AABB &entry = bvh[index];
                    const sse_f m_min = entry.min_f();
                    const sse_f m_max = entry.max_f();
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
                    mesh.template intersectPrimitive<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>(packet,triID,hitID,N);
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

            // create two sse_f data types for storing min/max of inverse directions and min/max of origins in xyz format
            sse_f min_rcp_direction, max_rcp_direction, min_origin, max_origin;
            if (MULTIPLE_ORIGINS)
            {
                min_rcp_direction = setHorizontalMin3f(packet.reciprocalMin(0),packet.reciprocalMin(1),packet.reciprocalMin(2));
                max_rcp_direction = setHorizontalMax3f(packet.reciprocalMax(0),packet.reciprocalMax(1),packet.reciprocalMax(2));
                sse_f minX,maxX,minY,maxY,minZ,maxZ;
                minX = maxX = packet.originX(0);
                minY = maxY = packet.originY(0);
                minZ = maxZ = packet.originZ(0);
                for (int i=1;i<N;i++)
                {
                  minX = min(minX,packet.originX(i));
                  maxX = max(maxX,packet.originX(i));
                  minY = min(minY,packet.originY(i));
                  maxY = max(maxY,packet.originY(i));
                  minZ = min(minZ,packet.originZ(i));
                  maxZ = max(maxZ,packet.originZ(i));
                }
                min_origin = setHorizontalMin3f(minX,minY,minZ);
                max_origin = setHorizontalMax3f(maxX,maxY,maxZ);
            }

            while(1)
            {
start_traverse_same_signs:
                if (__builtin_expect(sptr == bvhStack,0)) return;
                sptr--;

                unsigned int index = sptr->bvhIndex;
                hitID = sptr->fastHitID;

                while(1) {
                    BVH_STAT_COLLECTOR(BVHStatCollector::global.numTraversalSteps++);
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
                        {
                            if (RayIntervalIntersectAABB
                                <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>
                                (min_rcp,max_rcp,min_x,max_x,min_y,max_y,min_z,max_z) == 0)
                            {
                                BVH_STAT_COLLECTOR(BVHStatCollector::global.numIntervalPruningTests++);
                                goto start_traverse_same_signs;
                            }
                        }
                        else
                        {
                            if (RayIntervalIntersectAABB
                                <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>
                                (min_rcp_direction,max_rcp_direction,min_origin,max_origin,t[0],t[1]) == 0)
                            {
                                BVH_STAT_COLLECTOR(BVHStatCollector::global.numIntervalPruningTests++);
                                goto start_traverse_same_signs;
                            }
                        }


                        unsigned int intersectRays = 0;
                        while (++hitID<N)
                            if (RayPacketIntersectAABB
                                <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS, true>
                                (packet,hitID,min_x,max_x,min_y,max_y,min_z,max_z))
                                break;
                        if (hitID == N)
                            goto start_traverse_same_signs;
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
//#define LEAVES_KILL_RAYS
#ifdef LEAVES_KILL_RAYS
                int rayID[N];
                int rayIDs = 0;
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
                for (int i=hitID;i<N;i++) {
                    if (RayPacketIntersectAABB
                        <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS, true>
                        (packet,i,min_x,max_x,min_y,max_y,min_z,max_z))
                        rayID[rayIDs++] = i;
                }
                BVH_STAT_COLLECTOR(BVHStatCollector::global.numLeafIntersections++);
                const AABB &entry = bvh[index];
                const unsigned int items = entry.items();
                int *start = (int*)item + entry.itemOffset();
                for (unsigned int i=0;i<items;i++)
                {
                    BVH_STAT_COLLECTOR(BVHStatCollector::global.numPrimitiveIntersections++);
                    const int triID = start[i];
                    mesh.intersectPrimitive<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>(packet,triID,hitID,N);
                }
#else
                BVH_STAT_COLLECTOR(BVHStatCollector::global.numLeafIntersections++);
                const AABB &entry = bvh[index];
                const unsigned int items = entry.items();
                int *start = (int*)item + entry.itemOffset();
                for (unsigned int i=0;i<items;i++)
                {
                    BVH_STAT_COLLECTOR(BVHStatCollector::global.numPrimitiveIntersections++);
                    const int triID = start[i];
                    mesh.template intersectPrimitive<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>(packet,triID,hitID,N);
                }
#endif
            }
    }

    template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS >
   /* _INLINE*/ void TraverseBVH_with_StandardMesh(RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> &packet,
                 const AABB *const bvh,
                 const int *const item,
                 const RTTL::StandardTriangleMesh &mesh)
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
                    const AABB &entry = bvh[index];
                    const sse_f m_min = entry.min_f();
                    const sse_f m_max = entry.max_f();
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
                    mesh.intersectPrimitive<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>(packet,triID,hitID,N);
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

            // create two sse_f data types for storing min/max of inverse directions and min/max of origins in xyz format
            sse_f min_rcp_direction, max_rcp_direction, min_origin, max_origin;
            if (MULTIPLE_ORIGINS)
            {
                min_rcp_direction = setHorizontalMin3f(packet.reciprocalMin(0),packet.reciprocalMin(1),packet.reciprocalMin(2));
                max_rcp_direction = setHorizontalMax3f(packet.reciprocalMax(0),packet.reciprocalMax(1),packet.reciprocalMax(2));
                sse_f minX,maxX,minY,maxY,minZ,maxZ;
                minX = maxX = packet.originX(0);
                minY = maxY = packet.originY(0);
                minZ = maxZ = packet.originZ(0);
                for (int i=1;i<N;i++)
                {
                  minX = min(minX,packet.originX(i));
                  maxX = max(maxX,packet.originX(i));
                  minY = min(minY,packet.originY(i));
                  maxY = max(maxY,packet.originY(i));
                  minZ = min(minZ,packet.originZ(i));
                  maxZ = max(maxZ,packet.originZ(i));
                }
                min_origin = setHorizontalMin3f(minX,minY,minZ);
                max_origin = setHorizontalMax3f(maxX,maxY,maxZ);
            }

            while(1)
            {
start_traverse_same_signs:
                if (__builtin_expect(sptr == bvhStack,0)) return;
                sptr--;

                unsigned int index = sptr->bvhIndex;
                hitID = sptr->fastHitID;

                while(1) {
                    BVH_STAT_COLLECTOR(BVHStatCollector::global.numTraversalSteps++);
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
                        {
                            if (RayIntervalIntersectAABB
                                <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>
                                (min_rcp,max_rcp,min_x,max_x,min_y,max_y,min_z,max_z) == 0)
                            {
                                BVH_STAT_COLLECTOR(BVHStatCollector::global.numIntervalPruningTests++);
                                goto start_traverse_same_signs;
                            }
                        }
                        else
                        {
                            if (RayIntervalIntersectAABB
                                <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>
                                (min_rcp_direction,max_rcp_direction,min_origin,max_origin,t[0],t[1]) == 0)
                            {
                                BVH_STAT_COLLECTOR(BVHStatCollector::global.numIntervalPruningTests++);
                                goto start_traverse_same_signs;
                            }
                        }


                        //unsigned int intersectRays = 0;
                        while (++hitID<N)
                            if (RayPacketIntersectAABB
                                <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS, true>
                                (packet,hitID,min_x,max_x,min_y,max_y,min_z,max_z))
                                break;
                        if (hitID == N)
                            goto start_traverse_same_signs;
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
//#define LEAVES_KILL_RAYS
#ifdef LEAVES_KILL_RAYS
                int rayID[N];
                int rayIDs = 0;
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
                for (int i=hitID;i<N;i++) {
                    if (RayPacketIntersectAABB
                        <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS, true>
                        (packet,i,min_x,max_x,min_y,max_y,min_z,max_z))
                        rayID[rayIDs++] = i;
                }
                BVH_STAT_COLLECTOR(BVHStatCollector::global.numLeafIntersections++);
                const AABB &entry = bvh[index];
                const unsigned int items = entry.items();
                int *start = (int*)item + entry.itemOffset();
                for (unsigned int i=0;i<items;i++)
                {
                    BVH_STAT_COLLECTOR(BVHStatCollector::global.numPrimitiveIntersections++);
                    const int triID = start[i];
                    mesh.intersectPrimitive<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>(packet,triID,hitID,N);
                }
#else
                BVH_STAT_COLLECTOR(BVHStatCollector::global.numLeafIntersections++);
                const AABB &entry = bvh[index];
                const unsigned int items = entry.items();
                int *start = (int*)item + entry.itemOffset();
                for (unsigned int i=0;i<items;i++)
                {
                    BVH_STAT_COLLECTOR(BVHStatCollector::global.numPrimitiveIntersections++);
                    const int triID = start[i];
                    mesh.intersectPrimitive<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>(packet,triID,hitID,N);
                }
#endif
            }
    }


    template <int N, int LAYOUT, int MULTIPLE_ORIGINS, int SHADOW_RAYS >
   /* _INLINE*/ void TraverseBVH_with_DirectMesh(RayPacket<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS> &packet,
                 const AABB *const bvh,
                 const int *const item,
                 const RTTL::DirectedEdgeMesh &mesh)
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
                    const AABB &entry = bvh[index];
                    const sse_f m_min = entry.min_f();
                    const sse_f m_max = entry.max_f();
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
                    mesh.intersectPrimitive<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>(packet,triID,hitID,N);
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

            // create two sse_f data types for storing min/max of inverse directions and min/max of origins in xyz format
            sse_f min_rcp_direction, max_rcp_direction, min_origin, max_origin;
            if (MULTIPLE_ORIGINS)
            {
                min_rcp_direction = setHorizontalMin3f(packet.reciprocalMin(0),packet.reciprocalMin(1),packet.reciprocalMin(2));
                max_rcp_direction = setHorizontalMax3f(packet.reciprocalMax(0),packet.reciprocalMax(1),packet.reciprocalMax(2));
                sse_f minX,maxX,minY,maxY,minZ,maxZ;
                minX = maxX = packet.originX(0);
                minY = maxY = packet.originY(0);
                minZ = maxZ = packet.originZ(0);
                for (int i=1;i<N;i++)
                {
                  minX = min(minX,packet.originX(i));
                  maxX = max(maxX,packet.originX(i));
                  minY = min(minY,packet.originY(i));
                  maxY = max(maxY,packet.originY(i));
                  minZ = min(minZ,packet.originZ(i));
                  maxZ = max(maxZ,packet.originZ(i));
                }
                min_origin = setHorizontalMin3f(minX,minY,minZ);
                max_origin = setHorizontalMax3f(maxX,maxY,maxZ);
            }

            while(1)
            {
start_traverse_same_signs:
                if (__builtin_expect(sptr == bvhStack,0)) return;
                sptr--;

                unsigned int index = sptr->bvhIndex;
                hitID = sptr->fastHitID;

                while(1) {
                    BVH_STAT_COLLECTOR(BVHStatCollector::global.numTraversalSteps++);
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
                        {
                            if (RayIntervalIntersectAABB
                                <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>
                                (min_rcp,max_rcp,min_x,max_x,min_y,max_y,min_z,max_z) == 0)
                            {
                                BVH_STAT_COLLECTOR(BVHStatCollector::global.numIntervalPruningTests++);
                                goto start_traverse_same_signs;
                            }
                        }
                        else
                        {
                            if (RayIntervalIntersectAABB
                                <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>
                                (min_rcp_direction,max_rcp_direction,min_origin,max_origin,t[0],t[1]) == 0)
                            {
                                BVH_STAT_COLLECTOR(BVHStatCollector::global.numIntervalPruningTests++);
                                goto start_traverse_same_signs;
                            }
                        }


                        //unsigned int intersectRays = 0;
                        while (++hitID<N)
                            if (RayPacketIntersectAABB
                                <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS, true>
                                (packet,hitID,min_x,max_x,min_y,max_y,min_z,max_z))
                                break;
                        if (hitID == N)
                            goto start_traverse_same_signs;
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
//#define LEAVES_KILL_RAYS
#ifdef LEAVES_KILL_RAYS
                int rayID[N];
                int rayIDs = 0;
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
                for (int i=hitID;i<N;i++) {
                    if (RayPacketIntersectAABB
                        <N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS, true>
                        (packet,i,min_x,max_x,min_y,max_y,min_z,max_z))
                        rayID[rayIDs++] = i;
                }
                BVH_STAT_COLLECTOR(BVHStatCollector::global.numLeafIntersections++);
                const AABB &entry = bvh[index];
                const unsigned int items = entry.items();
                int *start = (int*)item + entry.itemOffset();
                for (unsigned int i=0;i<items;i++)
                {
                    BVH_STAT_COLLECTOR(BVHStatCollector::global.numPrimitiveIntersections++);
                    const int triID = start[i];
                    mesh.intersectPrimitive<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>(packet,triID,hitID,N);
                }
#else
                BVH_STAT_COLLECTOR(BVHStatCollector::global.numLeafIntersections++);
                const AABB &entry = bvh[index];
                const unsigned int items = entry.items();
                int *start = (int*)item + entry.itemOffset();
                for (unsigned int i=0;i<items;i++)
                {
                    BVH_STAT_COLLECTOR(BVHStatCollector::global.numPrimitiveIntersections++);
                    const int triID = start[i];
                    mesh.intersectPrimitive<N, LAYOUT, MULTIPLE_ORIGINS, SHADOW_RAYS>(packet,triID,hitID,N);
                }
#endif
            }
    }




};

#endif
