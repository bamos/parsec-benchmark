# $Id: Makefile.inc,v 1.1.1.1 2012/03/29 17:21:02 uid42307 Exp $

libyasm_a_SOURCES += modules/objfmts/coff/coff-objfmt.c
libyasm_a_SOURCES += modules/objfmts/coff/coff-objfmt.h
libyasm_a_SOURCES += modules/objfmts/coff/win64-except.c

YASM_MODULES += objfmt_coff

EXTRA_DIST += modules/objfmts/coff/tests/Makefile.inc

include modules/objfmts/coff/tests/Makefile.inc
