/*
    Copyright 2005-2010 Intel Corporation.  All Rights Reserved.

    This file is part of Threading Building Blocks.

    Threading Building Blocks is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    Threading Building Blocks is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Threading Building Blocks; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    As a special exception, you may use this file as part of a free software
    library without restriction.  Specifically, if other files instantiate
    templates or use macros or inline functions from this file, or you compile
    this file and link it with other files to produce an executable, this
    file does not by itself cause the resulting executable to be covered by
    the GNU General Public License.  This exception does not however
    invalidate any other reasons why the executable file might be covered by
    the GNU General Public License.
*/

#define HARNESS_NO_PARSE_COMMAND_LINE 1
#include "harness.h"

union char2bool {
    unsigned char c;
    volatile bool b;
} u;

// The function proves the compiler uses 0 or 1 to store a bool. It
// inspects what a compiler does when it loads a bool.  A compiler that
// uses a value other than 0 or 1 to represent a bool will have to normalize
// the value to 0 or 1 when the bool is cast to an unsigned char.
// Compilers that pass this test do not do the normalization, and thus must
// be assuming that a bool is a 0 or 1.
void TestBoolRepresentation () {
    unsigned i = 0;
    for( ; i<256; ++i ) {
        u.c = (unsigned char)i;
        unsigned char x = (unsigned char)u.b;
        if( x != i ) {
            REPORT("Test failed at iteration i=%d\n",i);
            break;
        }
    }
    ASSERT( i == 256, "bool representation differs from {0,1} set" );
}

// Test if all the necessary symbols are exported for the exceptions thrown by TBB.
// Missing exports result either in link error or in runtime assertion failure.
#include "tbb/tbb_exception.h"

template <typename E>
void TestExceptionClassExports ( const E& exc, tbb::internal::exception_id eid ) {
    // The assertion here serves to shut up warnings about "eid not used". 
    ASSERT( eid<tbb::internal::eid_max, NULL );
#if TBB_USE_EXCEPTIONS
    for ( int i = 0; i < 2; ++i ) {
        try {
            if ( i == 0 )
                throw exc;
#if !__TBB_THROW_ACROSS_MODULE_BOUNDARY_BROKEN
            else
                tbb::internal::throw_exception( eid );
#endif
        }
        catch ( E& e ) {
            ASSERT ( e.what(), "Missing what() string" );
        }
        catch ( ... ) {
            ASSERT ( __TBB_EXCEPTION_TYPE_INFO_BROKEN, "Unrecognized exception. Likely RTTI related exports are missing" );
        }
    }
#else /* !TBB_USE_EXCEPTIONS */
    (void)exc;
#endif /* !TBB_USE_EXCEPTIONS */
}

void TestExceptionClassesExports () {
    TestExceptionClassExports( std::bad_alloc(), tbb::internal::eid_bad_alloc );
    TestExceptionClassExports( tbb::bad_last_alloc(), tbb::internal::eid_bad_last_alloc );
    TestExceptionClassExports( std::invalid_argument("test"), tbb::internal::eid_nonpositive_step );
    TestExceptionClassExports( std::out_of_range("test"), tbb::internal::eid_out_of_range );
    TestExceptionClassExports( std::range_error("test"), tbb::internal::eid_segment_range_error );
    TestExceptionClassExports( std::range_error("test"), tbb::internal::eid_index_range_error );
    TestExceptionClassExports( tbb::missing_wait(), tbb::internal::eid_missing_wait );
    TestExceptionClassExports( tbb::invalid_multiple_scheduling(), tbb::internal::eid_invalid_multiple_scheduling );
    TestExceptionClassExports( tbb::improper_lock(), tbb::internal::eid_improper_lock );
}

int TestMain () {
    TestBoolRepresentation();
    TestExceptionClassesExports();
    return Harness::Done;
}
