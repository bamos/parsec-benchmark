#! /bin/sh
# $Id: gas_macho32_test.sh,v 1.1.1.1 2012/03/29 17:21:03 uid42307 Exp $
${srcdir}/out_test.sh macho_test modules/objfmts/macho/tests/gas32 "GAS 32-bit macho objfmt" "-f macho32 -p gas" ".o"
exit $?
