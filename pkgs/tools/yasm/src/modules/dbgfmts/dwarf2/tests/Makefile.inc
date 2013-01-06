# $Id: Makefile.inc,v 1.1.1.1 2012/03/29 17:21:01 uid42307 Exp $

EXTRA_DIST += modules/dbgfmts/dwarf2/tests/pass32/Makefile.inc
EXTRA_DIST += modules/dbgfmts/dwarf2/tests/pass64/Makefile.inc
EXTRA_DIST += modules/dbgfmts/dwarf2/tests/passwin64/Makefile.inc
include modules/dbgfmts/dwarf2/tests/pass32/Makefile.inc
include modules/dbgfmts/dwarf2/tests/pass64/Makefile.inc
include modules/dbgfmts/dwarf2/tests/passwin64/Makefile.inc

