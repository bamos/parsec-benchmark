# $Id: Makefile.inc,v 1.1.1.1 2012/03/29 17:21:02 uid42307 Exp $

# Assume objfmt_coff is included

YASM_MODULES += objfmt_win32

EXTRA_DIST += modules/objfmts/win32/tests/Makefile.inc

include modules/objfmts/win32/tests/Makefile.inc
