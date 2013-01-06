*********Physical Simulation Benchmark*********
                 VERSION 1.0

PRELIMINARY COMPILE INSTRUCTIONS:

*Compiling and running in the tc shell is recommended
*icc version 9.0 is recommended

****** IMPORTANT NOTE ********
The Physical_Simulation directory must be installed for the following codes to
run:
PCG,Dual Intersection, Adjust Velocity, Face Simulation
Follow the below steps to successfully install 


*Environment variables need to be set
PHYSBAM to the physics_bench directory
PHYSBAM_THREADS 1

exampe in .cshrc file: 
setenv PHYSBAM /home/user/Physical_Sim/physics_bench
setenv PHYSBAM_THREADS 1

*Go to TaskQ/lib
at the command line 
>make

*Then go to physics_bench/Public_Library
at the command line 
>make

*Actual Benchmarks must be downloaded and unzipped seperatly.
*Once unzipped, the files will be located at:
Physical_Sim/physics_bench/Benchmarks 

