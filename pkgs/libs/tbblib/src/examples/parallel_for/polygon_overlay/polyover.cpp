/*
    Copyright 2005-2010 Intel Corporation.  All Rights Reserved.

    This file is part of Threading Building Blocks.

    Threading Building Blocks is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    Threading Building Blocks is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Threading Building Blocks; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    As a special exception, you may use this file as part of a free software
    library without restriction.  Specifically, if other files instantiate
    templates or use macros or inline functions from this file, or you compile
    this file and link it with other files to produce an executable, this
    file does not by itself cause the resulting executable to be covered by
    the GNU General Public License.  This exception does not however
    invalidate any other reasons why the executable file might be covered by
    the GNU General Public License.
*/

// Polygon overlay
//
#include <iostream>
#include <algorithm>
#include <string.h>
#include <cstdlib>
#include <assert.h>
#include "tbb/tick_count.h"
#include "tbb/blocked_range.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/parallel_for.h"
#include "tbb/mutex.h"
#include "tbb/spin_mutex.h"
#include "polyover.h"
#include "polymain.h"
#include "pover_video.h"

using namespace std;

/*!
* @brief intersects a polygon with a map, adding any results to output map
*
* @param[out] resultMap output map (must be allocated)
* @param[in] polygon to be intersected
* @param[in] map intersected against
* @param[in] lock to use when adding output polygons to result map
*
*/
void OverlayOnePolygonWithMap(Polygon_map_t *resultMap, RPolygon *myPoly, Polygon_map_t *map2, tbb::spin_mutex *rMutex) {
    int r1, g1, b1, r2, g2, b2;
    int myr=0;
    int myg=0;
    int myb=0;
    int p1Area = myPoly->area();
    for(unsigned int j=1; (j < map2->size()) && (p1Area > 0); j++) {
        RPolygon *p2 = (*map2)[j];
        RPolygon *pnew;
        int newxMin, newxMax, newyMin, newyMax;
        myPoly->getColor(&r1, &g1, &b1);
        if(PolygonsOverlap(myPoly, p2, newxMin, newyMin, newxMax, newyMax)) {
            p2->getColor(&r2, &g2, &b2);
            myr = r1 + r2;
            myg = g1 + g2;
            myb = b1 + b2;
            pnew = RPolygon::alloc_RPolygon(newxMin, newyMin, newxMax, newyMax, myr, myg, myb);
            p1Area -= pnew->area(); // when all the area of the polygon is accounted for, we can quit.
            if(rMutex) {
                tbb::spin_mutex::scoped_lock lock(*rMutex);
#if _DEBUG
                pnew->print(int(resultMap->size()));
#endif
                resultMap->push_back(pnew);
            }
            else {
#ifdef _DEBUG
                pnew->print(int(resultMap->size()));
#endif
                resultMap->push_back(pnew);
            }
        }
    }
}

/*!
* @brief Serial version of polygon overlay
* @param[out] output map
* @param[in]  first map (map that individual polygons are taken from)
* @param[in]  second map (map passed to OverlayOnePolygonWithMap)
*/
void SerialOverlayMaps(Polygon_map_t **resultMap, Polygon_map_t *map1, Polygon_map_t *map2) {
    cout << "SerialOverlayMaps called" << std::endl;
    *resultMap = new Polygon_map_t;

    RPolygon *p0 = (*map1)[0];
    int mapxSize, mapySize, ignore1, ignore2;
    p0->get(&ignore1, &ignore2, &mapxSize, &mapySize);
    (*resultMap)->reserve(mapxSize*mapySize); // can't be any bigger than this
    // push the map size as the first polygon,
    p0 = RPolygon::alloc_RPolygon(0,0,mapxSize, mapySize);
    (*resultMap)->push_back(p0);
    for(unsigned int i=1; i < map1->size(); i++) {
        RPolygon *p1 = (*map1)[i];
        OverlayOnePolygonWithMap(*resultMap, p1, map2, NULL);
    }
}

