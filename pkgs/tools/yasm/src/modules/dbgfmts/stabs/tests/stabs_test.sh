#! /bin/sh
# $Id: stabs_test.sh,v 1.1.1.1 2012/03/29 17:21:01 uid42307 Exp $
# copied from yasm/modules/objfmts/coff/tests/coff_test.sh ; s/coff/stabs/g
${srcdir}/out_test.sh stabs_test modules/dbgfmts/stabs/tests "stabs dbgfmt" "-f elf -g stabs" ".o"
exit $?
