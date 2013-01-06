#ifdef _MSC_VER
// The next warning happens in system files.
// 4530 warning is a little bit strange: /EHsc *IS* actually enabled.
#pragma warning(disable: 4530) // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
#pragma warning(disable: 4996) // * was declared deprecated
#endif

#include "include/lrt.h"
#include <iostream>
#include <vector>
#include "FrameBuffer.hxx"
#include "RTTL/BVH/Builder/Builder.hxx"
#include "RTTL/common/MapOptions.hxx"

using std::vector;
using std::copy;

using namespace std;

namespace LRT {


  /*! \todo TAKE RECOGNIZED PARAMETERS _OFF_ THE COMMAND LINE !!! */
  void ParseCmdLine(int *argc, char **argv) 
  {
    /* (iw, oct 22, 07: hmmm... reverting to 'old-style' parameter
       parsing, even though maptions is more elegant. reason: idea
       of rtInit(&argc,argv) etc is that they can take thoes options
       off the command line that they know, and return the rest to
       the app w/o the app ever actually seeing them. in particular,
       the kernel and the app should not use the same options class
       _instance_ (though they could of course use the same class to
       implement their behavior). to allow that, the options class
       would have to allow pulling parameters off, and returning a
       valid (though stripped) argc/argv to the caller.
       
       i.e., something like:
       
       //  while (myOptions.defined("res")) {
       //     FrameBuffer::options::defaultRes.x = myOptions.get("res", FrameBuffer::options::defaultRes.x, 0);
       //     FrameBuffer::options::defaultRes.y = myOptions.get("res", FrameBuffer::options::defaultRes.y, 1);
       //     myOptions.remove("res",2);
       //  }
         
       */

    MapOptions myOptions;
    myOptions.parse(*argc,argv);
    
    // cout << myOptions << endl;
    
    FrameBuffer::Options::usePBOs = !myOptions.defined("no-pbo, no-pbos");
    if (myOptions.get("pbo", 0) || myOptions.get("pbo") == string("on"))
      FrameBuffer::Options::usePBOs = true;
    
    FrameBuffer::Options::useMemoryFB = myOptions.defined("mem-fb, memory-fb");
    
    BVHBuilder::Options::defaultBuilder = myOptions.get("bvh-build-method", BVHBuilder::Options::defaultBuilder);
    BVHBuilder::Options::defaultBuilder = myOptions.get("bvh-builder", BVHBuilder::Options::defaultBuilder);
    
    cout << "default BVH builder : " << BVHBuilder::Options::defaultBuilder << endl;
    
  }
};

void lrtInit(int *argc, char **argv) {
    // flush stuff to zero, turn off denormals, set denormals to zero,
    // see http://softpixel.com/~cwright/programming/simd/sse.php for
    // which bits are which ;)
    int oldMXCSR = _mm_getcsr();
    int newMXCSR = oldMXCSR | 0x80A0;
    _mm_setcsr( newMXCSR );
    
    LRT::ParseCmdLine(argc,argv);
}

