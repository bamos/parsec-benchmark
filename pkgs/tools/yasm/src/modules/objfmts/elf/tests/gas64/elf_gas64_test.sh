#! /bin/sh
# $Id: elf_gas64_test.sh,v 1.1.1.1 2012/03/29 17:21:02 uid42307 Exp $
${srcdir}/out_test.sh elf_gas64_test modules/objfmts/elf/tests/gas64 "GAS elf-amd64 objfmt" "-f elf64 -p gas" ".o"
exit $?
