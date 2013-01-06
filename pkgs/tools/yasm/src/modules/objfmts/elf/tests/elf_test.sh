#! /bin/sh
# $Id: elf_test.sh,v 1.1.1.1 2012/03/29 17:21:02 uid42307 Exp $
# copied from yasm/modules/objfmts/coff/tests/coff_test.sh ; s/coff/elf/g
${srcdir}/out_test.sh elf_test modules/objfmts/elf/tests "elf objfmt" "-f elf" ".o"
exit $?
