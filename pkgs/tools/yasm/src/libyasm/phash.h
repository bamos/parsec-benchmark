/* Modified for use with yasm by Peter Johnson.
 * $Id: phash.h,v 1.1.1.1 2012/03/29 17:21:04 uid42307 Exp $
 */
/*
------------------------------------------------------------------------------
By Bob Jenkins, September 1996.
lookupa.h, a hash function for table lookup, same function as lookup.c.
Use this code in any way you wish.  Public Domain.  It has no warranty.
Source is http://burtleburtle.net/bob/c/lookupa.h
------------------------------------------------------------------------------
*/

unsigned long phash_lookup(const char *k, size_t length,
                           unsigned long level);
void phash_checksum(const char *k, size_t length, unsigned long *state);