/*!
* @class ApplyOverlay
* @brief Simple version of parallel overlay (make parallel on polygons in map1)
*/
class ApplyOverlay {
    Polygon_map_t *m_map1, *m_map2, *m_resultMap;
    tbb::spin_mutex *m_rMutex;
public:
    /*!
    * @brief functor to apply
    * @param[in] r range of polygons to intersect from map1
    */
    void operator()( const tbb::blocked_range<int> & r) const {
        PRINT_DEBUG("From " << r.begin() << " to " << r.end());
        for(int i=r.begin(); i != r.end(); i++) {
            RPolygon *myPoly = (*m_map1)[i];
            OverlayOnePolygonWithMap(m_resultMap, myPoly, m_map2, m_rMutex);
        }
    }
    ApplyOverlay(Polygon_map_t *resultMap, Polygon_map_t *map1, Polygon_map_t *map2, tbb::spin_mutex *rmutex) :
    m_resultMap(resultMap), m_map1(map1), m_map2(map2), m_rMutex(rmutex) {}
};

/*!
* @brief apply the parallel algorithm
* @param[out] result_map generated map
* @param[in] polymap1 first map to be applied (algorithm is parallel on this map)
* @param[in] polymap2 second map.
*/
void NaiveParallelOverlay(Polygon_map_t *&result_map, Polygon_map_t &polymap1, Polygon_map_t &polymap2) {
// -----------------------------------
    bool automatic_threadcount = false;

    if(gThreadsLow == THREADS_UNSET || gThreadsLow == tbb::task_scheduler_init::automatic) {
        gThreadsLow = gThreadsHigh = tbb::task_scheduler_init::automatic;
        automatic_threadcount = true;
    }
    result_map = new Polygon_map_t;

    RPolygon *p0 = polymap1[0];
    int mapxSize, mapySize, ignore1, ignore2;
    p0->get(&ignore1, &ignore2, &mapxSize, &mapySize);
    result_map->reserve(mapxSize*mapySize); // can't be any bigger than this
    // push the map size as the first polygon,
    tbb::spin_mutex *resultMutex = new tbb::spin_mutex();
    int grain_size = gGrainSize;

    for(int nthreads = gThreadsLow; nthreads <= gThreadsHigh; nthreads++) {
        tbb::task_scheduler_init init(nthreads);
        if(gIsGraphicalVersion) {
            RPolygon *xp = RPolygon::alloc_RPolygon(0, 0, gMapXSize-1, gMapYSize-1, 0, 0, 0);  // Clear the output space
            RPolygon::free_RPolygon( xp );
        }
        // put size polygon in result map
        p0 = RPolygon::alloc_RPolygon(0,0,mapxSize, mapySize);
        result_map->push_back(p0);

        tbb::tick_count t0 = tbb::tick_count::now();
        tbb::parallel_for (tbb::blocked_range<int>(1,(int)(polymap1.size()),grain_size), ApplyOverlay(result_map, &polymap1, &polymap2, resultMutex));
        tbb::tick_count t1 = tbb::tick_count::now();

        double naiveParallelTime = (t1-t0).seconds() * 1000;
        cout << "Naive parallel with spin lock and ";
        if(automatic_threadcount) cout << "automatic";
        else cout << nthreads;
        cout << ((nthreads == 1) ? " thread" : " threads");
        cout << " took " << naiveParallelTime << " msec : speedup over serial " << (gSerialTime / naiveParallelTime) << std::endl;
        if(gCsvFile.is_open()) {
            gCsvFile << "," << naiveParallelTime;
        }
#if _DEBUG
        CheckPolygonMap(result_map);
        ComparePolygonMaps(result_map, gResultMap);
#endif
        for(int i=0; i<int(result_map->size());i++) {
            RPolygon::free_RPolygon(result_map->at(i));
        }
        result_map->clear();
    }
    delete resultMutex;
    if(gCsvFile.is_open()) {
        gCsvFile << std::endl;
    }
// -----------------------------------
}

/*!
* @class ApplySplitOverlay
* @brief parallel by columnar strip
*/

