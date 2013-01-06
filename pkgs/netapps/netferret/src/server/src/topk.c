/* AUTORIGHTS
Copyright (C) 2007 Princeton University
      
This file is part of Ferret Toolkit.

Ferret Toolkit is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <cass.h>

static inline int itopk_ge (itopk_t a, itopk_t b)
{
	return a.key >= b.key;
}

static inline int itopk_rev_ge (itopk_t a, itopk_t b)
{
	return b.key >= a.key;
}

static inline int ftopk_ge (ftopk_t a, ftopk_t b)
{
	return a.key >= b.key;
}

static inline int ftopk_rev_ge (ftopk_t a, ftopk_t b)
{
	return b.key >= a.key;
}

QUICKSORT_GENERATE(ftopk, ftopk_t)

QUICKSORT_GENERATE(ftopk_rev, ftopk_t)

QUICKSORT_GENERATE(itopk, itopk_t)

QUICKSORT_GENERATE(itopk_rev, itopk_t)

