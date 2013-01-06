#! /bin/sh
# $Id: win64_test.sh,v 1.1.1.1 2012/03/29 17:21:03 uid42307 Exp $
${srcdir}/out_test.sh win64_test modules/objfmts/win64/tests "win64 objfmt" "-f win64" ".obj"
exit $?
