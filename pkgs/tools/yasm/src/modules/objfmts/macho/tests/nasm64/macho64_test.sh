#! /bin/sh
# $Id: macho64_test.sh,v 1.1.1.1 2012/03/29 17:21:03 uid42307 Exp $
${srcdir}/out_test.sh macho_test modules/objfmts/macho/tests/nasm64 "64-bit macho objfmt" "-f macho64" ".o"
exit $?
