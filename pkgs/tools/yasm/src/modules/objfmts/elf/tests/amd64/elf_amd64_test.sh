#! /bin/sh
# $Id: elf_amd64_test.sh,v 1.1.1.1 2012/03/29 17:21:02 uid42307 Exp $
${srcdir}/out_test.sh elf_amd64_test modules/objfmts/elf/tests/amd64 "elf-amd64 objfmt" "-m amd64 -f elf" ".o"
exit $?
