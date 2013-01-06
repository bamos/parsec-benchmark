#! /bin/sh
# $Id: x86_gas32_test.sh,v 1.1.1.1 2012/03/29 17:21:01 uid42307 Exp $
${srcdir}/out_test.sh x86_gas32_test modules/arch/x86/tests/gas32 "x86 gas format" "-f elf32 -p gas" ".o"
exit $?
