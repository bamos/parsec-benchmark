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

#include "tbb/pipeline.h"
#include "tbb/spin_mutex.h"
#include "tbb/atomic.h"
#include "tbb/tbb_thread.h"
#include <cstdlib>
#include <cstdio>
#include "harness.h"

// In the test, variables related to token counting are declared
// as unsigned long to match definition of tbb::internal::Token.

//! Id of thread that first executes work on non-thread-bound stages
tbb::tbb_thread::id thread_id;
//! Zero thread id
tbb::tbb_thread::id id0;
//! True if non-thread-bound stages must be executed on one thread
bool is_serial_execution;

struct Buffer {
    //! Indicates that the buffer is not used.
    static const unsigned long unused = ~0ul;
    unsigned long id;
    //! True if Buffer is in use.
    bool is_busy;
    unsigned long sequence_number;
    Buffer() : id(unused), is_busy(false), sequence_number(unused) {}
};

class waiting_probe {
    size_t check_counter;
public:
    waiting_probe() : check_counter(0) {}
    bool required( ) {
        ++check_counter;
        return !((check_counter+1)&size_t(0x7FFF));
    }
    void probe( ); // defined below
};

static const unsigned MaxStreamSize = 8000;
static const unsigned MaxStreamItemsPerThread = 1000;
//! Maximum number of filters allowed
static const unsigned MaxFilters = 4;
static unsigned StreamSize;
static const unsigned MaxBuffer = 8;
static bool Done[MaxFilters][MaxStreamSize];
static waiting_probe WaitTest;
static unsigned out_of_order_count;

#include "harness_concurrency_tracker.h"

template<typename T>
class BaseFilter: public T {
    bool* const my_done;
    const bool my_is_last;  
    bool my_is_running;
public:
    tbb::atomic<tbb::internal::Token> current_token;
    BaseFilter( tbb::filter::mode type, bool done[], bool is_last ) : 
        T(type),
        my_done(done),
        my_is_last(is_last),
        my_is_running(false),
        current_token()
    {}
    virtual Buffer* get_buffer( void* item ) {
        current_token++;
        return static_cast<Buffer*>(item);
    } 
    /*override*/void* operator()( void* item ) {
        // Check if work is done only on one thread when ntokens==1 or 
        // when pipeline has only one filter that is serial and non-thread-bound
        if( is_serial_execution && !this->is_bound() ) {
            // Get id of current thread
            tbb::tbb_thread::id id = tbb::this_tbb_thread::get_id();
            // At first execution, set thread_id to current thread id.
            // Serialized execution is expected, so there should be no race.
            if( thread_id == id0 )
                thread_id = id;
            // Check if work is done on one thread 
            ASSERT( thread_id == id, "non-thread-bound stages executed on different threads when must be executed on a single one");
        }
        Harness::ConcurrencyTracker ct;
        if( this->is_serial() )
            ASSERT( !my_is_running, "premature entry to serial stage" );
        my_is_running = true;
        Buffer* b = get_buffer(item);
        if( b ) {
            if( this->is_ordered() ) {
                if( b->sequence_number == Buffer::unused ) 
                    b->sequence_number = current_token-1;
                else
                    ASSERT( b->sequence_number==current_token-1, "item arrived out of order" );
            } else if( this->is_serial() ) {
                if( b->sequence_number != current_token-1 && b->sequence_number != Buffer::unused )
                    out_of_order_count++;
            }
            ASSERT( b->id < StreamSize, NULL ); 
            ASSERT( !my_done[b->id], "duplicate processing of token?" ); 
            ASSERT( b->is_busy, NULL );
            my_done[b->id] = true;
            if( my_is_last ) {
                b->id = Buffer::unused;
                b->sequence_number = Buffer::unused;
                __TBB_store_with_release(b->is_busy, false);
            }
        }
        my_is_running = false;
        return b;  
    }
};

template<typename T>
class InputFilter: public BaseFilter<T> {
    tbb::spin_mutex input_lock;
    Buffer buffer[MaxBuffer];
    const tbb::internal::Token my_number_of_tokens;
public:
    InputFilter( tbb::filter::mode type, tbb::internal::Token ntokens, bool done[], bool is_last ) :
        BaseFilter<T>(type, done, is_last),
        my_number_of_tokens(ntokens)
    {}
    /*override*/Buffer* get_buffer( void* ) {
        unsigned long next_input;
        unsigned free_buffer = 0; 
        { // lock protected scope
            tbb::spin_mutex::scoped_lock lock(input_lock);
            if( this->current_token>=StreamSize )
                return NULL;
            next_input = this->current_token++; 
            // once in a while, emulate waiting for input; this only makes sense for serial input
            if( this->is_serial() && WaitTest.required() )
                WaitTest.probe( );
            while( free_buffer<MaxBuffer )
                if( __TBB_load_with_acquire(buffer[free_buffer].is_busy) )
                    ++free_buffer;
                else {
                    buffer[free_buffer].is_busy = true;
                    break;
                }
        }
        ASSERT( free_buffer<my_number_of_tokens, "premature reuse of buffer" );
        Buffer* b = &buffer[free_buffer]; 
        ASSERT( &buffer[0] <= b, NULL ); 
        ASSERT( b <= &buffer[MaxBuffer-1], NULL ); 
        ASSERT( b->id == Buffer::unused, NULL);
        b->id = next_input;
        ASSERT( b->sequence_number == Buffer::unused, NULL);
        return b;
    }
};