class ApplySplitOverlay {
    Polygon_map_t *m_map1, *m_map2, *m_resultMap;
    tbb::spin_mutex *m_rMutex;
public:
    /*!
    * @brief functor for columnar parallel version
    * @param[in] r range of map to be operated on
    */
    void operator()(const tbb::blocked_range<int> & r) const {
#ifdef _DEBUG
        // if we are debugging, serialize the method.  That way we can
        // see what is happening in each strip without the interleaving
        // confusing things.
        tbb::spin_mutex::scoped_lock lock(*m_rMutex);
        cout << unitbuf << "From " << r.begin() << " to " << r.end()-1 << std::endl;
#endif
        // instead of handing out subsets of polygons from map1 to intersect
        // with the polygons in map2, we are handed a strip of the map from
        // [(r.begin(),0)-(r.end()-1,yMapSize)].
        //
        // make a polygon with those values, and intersect with all the polygons
        // in map1 and map2, creating flagged polygon lists fmap1 and fmap2.
        // There are four possiblities:
        //
        //   1) a polygon is contained entirely within the strip.  We just
        //      add the polygon to our flagged map.
        //   2) the polygon will be partly contained in our strip, and partly
        //      in the strip to our right (higher x values).  Add the polygon
        //      to our flagged map.
        //   3) the polygon is partly contained in our map, and partly in the
        //      strip to our left.  Add the polygon to our map, but flag it as
        //      a duplicate.
        //   4) the polygons do not intersect. Don't add to flagged map.
        //

        // get yMapSize
        int r1, g1, b1, r2, g2, b2;
        int myr=-1;
        int myg=-1;
        int myb=-1;
        int i1, i2, i3, yMapSize;
        m_map1->at(0)->get(&i1, &i2, &i3, &yMapSize);
        RPolygon *slicePolygon = RPolygon::alloc_RPolygon(r.begin(), 0, r.end() - 1, yMapSize);

        Flagged_map_t *fmap1, *fmap2;
        fmap1 = new std::vector<RPolygon_flagged>;
        fmap1->reserve(m_map1->size());
        fmap2 = new Flagged_map_t;
        fmap2->reserve(m_map2->size());

        PRINT_DEBUG(std::endl << "Map1 -------------------");
        for(unsigned int i=1; i<m_map1->size(); i++) {
            int xl, yl, xh, yh;
            RPolygon *px = m_map1->at(i);
            if(PolygonsOverlap(slicePolygon, px, xl, yl, xh, yh)) {
                bool is_duplicate = false;
                int pxl, pyl, pxh, pyh;
                int indx = (int)(fmap1->size());
                fmap1->resize(indx+1);
                fmap1->at(indx).setp(px);
                px->get(&pxl, &pyl, &pxh, &pyh);
                if(pxl < xl) {
                    is_duplicate = true;
                }
                //fmap1->at(indx).setp(px);
                fmap1->at(indx).setDuplicate(is_duplicate);
                PRINT_DEBUG(" Polygon " << *px << " is in map, is_duplicate=" << is_duplicate);

            }
        }

        PRINT_DEBUG(std::endl << "Map2 -------------------");

        for(unsigned int i=1; i<m_map2->size(); i++) {
            int xl, yl, xh, yh;
            RPolygon *px = m_map2->at(i);

            if(PolygonsOverlap(slicePolygon, px, xl, yl, xh, yh)) {
                bool is_duplicate = false;
                int pxl, pyl, pxh, pyh;
                int indx = (int)(fmap2->size());
                fmap2->resize(indx+1);
                fmap2->at(indx).setp(px);
                px->get(&pxl, &pyl, &pxh, &pyh);
                if(pxl < xl) {
                    is_duplicate = true;
                }
                fmap2->at(indx).setDuplicate(is_duplicate);
                PRINT_DEBUG(" Polygon " << *px << " is in map, is_duplicate=" << is_duplicate);
            }
        }

        // When intersecting polygons from fmap1 and fmap2, if BOTH are flagged
        // as duplicate, don't add the result to the output map.  We can still
        // intersect them, because we are keeping track of how much of the polygon
        // is left over from intersecting, and quitting when the polygon is
        // used up.

        for(unsigned int ii=0; ii < fmap1->size(); ii++) {
            RPolygon *p1 = fmap1->at(ii).p();
            bool is_dup = fmap1->at(ii).isDuplicate();
            int parea = p1->area();
            p1->getColor(&r1, &g1, &b1);
            for(unsigned int jj=0;(jj < fmap2->size()) && (parea > 0); jj++) {
                int xl, yl, xh, yh;
                RPolygon *p2 = fmap2->at(jj).p();
                if(PolygonsOverlap(p1, p2, xl, yl, xh, yh)) {
                    if(!(is_dup && fmap2->at(jj).isDuplicate())) {
                        p2->getColor(&r2, &g2, &b2);
                        myr = r1 + r2;
                        myg = g1 + g2;
                        myb = b1 + b2;
                        RPolygon *pnew = RPolygon::alloc_RPolygon(xl, yl, xh, yh, myr, myg, myb);
#ifdef _DEBUG
#else
                        tbb::spin_mutex::scoped_lock lock(*m_rMutex);
#endif
                        (*m_resultMap).push_back(pnew);
                    }
                    parea -= (xh-xl+1)*(yh-yl+1);
                }
            }
        }

        delete fmap1;
        delete fmap2;
        RPolygon::free_RPolygon( slicePolygon );
    }

