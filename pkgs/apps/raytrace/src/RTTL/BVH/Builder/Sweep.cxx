#include "Sweep.hxx"

namespace RTTL {

  void SweepBVHBuilder::recursiveBuild(const int begin,
                    const int end,
                    const int nodeIndex,
                    const AABB& bounds,
                    const sse_f *const centroid,
                    int *const item,
                    AABB *const bvh,
                    int &numNodes,
                    const int depth)
  {
    AABB leftBounds,rightBounds;
    const unsigned int items = end-begin+1;
    assert(items>0);

    if ((items <= MIN_ITEMS) ||
    (depth >  MAX_DEPTH) ||
    bounds.m_min == bounds.m_max)
      {
      createLeaf:
    //cout << "Leaf: ";
    //DBG_PRINT(items);
    //DBG_PRINT(depth);
    //DBG_PRINT(bounds);
    //DBG_PRINT((items <= MIN_ITEMS));
    //DBG_PRINT((depth >  MAX_DEPTH));
    //DBG_PRINT(bounds.m_min == bounds.m_max);

    bvh[nodeIndex].createLeaf(begin,items);
    //             bvh[nodeIndex].extMinComponent() = BVHExtData(begin | (unsigned int)(1<<31));
    //             bvh[nodeIndex].extMaxComponent() = BVHExtData(0,0,items);
    return;
      }

    int left = begin;
    int right = end;

    leftBounds.setEmpty();
    rightBounds.setEmpty();

    const sse_f center = bounds.center();
    const sse_f diameter = bounds.diameter();

    unsigned int dim = maxDim3(diameter);
    float split = M128_FLOAT(center,dim);

    for (int i=0;i<3;i++)
      {
    while (1) {
      while (__builtin_expect((left <= right),1) && (M128_FLOAT(centroid[item[left]],dim) < split))
        leftBounds.extend(centroid[item[left++]]);

      while (__builtin_expect((left <= right),1) && (M128_FLOAT(centroid[item[right]],dim) >= split))
        rightBounds.extend(centroid[item[right--]]);

      if (__builtin_expect(right <= left,0))
        break;

      leftBounds.extend(centroid[item[right]]);
      rightBounds.extend(centroid[item[left]]);

      swap(item[left],item[right]);
      left++;
      right--;
    }
    if ((left-begin>0) && (end-right>0)) break;
    dim = (dim+1) % 3;
    split = M128_FLOAT(center,dim);
      }
    /*
      DBG_PRINT(split);
      DBG_PRINT(dim);
      DBG_PRINT(begin);
      DBG_PRINT(end);
      DBG_PRINT(left);
      DBG_PRINT(right);
    */

    const unsigned int itemsLeft  = left - begin;
    const unsigned int itemsRight = end - right;

    if (itemsLeft == 0 || itemsRight == 0)
      goto createLeaf;

    numNodes+=2;
    const unsigned int leftIndex  = numNodes + 0;
    const unsigned int rightIndex = numNodes + 1;


    bvh[nodeIndex].createNode(leftIndex,dim);

    //         bvh[nodeIndex].extMinComponent() = BVHExtData(leftIndex);
    //         bvh[nodeIndex].extMaxComponent() = BVHExtData(dim,0,0);

    recursiveBuild(begin           ,begin+itemsLeft-1,leftIndex, leftBounds, centroid,item,bvh,numNodes,depth+1);
    recursiveBuild(end-itemsRight+1,end              ,rightIndex,rightBounds,centroid,item,bvh,numNodes,depth+1);

  }

  void SweepBVHBuilder::my_build(const AABB *const aabb,
                 int *const item,
                 AABB *const bvh,
                 const int numAABBs)
  {
    sse_f *centroid = (sse_f*)malloc_align(sizeof(sse_f)*numAABBs);

    AABB sceneBounds;
    AABB centroidBounds;
    centroidBounds.setEmpty();
    sceneBounds.setEmpty();
    for (int i=0;i<numAABBs;i++)
      {
    item[i] = i;
    const sse_f c = aabb[i].center();
    centroid[i] = c;
    centroidBounds.extend(c);
    sceneBounds.extend(aabb[i]);
      }

    int numNodes = 0;
    recursiveBuild(0,numAABBs-1,0,centroidBounds,centroid,item,bvh,numNodes,0);
    //checkTree(bvh,item,0);
    adjustBounds(bvh,aabb,item,0,0);

    free_align(centroid);
  }

  void SweepBVHBuilder::checkTree(AABB *const bvh,
                   const int *const item,
                   const unsigned int index)
  {
    AABB &entry = bvh[index];

    if (entry.isLeaf())
      {
    const int items = entry.items();
    assert(items>0);
    //      cout << "Leaf (" << items << "): ";
    int *begin = (int*)item + entry.itemOffset();
    //for (int i=0;i<items;i++) cout << *begin++ << " ";
    cout << endl;
      }
    else
      {
    //      cout << "Node " << index << endl << flush;
    const unsigned int leftIndex  = entry.children()+0;
    const unsigned int rightIndex = entry.children()+1;
    DBG_PRINT(leftIndex);
    DBG_PRINT(rightIndex);
    checkTree(bvh,item,leftIndex);
    checkTree(bvh,item,rightIndex);
      }
  }

  unsigned int SweepBVHBuilder::adjustBounds(AABB *const bvh,
                          const AABB *const aabb,
                          const int *const item,
                          const unsigned int index,
                          const unsigned int begin)
  {
    AABB &entry = bvh[index];

    if (entry.isLeaf())
      {
    unsigned int t0 = entry.extMin();
    unsigned int t1 = entry.extMax();

    AABB box;
    box.setEmpty();
    const int items = entry.items();
    int *begin = (int*)item + entry.itemOffset();
    for (int i=0;i<items;i++)
      box.extend(aabb[*begin++]);
    box.extMin() = t0;
    box.extMax() = t1;
    entry = box;
    return items;
      }
    else
      {
    const unsigned int leftIndex  = entry.children()+0;
    const unsigned int rightIndex = entry.children()+1;
    const int numItemsLeft = adjustBounds(bvh,aabb,item,leftIndex,begin);
    const int numItemsRight = adjustBounds(bvh,aabb,item,rightIndex,begin+numItemsLeft);

    const AABB &boundsLeft = bvh[leftIndex];
    const AABB &boundsRight = bvh[rightIndex];

    unsigned int t0 = entry.extMin();
    unsigned int t1 = entry.extMax();

    entry.setEmpty();
    entry.extend(boundsLeft);
    entry.extend(boundsRight);
    entry.extMin() = t0;
    entry.extMax() = t1;

    const int items = numItemsLeft + numItemsRight;
    const float area = entry.area();
    const float lbArea = boundsLeft.area();
    const float rbArea = boundsRight.area();
    const float cost = (lbArea * (float)numItemsLeft + rbArea * (float)numItemsRight) * INTERSECTION_COST + area * TRAVERSAL_COST;

    /*
      DBG_PRINT(area);
      DBG_PRINT(lbArea);
      DBG_PRINT(rbArea);
      DBG_PRINT(numItemsLeft);
      DBG_PRINT(numItemsRight);
      DBG_PRINT(cost);
      DBG_PRINT((float)items*INTERSECTION_COST*area);
    */

    if ((float)items*INTERSECTION_COST*area < cost && items < ((1<<16)-1))
      {
        entry.createLeaf(begin,items);
        //entry.extMinComponent() = BVHExtData(begin | (unsigned int)(1<<31));
        //entry.extMaxComponent() = BVHExtData(0,0,items);
      }

    return items;
      }
  }

};
