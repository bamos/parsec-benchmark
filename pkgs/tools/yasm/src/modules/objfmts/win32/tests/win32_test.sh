#! /bin/sh
# $Id: win32_test.sh,v 1.1.1.1 2012/03/29 17:21:03 uid42307 Exp $
${srcdir}/out_test.sh win32_test modules/objfmts/win32/tests "win32 objfmt" "-f win32" ".obj"
exit $?
