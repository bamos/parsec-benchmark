#! /bin/sh
# $Id: gas_macho64_test.sh,v 1.1.1.1 2012/03/29 17:21:03 uid42307 Exp $
${srcdir}/out_test.sh macho_test modules/objfmts/macho/tests/gas64 "GAS 64-bit macho objfmt" "-f macho64 -p gas" ".o"
exit $?
