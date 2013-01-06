# Wrapper Makefile to choose how to build the hooks library. Supported options:
#   1. Makefile.static - Create a static library
#   2. Makefile.shared - Create a shared library
#
# Creating a shared library is the default. This allows us to change the
# instrumentation of PARSEC workloads without recompiling the benchmark
# programs, but it makes things a little more complicated
#
# The shared library Makefile uses libtool, which is part of the PARSEC
# distribution. You also need to make sure that the shared library can be
# found at run time. The PARSEC framework already takes care of this.

#include Makefile.static
include Makefile.shared

