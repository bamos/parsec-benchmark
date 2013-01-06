# $Id: Makefile.inc,v 1.1.1.1 2012/03/29 17:21:03 uid42307 Exp $

# Assume objfmt_coff is included

YASM_MODULES += objfmt_win64 objfmt_x64

EXTRA_DIST += modules/objfmts/win64/tests/Makefile.inc

include modules/objfmts/win64/tests/Makefile.inc
