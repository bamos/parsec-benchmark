#! /bin/sh
# $Id: gas_test.sh,v 1.1.1.1 2012/03/29 17:20:59 uid42307 Exp $
${srcdir}/out_test.sh gas_test modules/parsers/gas/tests "gas-compat parser" "-f elf -p gas" ".o"
exit $?
