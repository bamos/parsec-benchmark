#include "BinnedAllDims.hxx"

namespace RTTL {

#define DBG(a) /* do nothing */

  void BinnedAllDims::recursiveBuild(const AABB *const triAABB,
                              const sse_f *const triCentroid,
                              AABB *const bvh,
                              int *const item,
                              int nodeID,
                              int &nextFree,
                              int begin,
                              int end,
                              AABB &voxel)
  {
    DBG(PING);
    DBG(cout << "nodeID : " << nodeID << " " << begin << "-" << end << endl);
    static const int numBins = 4;
    _ALIGN(16) AABB centroid_bounds;
    AABB rightBox[numBins+1];
    AABB leftBox[numBins+1];
    AABB binBounds[numBins];
    AABB box;

    //      PING; cout << begin << " " << end << endl;
    //     const int numBins = 16; //8 + 2*(int)sqrtf((end-begin));
    // #if defined(_WIN32)
    // windows compiler can't handle dynamic stack arrays
    if (end - begin <= MIN_ITEMS)
      {
      createLeaf:
    //      cout << "Leaf" << endl;
    //DBG_PRINT(end-begin);
    DBG(cout << "LEAF! " << begin << " " << (end-begin) << endl);
    bvh[nodeID] = voxel;
    bvh[nodeID].createLeaf(begin,end-begin);
    return;
      }
    //      cout << "new bounds " << endl;
    //      PING;
    //      cout << "addr " << (int*)&centroid_bounds << endl;
    centroid_bounds.setEmpty();
    //      PING;
#pragma unroll(8)
    for (int i=begin; i<end;i++)
      {
    //      cout << " i " << i << endl;
    //      cout << " item " << item[i] << endl;
    //      cout << " ctr " << triCentroid[item[i]] << endl;
    //      cout << "centroid "<< i << " "  << triCentroid[item[i]] << endl;
    centroid_bounds.extend(triCentroid[item[i]]);
      }
    //      cout << "boudns " << centroid_bounds << endl;

    DBG(cout << "tribounds " << voxel << " / centbounds " << centroid_bounds << endl);
    //     if (centroid_bounds.min3f() == centroid_bounds.max3f())
    //       goto createLeaf;
    // OK, centroid_bounds is input centroid_bounds of current node
    int dim = centroid_bounds.maxIndex();
    //      cout << "DIM " << dim << endl;
    FATAL("This is where it fails -- maxIndex on a (N=1,T=m128) type box is always 0");
    int bestSplit = -1;
    float scale = 0.0f;


    DBG(cout << "--------------------------------------------" << endl;
    cout << "maxindex of " << centroid_bounds << " is " << dim << endl);

    const sse_f c_min = centroid_bounds.min_f();

#define CHECK_ALL_THREE_DIMS
#ifdef CHECK_ALL_THREE_DIMS
    for (int j=0;j<3;j++,dim = (dim+1)%3)
#else
      ;
    for (int j=0;j<1;j++) // yes, a single loop, I know -- at least we can keep the continue's below.
#endif
      {
    DBG(cout << "checking dim " << dim << "  (best split so far " << bestSplit << ")" << endl);
    //      PING;
    //const float voxWidth = centroid_bounds.max3f()[dim] - centroid_bounds.min3f()[dim];
    //const RTVec3f diameter = centroid_bounds[1] - centroid_bounds[0];
    const sse_f diameter = centroid_bounds.diameter();
    const float voxWidth = M128_FLOAT(diameter,dim);

    //      PING;

    if (voxWidth < 1E-3f) continue;

#pragma unroll(8)
    //      PING;
    for (int i=0;i<numBins;i++)
      binBounds[i].setEmpty();
    //      PING;

    int  binCount[numBins];
#pragma unroll(8)
    for (int i=0;i<numBins;i++)
      binCount[i] = 0;
    //      PING;

    scale = (numBins/voxWidth) * 0.999f;

    const int skip = 1;
#pragma unroll(8)
    for (int i=begin;i<end;i+=skip)
      {
        //          cout << "process " << i << endl;

        const int t = item[i];
        //          cout << " t " << t << endl;
        //          cout << (int *)&triCentroid[0] << endl;
        //          cout << (int *)&centroid_bounds << endl;
        //          cout << (int *)&triAABB[0] << endl;
        //          {
        //            float cent_t_dim = M128_FLOAT(triCentroid[t],dim);
        //            cout << cent_t_dim << endl;
        //            float min_bounds_dim = centroid_bounds.min3f()[dim];
        //            cout << min_bounds_dim << endl;
        //            float f = (cent_t_dim - min_bounds_dim) * scale;
        //            cout << f << endl;
        //            cout << "rounding ... " << endl;
        //            int b = int(f);
        // //          FATAL("gcc dies before this 'cout << bin' line; i.e. _in_ the typecast...");
        //            cout << (float)b << endl;
        //          }
        //const int bin = int((M128_FLOAT(triCentroid[t],dim) - centroid_bounds.min3f()[dim]) * scale);
        const int bin = int((M128_FLOAT(triCentroid[t],dim) - M128_FLOAT(c_min,dim)) * scale);

        //          cout << " bin " << bin << endl;
        assert(bin >= 0 && bin < numBins);
        binBounds[bin].extend(triAABB[t]);
        binCount[bin]++;
      }

    //      PING;

    // now, have extent of all bins; sweep forward and backward
    float rightArea[numBins];
    int rightCount[numBins];

    box.setEmpty();
    int count = 0;
    for (int i=numBins-1;i>=0;--i)
      {
        box.extend(binBounds[i]);
        count += binCount[i];
        rightBox[i] = box;
        rightArea[i] = box.area();
        rightCount[i] = count;
      }
    // now, sweep from the left
    box.setEmpty();
    count = 0;
    float bestCost = (end - begin) * voxel.area();
    //     float bestCost = (end - begin) * voxel.area() * INTERSECTION_COST;
    for (int i=1;i<numBins;i++)
      {
        box.extend(binBounds[i-1]);
        count += binCount[i-1];
        leftBox[i] = box;

        int lnum = count;
        int rnum = rightCount[i];

        DBG(cout << "- bin " << i << endl);
        //         DBG(cout << "- bin " << i << "  " << lnum << "*" << leftBox[i] << "/" << box.area() << " + " << rnum << "*" << rightBox[i] << "/" << rightArea[i] << endl);


        DBG(cout << "   l   : " << (lnum *  box.area()) << "  " << lnum << "*" << leftBox[i] << endl);
        DBG(cout << "   r   : " << (rnum *  rightArea[i]) << "  " << rnum << "*" << rightBox[i] << endl);
        DBG(cout << "   vox : " << (voxel.area()) << endl);
        if (lnum == 0 || rnum == 0)
          continue;

        float lsa = box.area();
        float rsa = rightArea[i];

        //float cost =  lsa * lnum + rsa * rnum;
        float cost =  (lsa * lnum + rsa * rnum + 1 * voxel.area()); // the '1' is the traversal cost...
        //         float cost =  (lsa * lnum + rsa * rnum) * INTERSECTION_COST + voxel.area() * TRAVERSAL_COST;

        DBG(cout << "   = " << cost << endl);
        if (cost < bestCost)
          {
        bestCost = cost;
        bestSplit = i;
        DBG(cout << "new best split " << i << " / " << cost << endl);
          }

      }
    if (bestSplit != -1) break;
      }

    //      PING;
    //      cout << "best split is " << bestSplit << endl;
    if (bestSplit == -1) {
      goto createLeaf;
    }

    int *l = item + begin;
    int *r = item + end-1;
    int mid;
    while (1)
      {
    //while (l < r && int((M128_FLOAT(triCentroid[*l],dim) - centroid_bounds.min3f()[dim]) * scale) < bestSplit)
    while (l < r && int((M128_FLOAT(triCentroid[*l],dim) - M128_FLOAT(c_min,dim)) * scale) < bestSplit)

      ++l;

    //while (l < r && int((M128_FLOAT(triCentroid[*l],dim) - centroid_bounds.min3f()[dim]) * scale) < bestSplit)
    while (l < r && int((M128_FLOAT(triCentroid[*l],dim) - M128_FLOAT(c_min,dim)) * scale) < bestSplit)

      --r;
    if (l == r)
      {
        mid = r - (item);// + 1;
        break;
      }
    const int h = *l; *l = *r; *r = h;
      }

    bvh[nodeID] = voxel;
    bvh[nodeID].createNode(nextFree,dim);

    DBG(cout << "  split into " << (mid-begin) << " + " << (end-mid) << " items" << endl);
    nextFree += 2;
    recursiveBuild(triAABB,triCentroid,bvh,item,bvh[nodeID].children()+0,nextFree,begin,mid,leftBox[bestSplit]);
    recursiveBuild(triAABB,triCentroid,bvh,item,bvh[nodeID].children()+1,nextFree,mid,end,rightBox[bestSplit]);
  }


  void BinnedAllDims::my_build(const AABB *const aabb,
                   int *const item,
                   AABB *const bvh,
                   const int numAABBs)
  {
    sse_f *centroid = (sse_f*)malloc_align(sizeof(sse_f)*numAABBs);

    AABB triBounds;
    triBounds.setEmpty();
    for (int i=0;i<numAABBs;i++)
      {
    item[i] = i;
    centroid[i] = aabb[i].center();
    triBounds.extend(aabb[i]);
      }
    int nextFreeNode = 1;
    recursiveBuild(aabb,centroid,bvh,item,0,nextFreeNode,0,numAABBs,triBounds);
    free_align(centroid);
  }

};
