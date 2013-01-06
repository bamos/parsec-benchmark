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

// Before including pipeline.h, set up the variable to count heap allocated
// filter_node objects, and make it known for the header.
int filter_node_count = 0;
#define __TBB_TEST_FILTER_NODE_COUNT filter_node_count
#include "tbb/pipeline.h"

#include "tbb/atomic.h"
#include "harness.h"

const int n_tokens = 8;
const int max_counter = 16;
static tbb::atomic<int> tmp_counter;

class check_type {
    unsigned int id;
    bool is_ready;
public:
    check_type() : id(0), is_ready(false) {}
    void function() {
        if( id == 0 ) {
            id = 1;
            is_ready = true;
        }
    }
};

template<typename U>
class input_filter {
    tbb::atomic<int> counter;
public:
    input_filter() {
        counter = max_counter;
    } 
    U operator()( tbb::flow_control& control ) {
        if( --counter < 0 ) {
            control.stop();
        }
        return U();
    }
};

template<>
class input_filter<void> {
    tbb::atomic<int> counter;
public:
    input_filter() {
        counter = max_counter;
    } 
    void operator()( tbb::flow_control& control ) {
        if( --counter < 0 ) {
            control.stop();
        }
    }
};

template<typename T, typename U>
class middle_filter {
public:
    U operator()(T /*my_storage*/) {
        return U();
    }
};

template<typename T>
class output_filter {
public:
    void operator()(T) {
        tmp_counter++;
    }
};

void check_and_reset() {
    ASSERT(tmp_counter == max_counter, "not all tokens were passed through pipeline");
    tmp_counter = 0;
}

static const tbb::filter::mode filter_table[] = { tbb::filter::parallel, tbb::filter::serial_in_order, tbb::filter::serial_out_of_order}; 
const unsigned number_of_filter_types = sizeof(filter_table)/sizeof(filter_table[0]);

void run_function_spec() {
    ASSERT(!filter_node_count, NULL);
    input_filter<void> i_filter;
    // Test pipeline that contains only one filter
    for( unsigned i = 0; i<number_of_filter_types; i++) {
        tbb::filter_t<void, void> one_filter( filter_table[i], i_filter );
        ASSERT(filter_node_count==1, "some filter nodes left after previous iteration?");
        tbb::parallel_pipeline( n_tokens, one_filter );
#if __TBB_LAMBDAS_PRESENT
        tbb::atomic<int> counter;
        counter = max_counter;
        // Construct filter using lambda-syntax when parallel_pipeline() is being run;
        tbb::parallel_pipeline( n_tokens, 
            tbb::make_filter<void, void>(filter_table[i], [&counter]( tbb::flow_control& control ) {
                    if( counter-- == 0 )
                        control.stop();
                    }
            )
        );
#endif
    }
    ASSERT(!filter_node_count, "filter_node objects leaked");
}

template<typename type1, typename type2>
void run_function() {
    ASSERT(!filter_node_count, NULL);

    const size_t number_of_filters = 3;

    input_filter<type1> i_filter;
    middle_filter<type1, type2> m_filter;
    output_filter<type2> o_filter;

    unsigned limit = 1;
    // Test pipeline that contains number_of_filters filters
    for( unsigned i=0; i<number_of_filters; ++i)
        limit *= number_of_filter_types;
    // Iterate over possible filter sequences
    for( unsigned numeral=0; numeral<limit; ++numeral ) {
        unsigned temp = numeral;
        tbb::filter::mode filter_type[number_of_filter_types];
        for( unsigned i=0; i<number_of_filters; ++i, temp/=number_of_filter_types ) 
            filter_type[i] = filter_table[temp%number_of_filter_types];

        tbb::filter_t<void, type1> filter1( filter_type[0], i_filter );
        tbb::filter_t<type1, type2> filter2( filter_type[1], m_filter );
        tbb::filter_t<type2, void> filter3( filter_type[2], o_filter );
        ASSERT(filter_node_count==3, "some filter nodes left after previous iteration?");
        // Create filters sequence when parallel_pipeline() is being run
        tbb::parallel_pipeline( n_tokens, filter1 & filter2 & filter3 );
        check_and_reset();

        // Create filters sequence partially outside parallel_pipeline() and also when parallel_pipeline() is being run
        tbb::filter_t<void, type2> filter12;
        filter12 = filter1 & filter2;
        tbb::parallel_pipeline( n_tokens, filter12 & filter3 );
        check_and_reset();

        tbb::filter_t<void, void> filter123 = filter12 & filter3;
        // Run pipeline twice with the same filter sequence
        for( unsigned i = 0; i<2; i++ ) {
            tbb::parallel_pipeline( n_tokens, filter123 );
            check_and_reset();
        }

        // Now copy-construct another filter_t instance, and use it to run pipeline
        {
            tbb::filter_t<void, void> copy123( filter123 );
            tbb::parallel_pipeline( n_tokens, copy123 );
            check_and_reset();
        }

        // Construct filters and create the sequence when parallel_pipeline() is being run
        tbb::parallel_pipeline( n_tokens, 
                   tbb::make_filter<void, type1>(filter_type[0], i_filter) &
                   tbb::make_filter<type1, type2>(filter_type[1], m_filter) &
                   tbb::make_filter<type2, void>(filter_type[2], o_filter) );
        check_and_reset();

        // Construct filters, make a copy, destroy the original filters, and run with the copy
        int cnt = filter_node_count;
        {
            tbb::filter_t<void, void>* p123 = new tbb::filter_t<void,void> (
                   tbb::make_filter<void, type1>(filter_type[0], i_filter) &
                   tbb::make_filter<type1, type2>(filter_type[1], m_filter) &
                   tbb::make_filter<type2, void>(filter_type[2], o_filter) );
            ASSERT(filter_node_count==cnt+5, "filter node accounting error?");
            tbb::filter_t<void, void> copy123( *p123 );
            delete p123;
            ASSERT(filter_node_count==cnt+5, "filter nodes deleted prematurely?");
            tbb::parallel_pipeline( n_tokens, copy123 );
            check_and_reset();
        }
        ASSERT(filter_node_count==cnt, "scope ended but filter nodes not deleted?");

#if __TBB_LAMBDAS_PRESENT
        tbb::atomic<int> counter;
        counter = max_counter;
        // Construct filters using lambda-syntax and create the sequence when parallel_pipeline() is being run;
        tbb::parallel_pipeline( n_tokens, 
            tbb::make_filter<void, type1>(filter_type[0], [&counter]( tbb::flow_control& control ) -> type1 {
                    if( --counter < 0 )
                        control.stop();
                    return type1(); }
            ) &
            tbb::make_filter<type1, type2>(filter_type[1], []( type1 /*my_storage*/ ) -> type2 {
                    return type2(); }
            ) &
            tbb::make_filter<type2, void>(filter_type[2], [] ( type2 ) -> void { 
                    tmp_counter++; }
            ) 
        );
        check_and_reset();
#endif
    }
    ASSERT(!filter_node_count, "filter_node objects leaked");
}

#include "tbb/task_scheduler_init.h"

int TestMain() {
    // Test with varying number of threads.
    for( int nthread=MinThread; nthread<=MaxThread; ++nthread ) {
        // Initialize TBB task scheduler
        tbb::task_scheduler_init init(nthread);
        
        // Run test several times with different types
        run_function_spec();
        run_function<size_t,int>();
        run_function<int,double>();
        run_function<check_type,size_t>();
    }
    return Harness::Done;
}