class process_loop {
public:
    void operator()( tbb::thread_bound_filter* tbf ) {
        tbb::thread_bound_filter::result_type flag;
        do
            flag = tbf->process_item();
        while( flag != tbb::thread_bound_filter::end_of_stream );
    }
};

//! The struct below repeats layout of tbb::pipeline.
struct hacked_pipeline {
    tbb::filter* filter_list;
    tbb::filter* filter_end;
    tbb::empty_task* end_counter;
    tbb::atomic<tbb::internal::Token> input_tokens;
    tbb::atomic<tbb::internal::Token> global_token_counter;
    bool end_of_input;
    bool has_thread_bound_filters;

    virtual ~hacked_pipeline();
};

//! The struct below repeats layout of tbb::internal::ordered_buffer.
struct hacked_ordered_buffer {
    void* array; // This should be changed to task_info* if ever used
    tbb::internal::Token array_size;
    tbb::internal::Token low_token;
    tbb::spin_mutex array_mutex;
    tbb::internal::Token high_token;
    bool is_ordered;
    bool is_bound;
};

//! The struct below repeats layout of tbb::filter.
struct hacked_filter {
    tbb::filter* next_filter_in_pipeline;
    hacked_ordered_buffer* input_buffer;
    unsigned char my_filter_mode;
    tbb::filter* prev_filter_in_pipeline;
    tbb::pipeline* my_pipeline;
    tbb::filter* next_segment;

    virtual ~hacked_filter();
};

#if _MSC_VER && !defined(__INTEL_COMPILER)
    // Workaround for overzealous compiler warnings
    // Suppress compiler warning about constant conditional expression
    #pragma warning (disable: 4127)
#endif

void clear_global_state() {
    Harness::ConcurrencyTracker::Reset();
    memset( Done, 0, sizeof(Done) );
    thread_id = id0;
    is_serial_execution = false;
}

