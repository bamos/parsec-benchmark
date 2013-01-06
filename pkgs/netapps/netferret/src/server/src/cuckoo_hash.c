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


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>

#include <cass.h>
#include "cuckoo_hash.h"

#ifndef __FUNCTION__
#define __FUNCTION__  "__FUNCTION__"
#endif

#define MALLOC( type, n ) ( type * )malloc( ( n ) * sizeof( type ) )
#define MALLOC_ERROR( str1, str2 ) fprintf( stderr, "MALLOC ERROR: In function `%s': could not allocate memory for `%s'\n", str1, str2 ), exit( EXIT_FAILURE )
#define FATAL_ERROR( str1, str2 ) fprintf( stderr, "FATAL ERROR: In function `%s': %s\n", str1, str2 ), exit( EXIT_FAILURE )

#define TRUE 1
#define FALSE !TRUE
#define MAX_FUNCTION_SIZE 256
#define keycmp( s1, s2 ) strcmp( ( const char * )s1, ( const char * )s2 )

typedef struct {             /* hash table cell type */
    cass_vecset_id_t vid;
    char *key;
    int value;
} CKHash_Cell;

struct CKHash_Table_ {
    int size;                         /*  current size */
    int shift;                        /*  value used for hash function */
    int table_size;                   /*  size of hash tables */
    int min_size, mean_size;          /*  rehash trigger sizes */
    int max_chain;                    /*  max. iterations in insert */
    CKHash_Cell *T1;                  /*  point to hash table 1 */
    CKHash_Cell *T2;                  /*  point to hash table 2 */
    int function_size;                /*  size of hash function */
    int *a1;                          /*  hash function 1 */
    int *a2;                          /*  hash function 2 */
    vtable_t *vtable;
};

static inline int ckh_get_size( CKHash_Table *D )
{
    return( D->size );
}

static inline int ckh_get_table_size( CKHash_Table *D )
{
    return( D->table_size );
}

static inline void ckh_init( int a[], int function_size )
{
    int i;

    for( i = 0; i < function_size; i++ )
        a[i] = ( ( int )(rand() * rand()) << 1 ) + 1;
}

static inline void ckh_hash( unsigned long *h, int a[], int function_size, int table_size, int shift, char *key )
{
    int i;

    *h = 0;
    for( i = 0; key[i]; i++ )
        *h ^= ( unsigned int )( a[( i % function_size )]* ( unsigned char )key[i] );
    *h = ( ( unsigned int )*h >> shift ) % table_size;
}

CKHash_Table *ckh_alloc_table( int table_size, vtable_t *_vt )
{
    CKHash_Table *D;

    if( ( D = MALLOC( CKHash_Table, 1 ) ) == NULL )
        MALLOC_ERROR( __FUNCTION__, "D" );
    D->vtable = _vt;
    D->size = 0;
    D->table_size = table_size;
    D->mean_size = 5 * ( 2 * table_size ) / 12;
    D->min_size = ( 2 * table_size ) / 5;
    D->shift = 32 - ( int )( log( table_size ) / log( 2 ) + 0.5 );
    D->max_chain = 4 + ( int )( 4 * log( table_size ) / log( 2 ) + 0.5 );

    if( ( D->T1 = MALLOC( CKHash_Cell, D->table_size ) ) == NULL )
        MALLOC_ERROR( __FUNCTION__, "D->T1" );
    if( ( D->T2 = MALLOC( CKHash_Cell, D->table_size ) ) == NULL )
        MALLOC_ERROR( __FUNCTION__, "D->T2" );

    D->function_size = MAX_FUNCTION_SIZE;
    if( ( D->a1 = MALLOC( int, D->function_size ) ) == NULL )
        MALLOC_ERROR( __FUNCTION__, "D->a1" );
    if( ( D->a2 = MALLOC( int, D->function_size ) ) == NULL )
        MALLOC_ERROR( __FUNCTION__, "D->a2" );

    ckh_init( D->a1, D->function_size );
    ckh_init( D->a2, D->function_size );

    int j;
    for( j = 0; j < D->table_size; j++ )
    {
         D->T1[j].vid = D->T2[j].vid = CASS_ID_INV;
    }

    return( D );
}

int ckh_rehash_insert( CKHash_Table *D, cass_vecset_id_t vid)
{
    unsigned long hkey;
    int j;
    CKHash_Cell x, temp;
    char *vkey;
    
    x.vid = vid;

    for( j = 0; j < D->max_chain; j++ ) 
    {
	vkey = ARRAY_GET(*(D->vtable), x.vid);	
        ckh_hash( &hkey, D->a1, D->function_size, D->table_size, D->shift, vkey );
        temp = D->T1[hkey];
        D->T1[hkey] = x;
        if( temp.vid == CASS_ID_INV )
            return( TRUE );

        x = temp;
	vkey = ARRAY_GET(*(D->vtable), x.vid);	
        ckh_hash( &hkey, D->a2, D->function_size, D->table_size, D->shift, vkey );
        temp = D->T2[hkey];
        D->T2[hkey] = x;
        if( temp.vid == CASS_ID_INV ) 
            return( TRUE );

        x = temp;
    }

    for( j = 0; j < D->table_size; j++ )
    {
         D->T1[j].vid = D->T2[j].vid = CASS_ID_INV;
    }

    ckh_init( D->a1, D->function_size );
    ckh_init( D->a2, D->function_size );

    return( FALSE );
}

