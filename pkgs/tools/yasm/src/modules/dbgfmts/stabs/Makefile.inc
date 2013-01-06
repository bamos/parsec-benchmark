# $Id: Makefile.inc,v 1.1.1.1 2012/03/29 17:21:01 uid42307 Exp $

libyasm_a_SOURCES += modules/dbgfmts/stabs/stabs-dbgfmt.c

YASM_MODULES += dbgfmt_stabs

EXTRA_DIST += modules/dbgfmts/stabs/tests/Makefile.inc
include modules/dbgfmts/stabs/tests/Makefile.inc