void TestTrivialPipeline( unsigned nthread, unsigned number_of_filters ) {
    // There are 3 non-thread-bound filter types: serial_in_order and serial_out_of_order, parallel
    static const tbb::filter::mode non_tb_filters_table[] = { tbb::filter::serial_in_order, tbb::filter::serial_out_of_order, tbb::filter::parallel}; 
    // There are 2 thread-bound filter types: serial_in_order and serial_out_of_order 
    static const tbb::filter::mode tb_filters_table[] = { tbb::filter::serial_in_order, tbb::filter::serial_out_of_order }; 
    
    const unsigned number_of_non_tb_filter_types = sizeof(non_tb_filters_table)/sizeof(non_tb_filters_table[0]);
    const unsigned number_of_tb_filter_types = sizeof(tb_filters_table)/sizeof(tb_filters_table[0]);
    const unsigned number_of_filter_types = number_of_non_tb_filter_types + number_of_tb_filter_types;

    REMARK( "testing with %lu threads and %lu filters\n", nthread, number_of_filters );
    ASSERT( number_of_filters<=MaxFilters, "too many filters" );
    tbb::internal::Token max_tokens = nthread < MaxBuffer ? nthread : MaxBuffer;
    // The loop has 1 iteration if max_tokens=1 and 2 iterations if max_tokens>1:
    // one iteration for ntokens=1 and second for ntokens=max_tokens 
    // Iteration for ntokens=1 is required in each test case to check if pipeline run only on one thread 
    unsigned max_iteration = max_tokens > 1 ? 2 : 1; 
    tbb::internal::Token ntokens = 1;
    for( unsigned iteration = 0; iteration < max_iteration; iteration++) {
        if( iteration > 0 ) 
            ntokens = max_tokens;
        // Count maximum iterations number
        unsigned limit = 1;
        for( unsigned i=0; i<number_of_filters; ++i)
            limit *= number_of_filter_types;
        // Iterate over possible filter sequences
        for( unsigned numeral=0; numeral<limit; ++numeral ) {
            REMARK( "testing configuration %lu of %lu\n", numeral, limit );
            // Build pipeline
            tbb::pipeline pipeline;
            tbb::filter* filter[MaxFilters];
            unsigned temp = numeral;
            // parallelism_limit is the upper bound on the possible parallelism
            unsigned parallelism_limit = 0;
            // number of thread-bound-filters in the current sequence
            unsigned number_of_tb_filters = 0;
            // ordinal numbers of thread-bound-filters in the current sequence
            unsigned array_of_tb_filter_numbers[MaxFilters];
            for( unsigned i=0; i<number_of_filters; ++i, temp/=number_of_filter_types ) {
                bool is_bound = temp%number_of_filter_types&0x1;
                tbb::filter::mode filter_type;
                if( is_bound ) {
                    filter_type = tb_filters_table[temp%number_of_filter_types/number_of_non_tb_filter_types];
                } else
                    filter_type = non_tb_filters_table[temp%number_of_filter_types/number_of_tb_filter_types];
                const bool is_last = i==number_of_filters-1;
                if( is_bound ) {
                    if( i == 0 )
                        filter[i] = new InputFilter<tbb::thread_bound_filter>(filter_type,ntokens,Done[i],is_last);
                    else
                        filter[i] = new BaseFilter<tbb::thread_bound_filter>(filter_type,Done[i],is_last);
                    array_of_tb_filter_numbers[number_of_tb_filters] = i;
                    number_of_tb_filters++;
                } else {
                    if( i == 0 )
                       filter[i] = new InputFilter<tbb::filter>(filter_type,ntokens,Done[i],is_last);
                    else
                        filter[i] = new BaseFilter<tbb::filter>(filter_type,Done[i],is_last);
                }
                pipeline.add_filter(*filter[i]);
                if ( filter[i]->is_serial() ) {
                    parallelism_limit += 1;
                } else {
                    parallelism_limit = nthread;
                }
            }
            clear_global_state();
            // Account for clipping of parallelism.
            if( parallelism_limit>nthread ) 
                parallelism_limit = nthread;
            if( parallelism_limit>ntokens )
                parallelism_limit = (unsigned)ntokens;
            StreamSize = nthread; // min( MaxStreamSize, nthread * MaxStreamItemsPerThread );

            for( unsigned i=0; i<number_of_filters; ++i ) {
                static_cast<BaseFilter<tbb::filter>*>(filter[i])->current_token=0;
            }
            tbb::tbb_thread* t[MaxFilters];
            for( unsigned j = 0; j<number_of_tb_filters; j++)
                t[j] = new tbb::tbb_thread(process_loop(), static_cast<tbb::thread_bound_filter*>(filter[array_of_tb_filter_numbers[j]]));
            if( ntokens == 1 || ( number_of_filters == 1 && number_of_tb_filters == 0 && filter[0]->is_serial() ))
                is_serial_execution = true;
            pipeline.run( ntokens );
            for( unsigned j = 0; j<number_of_tb_filters; j++)
               t[j]->join();
            ASSERT( !Harness::ConcurrencyTracker::InstantParallelism(), "filter still running?" );
            for( unsigned i=0; i<number_of_filters; ++i )
                ASSERT( static_cast<BaseFilter<tbb::filter>*>(filter[i])->current_token==StreamSize, NULL );
            for( unsigned i=0; i<MaxFilters; ++i )
                for( unsigned j=0; j<StreamSize; ++j ) {
                    ASSERT( Done[i][j]==(i<number_of_filters), NULL );
                }
            if( Harness::ConcurrencyTracker::PeakParallelism() < parallelism_limit ) 
                REMARK( "nthread=%lu ntokens=%lu MaxParallelism=%lu parallelism_limit=%lu\n",
                    nthread, ntokens, Harness::ConcurrencyTracker::PeakParallelism(), parallelism_limit );
            for( unsigned i=0; i < number_of_filters; ++i ) {
                delete filter[i];
                filter[i] = NULL;
            }
            for( unsigned j = 0; j<number_of_tb_filters; j++)
                delete t[j];
            pipeline.clear();
        }
    }
}

#include "harness_cpu.h"

static int nthread; // knowing number of threads is necessary to call TestCPUUserTime

void waiting_probe::probe( ) {
    if( nthread==1 ) return;
    REMARK("emulating wait for input\n");
    // Test that threads sleep while no work.
    // The master doesn't sleep so there could be 2 active threads if a worker is waiting for input
    TestCPUUserTime(nthread, 2);
}

#include "tbb/task_scheduler_init.h"

int TestMain () {
    out_of_order_count = 0;
    if( MinThread<1 ) {
        REPORT("must have at least one thread");
        exit(1);
    }

    // Test with varying number of threads.
    for( nthread=MinThread; nthread<=MaxThread; ++nthread ) {
        // Initialize TBB task scheduler
        tbb::task_scheduler_init init(nthread);

        // Test pipelines with 1 and maximal number of filters
        for( unsigned n=1; n<=MaxFilters; n*=MaxFilters ) {
            // Thread-bound stages are serviced by user-created threads those 
            // don't run the pipeline and don't service non-thread-bound stages 
            TestTrivialPipeline(nthread,n);
        }

        // Test that all workers sleep when no work
        TestCPUUserTime(nthread);
    }
    if( !out_of_order_count )
        REPORT("Warning: out of order serial filter received tokens in order\n");
    return Harness::Done;
}
