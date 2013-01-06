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

#define TEST(a,b,c) if (a == b) return c;

#define TEST_NAME(a, b) TEST(a, b, #b);

/*
static char *err_str [] =
{
	"unknown",
	"out of memory",
	"malformat vector",
	"invalid parameter",
	"IO failure",
	"corrupted file"
};
*/


const char *cass_strerror (int err)
{
	TEST_NAME(err, CASS_ERR_UNKNOWN);
	TEST_NAME(err, CASS_ERR_OUTOFMEM);
	TEST_NAME(err, CASS_ERR_MALFORMATVEC);
	TEST_NAME(err, CASS_ERR_PARAMETER);
	TEST_NAME(err, CASS_ERR_IO);
	TEST_NAME(err, CASS_ERR_CORRUPTED);
	TEST(err, -1, "-1");
	return "unknown";
}


const char *cass_vec_dist_type_name (uint32_t dist)
{
	TEST_NAME(dist, CASS_VEC_DIST_TYPE_TRIVIAL);
	TEST_NAME(dist, CASS_VEC_DIST_TYPE_ID);
	TEST_NAME(dist, CASS_VEC_DIST_TYPE_L1);
	TEST_NAME(dist, CASS_VEC_DIST_TYPE_L2);
	TEST_NAME(dist, CASS_VEC_DIST_TYPE_MAX);
	TEST_NAME(dist, CASS_VEC_DIST_TYPE_HAMMING);
	TEST_NAME(dist, CASS_ANY);
	return "unknown";
}


const char *cass_vecset_dist_type_name (uint32_t dist)
{
	TEST_NAME(dist, CASS_VECSET_DIST_TYPE_TRIVIAL);
	TEST_NAME(dist, CASS_VECSET_DIST_TYPE_SINGLE);
	TEST_NAME(dist, CASS_VECSET_DIST_TYPE_EMD);
	TEST_NAME(dist, CASS_ANY);
	return "unknown";
}

const char *cass_vec_type_name (uint32_t t)
{
	TEST_NAME(t, CASS_VEC_INT);
	TEST_NAME(t, CASS_VEC_FLOAT);
	TEST_NAME(t, CASS_VEC_BIT);
	TEST_NAME(t, CASS_VEC_QUANT);
	TEST_NAME(t, CASS_ANY);
	return "unknown";
}

const char *cass_vecset_type_name (uint32_t t)
{
	TEST_NAME(t, CASS_VECSET_SINGLE);
	TEST_NAME(t, CASS_VECSET_SET);
	TEST_NAME(t, CASS_ANY);
	return "unknown";
}
