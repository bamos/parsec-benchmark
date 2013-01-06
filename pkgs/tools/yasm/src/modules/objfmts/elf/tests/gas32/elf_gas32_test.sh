#! /bin/sh
# $Id: elf_gas32_test.sh,v 1.1.1.1 2012/03/29 17:21:02 uid42307 Exp $
${srcdir}/out_test.sh elf_gas32_test modules/objfmts/elf/tests/gas32 "GAS elf-x86 objfmt" "-f elf32 -p gas" ".o"
exit $?
