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

// Tests that methods of class task that changed from non-static to static between 
// TBB 2.2 and TBB 3.0 still work when TBB_DEPRECATED_TASK_INTERFACE is defined.

#define TBB_DEPRECATED_TASK_INTERFACE 1
#include "tbb/task.h"
#include "tbb/atomic.h"
#include "../test/harness_assert.h"

tbb::atomic<int> Sum;
tbb::atomic<int> DummyCount;

class DummyTask: public tbb::task {
    tbb::task* execute() {
        ASSERT(false,"DummyTask should never execute");
        return NULL;
    }
public:
    //! Used to verify that right task was destroyed
    bool can_destroy;
    DummyTask() : can_destroy(false) {
        ++DummyCount;
    }
    ~DummyTask() {
        ASSERT(can_destroy,"destroyed wrong task?");
        --DummyCount;
    }
};

class ChildTask: public tbb::task {
public:
    const int addend;
    ChildTask( int addend_ ) : addend(addend_) {}
    tbb::task* execute() {
        Sum+=addend;
        return NULL;
    }
};

void TestFormallyNonstaticMethods() {
    // Check signatures
    void (tbb::task::*d)( tbb::task& ) = &tbb::task::destroy;
    void (tbb::task::*s)( tbb::task& ) = &tbb::task::spawn;
    void (tbb::task::*slist)( tbb::task_list& ) = &tbb::task::spawn;

    DummyTask& root = *new( tbb::task::allocate_root() ) DummyTask;

    for( int use_list=0; use_list<2; ++use_list ) {
        // Note: the cases n=0 is important to check, because spawn(task_list) 
        // has different logic for the empty case.
        for( int n=0; n<1000; n=n<5 ? n+1 : n=n+n/5 ) {
            Sum=0;
            root.set_ref_count(n+1);
            if( use_list ) {
                tbb::task_list list;
                for( int i=0; i<n; ++i ) {
                    tbb::task& child = *new( root.allocate_child() ) ChildTask(i+1);
                    list.push_back(child);
                }
                // Spawn the children
                (root.*slist)(list);
            } else { 
                // Do spawning without a list
                for( int i=0; i<n; ++i ) {
                    tbb::task& child = *new( root.allocate_child() ) ChildTask(i+1);
                    // Spawn one child
                    (root.*s)(child);
                }
            }
            root.wait_for_all();
            // Check that children ran.
            ASSERT( Sum==n*(n+1)/2, "child did not run?" );
        }
    }

    //! Check method destroy.
    root.set_ref_count(1);
    DummyTask& child = *new( root.allocate_child() ) DummyTask;
    child.can_destroy=true;
    (root.*d)( child );
    ASSERT( root.ref_count()==0, NULL );
    root.can_destroy=true;
    (root.*d)( root );
    ASSERT( !DummyCount, "constructor/destructor mismatch" );
}

#include "tbb/task_scheduler_init.h"
#include "../test/harness.h"

int TestMain () {
    for( int p=MinThread; p<=MaxThread; ++p ) {
        tbb::task_scheduler_init init(p);
        TestFormallyNonstaticMethods();
    }
    return Harness::Done;
}