void ckh_rehash( CKHash_Table *D, int new_size )
{
    CKHash_Table *D_new;
    int k;

    //debug("rehash: %d\n", new_size);
    D_new = ckh_alloc_table( new_size, D->vtable );

    for( k = 0; k < D->table_size; k++ ) 
    {
        if( ( D->T1[k].vid != CASS_ID_INV ) && ( !ckh_rehash_insert( D_new, D->T1[k].vid ) ) )
        {
	    //debug("restart at %d\n", k);
            k = -1;
            continue;
        }
        if( ( D->T2[k].vid != CASS_ID_INV ) && ( !ckh_rehash_insert( D_new, D->T2[k].vid ) ) ) {
	    //debug("restart at %d\n", k);
            k = -1;
	}
    }

    free( D->T1 );
    free( D->T2 );
    free( D->a1 );
    free( D->a2 );

    D_new->size = D->size;
    *D = *D_new;
    free( D_new );
}

int ckh_insert( CKHash_Table *D, cass_id_t vid )
{
    unsigned long h1, h2; 
    int j;
    CKHash_Cell x, temp;

    char *vkey = ARRAY_GET(*(D->vtable), vid);
    /*
    ** If the element is already in D, return
    */
    ckh_hash( &h1, D->a1, D->function_size, D->table_size, D->shift, vkey );
    if( D->T1[h1].vid != CASS_ID_INV )
        if ( D->T1[h1].vid == vid )
	    return( FALSE );

    ckh_hash( &h2, D->a2, D->function_size, D->table_size, D->shift, vkey );
    if( D->T2[h2].vid != CASS_ID_INV )
        if ( D->T2[h2].vid == vid )
	    return( FALSE );

    /*
    ** If not, insert the new element in D.
    */

    x.vid = vid;
    for( j = 0; j < D->max_chain; j++ ) 
    {
        temp = D->T1[h1];
        D->T1[h1] = x;
        if( temp.vid == CASS_ID_INV ) 
        {
            D->size++;
            if( D->table_size < D->size ) 
                ckh_rehash( D, 2*D->table_size );
            return( TRUE );
        }

        x = temp;
	vkey = ARRAY_GET(*(D->vtable), x.vid);
        ckh_hash( &h2, D->a2, D->function_size, D->table_size, D->shift, vkey );
        temp = D->T2[h2];
        D->T2[h2] = x;
        if( temp.vid == CASS_ID_INV ) 
        {
            D->size++;
            if( D->table_size < D->size ) 
                ckh_rehash( D, 2*D->table_size );
            return( TRUE );
        }

        x = temp;
	vkey = ARRAY_GET(*(D->vtable), x.vid);
        ckh_hash( &h1, D->a1, D->function_size, D->table_size, D->shift, vkey );
    }

    //debug("forced rehashing %s\n", vkey);
    
    /*
    ** Forced rehash.
    */
    if( D->size < D->mean_size )
        ckh_rehash( D, D->table_size );
    else
        ckh_rehash( D, 2*D->table_size );

    ckh_insert( D, x.vid );

    return( TRUE );
}

int ckh_lookup( CKHash_Table *D, char *key )
{
    unsigned long hkey;
    char *vkey;

    ckh_hash( &hkey, D->a1, D->function_size, D->table_size, D->shift, key );
    if( D->T1[hkey].vid != CASS_ID_INV )
	vkey = ARRAY_GET(*(D->vtable), D->T1[hkey].vid);
        if( !keycmp( vkey, key ) )
            return( TRUE );

    ckh_hash( &hkey, D->a2, D->function_size, D->table_size, D->shift, key );
    if( D->T2[hkey].vid != CASS_ID_INV )
	vkey = ARRAY_GET(*(D->vtable), D->T2[hkey].vid);
        if( !keycmp( vkey, key ) )
             return( TRUE );

    return( FALSE );
}

cass_id_t ckh_get( CKHash_Table *D, char *key ) 
{
    unsigned long hkey;
    char *vkey;
    ckh_hash( &hkey, D->a1, D->function_size, D->table_size, D->shift, key );
    if( D->T1[hkey].vid != CASS_ID_INV ) {
	vkey = ARRAY_GET(*(D->vtable), D->T1[hkey].vid);
        if( !keycmp( vkey, key ) )
            return( D->T1[hkey].vid );
    }

    ckh_hash( &hkey, D->a2, D->function_size, D->table_size, D->shift, key );
    if( D->T2[hkey].vid != CASS_ID_INV ) {
	vkey = ARRAY_GET(*(D->vtable), D->T2[hkey].vid);
        if( !keycmp( vkey, key ) )
             return( D->T2[hkey].vid );
    }

    return( CASS_ID_INV );
}

CKHash_Table *ckh_destruct_table( CKHash_Table *D )
{
    free( D->T1 );
    free( D->T2 );
    free( D->a1 );
    free( D->a2 );
    free( D );

    return( NULL );
}

