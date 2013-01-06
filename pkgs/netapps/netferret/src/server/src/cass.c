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
#include <cass_dist.h>


cass_reg_t cass_vec_dist_class_reg;
cass_reg_t cass_vecset_dist_class_reg;
cass_reg_t cass_table_opr_reg;


int cass_init (void)
{
	cass_vec_dist_class_init();
	cass_vecset_dist_class_init();
	cass_table_opr_init();

	cass_table_opr_add(opr_raw.name, &opr_raw);
	cass_table_opr_add(opr_lsh.name, &opr_lsh); 

	cass_vec_dist_class_add(&vec_dist_trivial);
	cass_vec_dist_class_add(&vec_dist_L1_int);
	cass_vec_dist_class_add(&vec_dist_L2_int);
	cass_vec_dist_class_add(&vec_dist_L1_float);
	cass_vec_dist_class_add(&vec_dist_L2_float);
	cass_vec_dist_class_add(&vec_dist_hamming);
	cass_vec_dist_class_add(&vec_dist_cos_float);

	cass_vecset_dist_class_add(&vecset_dist_trivial);
	cass_vecset_dist_class_add(&vecset_dist_single);
	cass_vecset_dist_class_add(&vecset_dist_emd);
	cass_vecset_dist_class_add(&vecset_dist_myemd);

	return 0;
}

int cass_cleanup (void)
{
	cass_table_opr_cleanup();
	cass_vecset_dist_class_cleanup();
	cass_vec_dist_class_cleanup();

	return 0;
}

