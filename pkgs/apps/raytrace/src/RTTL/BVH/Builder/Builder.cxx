#include <cstring>

#include "RTTL/BVH/BVH.hxx"

/*! \file Builder.cxx Actually, this file will eventually contain a
  registry mechanism for mapping from a builder type (currently
  specified via a string) to a respective builder plugin. for
  starters, let's just hard-code every builder we have */

// so first, we have to include all the builders we know

#include "Sweep.hxx"
#include "BinnedAllDims.hxx"
#include "BinnedAllDimsSaveSpace.hxx"
#include "OnDemandBuilder.hxx"

namespace RTTL
{
  const char *BVHBuilder::Options::defaultBuilder = "binnedalldimssavespace";

    // strcasecmp does not exits under windows !!!
  BVHBuilder *BVHBuilder::get(const char *builderType, BVH *bvh)
  {
    cout << "using BVH builder " << builderType << endl;
    if (builderType == NULL)
      return get("default",bvh);
    else if (!strcmp(builderType, "default"))
      //return new SweepBVHBuilder(bvh);
      return new BinnedAllDimsSaveSpace(bvh);
    else if (!strcmp(builderType, "sweep"))
      return new SweepBVHBuilder(bvh);
    else if (!strcmp(builderType, "binnedalldims"))
      return new BinnedAllDims(bvh);
    else if (!strcmp(builderType, "binnedalldimssavespace"))
      return new BinnedAllDimsSaveSpace(bvh);
    else if (!strcmp(builderType, "binned3d"))
      return new BinnedAllDims(bvh);
    else if (!strcmp(builderType, "binned"))
      // should be single-dimmed, but don't have any, yet
      return new BinnedAllDims(bvh);
    else if (!strcmp(builderType, "ondemand") || !strcmp(builderType, "lazy"))
      return new OnDemandBuilder(bvh);
    else {
        std::string error = std::string("unknown BVH builder type ");
        error+=std::string(builderType);
    //throw error;
    FATAL(error.c_str());
    }
  }

};

