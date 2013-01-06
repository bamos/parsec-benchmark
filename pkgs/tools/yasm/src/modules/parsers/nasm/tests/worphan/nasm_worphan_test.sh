#! /bin/sh
# $Id: nasm_worphan_test.sh,v 1.1.1.1 2012/03/29 17:21:00 uid42307 Exp $
${srcdir}/out_test.sh nasm_test modules/parsers/nasm/tests/worphan "nasm-compat parser" "-Worphan-labels -f bin" ""
exit $?
