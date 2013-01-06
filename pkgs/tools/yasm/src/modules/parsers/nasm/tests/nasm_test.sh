#! /bin/sh
# $Id: nasm_test.sh,v 1.1.1.1 2012/03/29 17:20:59 uid42307 Exp $
${srcdir}/out_test.sh nasm_test modules/parsers/nasm/tests "nasm-compat parser" "-f bin" ""
exit $?
