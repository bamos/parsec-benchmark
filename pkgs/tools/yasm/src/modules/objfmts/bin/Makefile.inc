# $Id: Makefile.inc,v 1.1.1.1 2012/03/29 17:21:03 uid42307 Exp $

libyasm_a_SOURCES += modules/objfmts/bin/bin-objfmt.c

YASM_MODULES += objfmt_bin

EXTRA_DIST += modules/objfmts/bin/tests/Makefile.inc

include modules/objfmts/bin/tests/Makefile.inc
