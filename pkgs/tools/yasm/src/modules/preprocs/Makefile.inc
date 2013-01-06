# $Id: Makefile.inc,v 1.1.1.1 2012/03/29 17:21:00 uid42307 Exp $

EXTRA_DIST += modules/preprocs/nasm/Makefile.inc
EXTRA_DIST += modules/preprocs/raw/Makefile.inc
EXTRA_DIST += modules/preprocs/cpp/Makefile.inc

include modules/preprocs/nasm/Makefile.inc
include modules/preprocs/raw/Makefile.inc
include modules/preprocs/cpp/Makefile.inc
