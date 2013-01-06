/*
** Copyright (C) 2005 Thai Computational Linguistics Laboratory (TCL)
** National Institute of Information and Communications Technology (NICT)
** Written by Canasai Kruengkrai <canasaiREMOVETHIS@gmail.com>
**
** This library is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#ifndef __cuckoo_hash_H
#define __cuckoo_hash_H
#include <cass_type.h>
#include <cass_array.h>

typedef ARRAY_TYPE(char *) vtable_t;

typedef struct CKHash_Table_ CKHash_Table;

CKHash_Table *ckh_alloc_table( int table_size, vtable_t *_vt);

int ckh_insert( CKHash_Table *D, cass_id_t id );

int ckh_lookup( CKHash_Table *D, char *key );

cass_id_t ckh_get( CKHash_Table *D, char *key );

CKHash_Table *ckh_destruct_table( CKHash_Table *D );

static inline int ckh_get_size( CKHash_Table *D );

static inline int ckh_get_table_size( CKHash_Table *D );

#endif /* __cuckoo_hash_H */
