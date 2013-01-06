#! /bin/sh
# $Id: coff_test.sh,v 1.1.1.1 2012/03/29 17:21:02 uid42307 Exp $
${srcdir}/out_test.sh coff_test modules/objfmts/coff/tests "coff objfmt" "-f coff" ".o"
exit $?
