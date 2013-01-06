/*
 * Functions and macros required for system compatibility
 *
 * Copyright 2007, Christian Bienia, licensed under Apache 2.0
 *
 * file : system.h
 * author : Christian Bienia - cbienia@cs.princeton.edu
 * description : Compatibility functions
 *
 */

#ifndef COMPAT_H
#define COMPAT_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif



/* Directory separator */
#if defined(__unix__) || defined(__unix) || defined(unix) || defined(__UNIX__) || defined(__UNIX) || defined(UNIX)
#  define DIR_SEPARATOR "/"
#else
# define DIR_SEPARATOR "\\"
#endif

#endif /* COMPAT_H */
