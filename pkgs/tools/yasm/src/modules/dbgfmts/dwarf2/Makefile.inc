# $Id: Makefile.inc,v 1.1.1.1 2012/03/29 17:21:01 uid42307 Exp $

libyasm_a_SOURCES += modules/dbgfmts/dwarf2/dwarf2-dbgfmt.h
libyasm_a_SOURCES += modules/dbgfmts/dwarf2/dwarf2-dbgfmt.c
libyasm_a_SOURCES += modules/dbgfmts/dwarf2/dwarf2-line.c
libyasm_a_SOURCES += modules/dbgfmts/dwarf2/dwarf2-aranges.c
libyasm_a_SOURCES += modules/dbgfmts/dwarf2/dwarf2-info.c

YASM_MODULES += dbgfmt_dwarf2

EXTRA_DIST += modules/dbgfmts/dwarf2/tests/Makefile.inc
include modules/dbgfmts/dwarf2/tests/Makefile.inc
