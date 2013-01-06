# $Id: Makefile.inc,v 1.1.1.1 2012/03/29 17:20:58 uid42307 Exp $

EXTRA_DIST += tools/re2c/Makefile.inc
EXTRA_DIST += tools/genperf/Makefile.inc
EXTRA_DIST += tools/python-yasm/Makefile.inc

include tools/re2c/Makefile.inc
include tools/genperf/Makefile.inc
include tools/python-yasm/Makefile.inc
