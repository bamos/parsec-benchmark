Name:  Face Simulation 

Description: Face simulation is an increasingly important production-quality physical simulation application.  It animates an anatomical model of a human face driven by the action of facial musculature and the motion of the jawbone.  A time sequence of muscle activation values and kinematic parameters for the jaw motion is provided as input.

The collision detection portion of this application (used to ensure lips are modeled properly) has been disabled.  Collision detection is only a small part of this application.  It has been studied in much more detail in other RMS applications, and so we did not take the time to parallelize it inside this application.

Parallelization: The mesh is statically partitioned, one partition per thread.  An overlap region is created around the boundaries of each partition to allow the partitions to be processed independently.  The cost of this reduced synchronization and communication is redundant computation in some modules, which increases with the number of partitions.

=======================================
Build Instructions:

This benchmark depends on the PhysBAM physical simulation library and a task queue object file that are distributed as part of this benchmark suite.  Please read README_FIRST.txt before attempting to build this benchmark.

Once the PhysBAM library and task queue object file are built, simply type:
> make

=======================================
User Notes: 

The input files for this benchmark may be packaged separately.  They are placed in a Public_Data directory parallel to the Benchmarks directory.

The region of interest is a small loop inside main.cpp. It is clearly marked with a comment.


=======================================
Program Execution 

face [-threads <# of threads>]

=======================================
Usage Example:

face -threads 1

The region of interest takes about 7.8s to execute on a 2.4GHz Conroe system with one thread and 4.3s for two threads.


=======================================
DEBUG / VERIFICATION / TESTING

The benchmark outputs a line of timing information per module like this:

        UPBS (FEM) - Initialize                     0.1233 s

The benchmark prints this information per frame, and then prints the cumulative information.  For a single frame run, it will actually print three copies of the timing information.  It is best to just use the last copy of the timing information.  The total time is printed like this:

SIMULATION                                          0.0000
  FRAME                                             4.3356

The number after “FRAME” is the execution time of the ROI.

There is currently no self-checking code in the benchmark.  The benchmark can be compiled with “make TYPE=debug” to enable a thorough set of assertions which is likely to catch most problems.

=========================
Revision History

12/8/2006: Chris Hughes & Adam Kerin – initial version of package

=======================================
Author: see ../PHYSBAM_COPYRIGHT.txt

Acknowledgements:  Christopher J. Hughes
