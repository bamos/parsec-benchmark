#ifndef LRT__CMDLINE_HXX
#define LRT__CMDLINE_HXX

#include "RTTL/common/RTInclude.hxx"
#include "RTTL/common/RTVec.hxx"

namespace LRT {

  struct Options
  {
    static CmdLineParser root;

    /*! command line options that cannot be parsed by \see parseThis
      are then passed on to all child parsers. only if neither of
      those could parse them are they returned to the parent
      parser. That way, we can have a certain option be used by multiple
      different objects.

      e.g., you can have

      <program> -accelstructure BVH -stats bvhfile.stat -framebuffer -stats fbstats.stat
    */
    vector<CmdLineParser *> child;

    /*! start parsing (argc,argv[]) from given argument
      'first'. return ID of first argument this parser could _not_
      parse, or argc if all optoins could be parsed */
    int parse(int argc, char *argv[], int first = 1) = 0;
  };

}

#endif
