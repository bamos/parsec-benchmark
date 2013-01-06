# $Id: Makefile.inc,v 1.1.1.1 2012/03/29 17:21:02 uid42307 Exp $

libyasm_a_SOURCES += modules/objfmts/rdf/rdf-objfmt.c

YASM_MODULES += objfmt_rdf

EXTRA_DIST += modules/objfmts/rdf/tests/Makefile.inc

include modules/objfmts/rdf/tests/Makefile.inc
