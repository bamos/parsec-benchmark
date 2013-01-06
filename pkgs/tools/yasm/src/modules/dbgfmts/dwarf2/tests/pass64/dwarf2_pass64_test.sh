#! /bin/sh
# $Id: dwarf2_pass64_test.sh,v 1.1.1.1 2012/03/29 17:21:02 uid42307 Exp $
${srcdir}/out_test.sh dwarf2_pass64_test modules/dbgfmts/dwarf2/tests/pass64 "dwarf2 dbgfmt pass64" "-f elf64 -p gas -g dwarf2" ".o"
exit $?