    ApplySplitOverlay(Polygon_map_t *resultMap, Polygon_map_t *map1, Polygon_map_t *map2, tbb::spin_mutex *rmutex) :
    m_resultMap(resultMap), m_map1(map1), m_map2(map2), m_rMutex(rmutex) {}
};


/*!
* @brief intersects two maps strip-wise
*
* @param[out] resultMap output map (must be allocated)
* @param[in] polymap1 map to be intersected
* @param[in] polymap2 map to be intersected
*/
void SplitParallelOverlay(Polygon_map_t **result_map, Polygon_map_t *polymap1, Polygon_map_t *polymap2) {
    int nthreads;
    bool automatic_threadcount = false;
    double domainSplitParallelTime;
    tbb::tick_count t0, t1;
    tbb::spin_mutex *resultMutex;
    if(gThreadsLow == THREADS_UNSET || gThreadsLow == tbb::task_scheduler_init::automatic ) {
        gThreadsLow = gThreadsHigh = tbb::task_scheduler_init::automatic;
        automatic_threadcount = true;
    }
    *result_map = new Polygon_map_t;

    RPolygon *p0 = (*polymap1)[0];
    int mapxSize, mapySize, ignore1, ignore2;
    p0->get(&ignore1, &ignore2, &mapxSize, &mapySize);
    (*result_map)->reserve(mapxSize*mapySize); // can't be any bigger than this
    resultMutex = new tbb::spin_mutex();

    int grain_size;
#ifdef _DEBUG
    grain_size = gMapXSize / 4;
#else
    grain_size = gGrainSize;
#endif

    for(nthreads = gThreadsLow; nthreads <= gThreadsHigh; nthreads++) {
        tbb::task_scheduler_init init(nthreads);
        if(gIsGraphicalVersion) {
            RPolygon *xp = RPolygon::alloc_RPolygon(0, 0, gMapXSize-1, gMapYSize-1, 0, 0, 0);  // Clear the output space
            RPolygon::free_RPolygon( xp );
        }
        // push the map size as the first polygon,
        p0 = RPolygon::alloc_RPolygon(0,0,mapxSize, mapySize);
        (*result_map)->push_back(p0);
        t0 = tbb::tick_count::now();
        tbb::parallel_for (tbb::blocked_range<int>(0,(int)(mapxSize+1),grain_size), ApplySplitOverlay((*result_map), polymap1, polymap2, resultMutex));
        t1 = tbb::tick_count::now();
        domainSplitParallelTime = (t1-t0).seconds()*1000;
        cout << "Splitting parallel with spin lock and ";
        if(automatic_threadcount) cout << "automatic";
        else cout << nthreads;
        cout << ((nthreads == 1) ? " thread" : " threads");
        cout << " took " << domainSplitParallelTime <<  " msec : speedup over serial " << (gSerialTime / domainSplitParallelTime) << std::endl;
        if(gCsvFile.is_open()) {
            gCsvFile << "," << domainSplitParallelTime;
        }
#if _DEBUG
        CheckPolygonMap(*result_map);
        ComparePolygonMaps(*result_map, gResultMap);
#endif
        for(int i=0; i<int((*result_map)->size());i++) {
            RPolygon::free_RPolygon((*result_map)->at(i));
        }
        (*result_map)->clear();

    }
    delete resultMutex;
    if(gCsvFile.is_open()) {
        gCsvFile << std::endl;
    }

}
