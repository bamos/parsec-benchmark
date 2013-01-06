# $Id: Makefile.inc,v 1.1.1.1 2012/03/29 17:21:02 uid42307 Exp $

libyasm_a_SOURCES += modules/objfmts/xdf/xdf-objfmt.c

YASM_MODULES += objfmt_xdf

EXTRA_DIST += modules/objfmts/xdf/tests/Makefile.inc

include modules/objfmts/xdf/tests/Makefile.inc
