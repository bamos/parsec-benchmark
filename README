PARSEC Benchmark Suite 3.0
==========================


1.) Overview

The Princeton Application Repository for Shared-Memory Computers (PARSEC) is a
collection of parallel programs which can be used for performance studies of
multiprocessor machines.

The PARSEC distribution is composed of packages and the PARSEC framework.
Packages correspond to benchmark programs, libraries and other essential
components. Each package can be compiled in a number of ways as determined by
a build configuration. Build configurations contain information such as which
features of the package should be enabled, which compilers to use and how the
package should be optimized. PARSEC ships with predefined inputs that can be
used to run the benchmarks. The inputs for each program exhibit different
characteristics such as execution time and working set size.


2.) Licenses

Before you start using, modifying or distributing PARSEC, its programs or the
supplied inputs in any way, make sure you understand all licenses involved.
The PARSEC framework itself is available under a liberal open source license,
as explained in the file LICENSE which is in the same directory as this README
file. Each program uses its own license, which is different in some cases. Some
of the inputs have their own license, too. Licenses for source code can be found
in the 'src/' directory of each package. Licenses for inputs can be found in the
'inputs/' directory of the package.

We distribute the programs and their workloads bundled with PARSEC merely to
allow PARSEC users a convenient access to them and because the license terms
allow us do so in each case. You have to take appropriate steps yourself to make
sure you don't violate any license terms.


3.) Requirements

PARSEC requires at least 8 GB, but we recommend 12 GB or more. The disk usage
can be broken down as follows:

PARSEC occupies about 7 GB with a raw installation. Additional 500 MB are
needed for each set of binaries. To build packages and run them extra space is
required for temporary files, up to several GB if the whole suite is to be
built and executed in one run without cleaning up intermittently.

The PARSEC benchmarks have been parallelized with pthreads, OpenMP, TBB and
atomic instructions. Many workloads support more than one parallelization. Each
parallelization has its own requirements that have to be fulfilled in order to
be able to build and run workloads that use it. By default only support for
pthreads and OpenMP are needed. Gcc supports OpenMP since version 4.2.0.

PARSEC has been successfully tested on the following systems:

	- Linux/i386
	- Linux/x86_64
	- Linux/Itanium
	- Solaris/Sparc

Limited support exists for the following platforms, but not all benchmark
programs might be available:

	- Darwin/PowerPC


3.) Usage

PARSEC ships with several tools which are installed in the 'bin/' directory. You
can use them to customize and manage your installation of the benchmark suite.
'parsecmgmt' is the main tool. Its purpose is to build and run packages as well
as perform other management operations. 'bldconfadd' and 'bldconfdel' can be
used to create and delete your own build configurations.

When you build and run PARSEC with 'parsecmgmt', it will create a log file which
will contain all output in the 'log/' directory. You can get a help summary for
each tool by invoking it with option '-h'.

A full set of man pages documenting PARSEC, its tools and the most important
parts of the source code is given in the `man/' directory. If you add the
`bin/' directory to the PATH environment variable and the `man/' directory
to the MANPATH variable then all tools and man pages are accessible at the
command line without having to specify the full path every time. A bash shell
script `env.sh' is provided in the root directory of the PARSEC distribution
which modifies the environment in that way. If you work with the bash shell you
can make use of it before you start working with PARSEC by executing
`source env.sh'. You can then start browsing the documentation by running
`man parsec'. Support for other shells is currently not available.

The following examples assume that the 'bin/' directory of PARSEC is in your
path.

3.1.) How to Build PARSEC

Before compiling the PARSEC benchmarks, please change some variables in the
file "config/gcc.bldconf" such that the PARSEC command  can locate the 
compiler path correctly.
 
To compile all programs of the benchmark suite with the default configuration,
simply run:

	parsecmgmt -a build

Building the whole benchmark suite takes a lot of time, usually 30-60 min
depending on your system It is possible to selectively build packages
(option '-p') and to choose different build configurations (option '-c'), read
Section 4 for a more comprehensive explanation of build configurations and
Section 6 for more complex usage examples.

3.2.) How to Run PARSEC

After you have built the suite, you can use the following command to run all
benchmarks with the minimal test input:

	parsecmgmt -a run

The test should finish within 5 seconds. Its purpose is to quickly verify that
all benchmarks have been built successfully and are executable. Do not use it
for performance measurements. You can also choose a different input (option
'-i') and alter the number of threads (option '-n'). Section 5 explains the
different inputs and Section 6 gives more detailed examples.

3.3.) How to Get More Information

To query PARSEC about all available packages and features, you can use the
following command:

	parsecmgmt -a info

If you would like to see information about dynamically created files such as
the available builds, you can run:

	parsecmgmt -a status


4.) Build Configurations

Besides the regular configuration which can be altered to customize the
benchmark suite, PARSEC also uses build configurations. A build configuration
is a specific way to compile a program. It determines, for example, what
compiler to use and which features of the package to enable. Build
configurations should be your first approach to alter how a benchmark program
is created. The only limitation is that the interface and output of a benchmark
must remain the same, otherwise the program will become incompatible with the
PARSEC framework. However, you can adapt the run configuration and reference
outputs to eliminate this problem.

PARSEC ships with the following preinstalled build configurations:

	- 'gcc'		  Build parallel version of suite with gcc
	- 'gcc-serial'	  Build serial version of suite with gcc
	- 'gcc-hooks'	  Build parallel version of suite with PARSEC hooks
			  enabled with gcc
	- 'icc'		  Build parallel version of suite with Intel compiler
	- 'gcc-pthreads'  Build with pthreads parallelization (if supported)
	- 'gcc-openmp'    Build with OpenMP parallelization (if supported)
	- 'gcc-tbb'       Build with TBB parallelization (if supported)

For example, to build PARSEC with enabled hooks, you can use:

	parsecmgmt -a build -c gcc-hooks

The three build configurations 'gcc-pthreads', 'gcc-openmp' and 'gcc-tbb' can
be used to compile a workload with one of these three parallelization models if
it is supported by the program. The build configuration 'gcc' implicitly
defines a standard parallelization for each workload by falling back to one of
these three configurations for each benchmark program.

We also defined an alias for each parallelization model that will be resolved
resolved to a complete list of all workloads that support this parallelization
model. They have the same name as the parallelization. For example, to build
all workloads which support OpenMP with exactly that parallelization, you
can use:

	parsecmgmt -a build -p openmp -c gcc-openmp

Additional build configurations can be created with the tool 'bldconfadd'. For
example, to add a new configuration 'myconfig' that is based on gcc-serial,
you can use the following command:

	bldconfadd -n myconfig -s gcc-serial

To remove this configuration, you can use 'bldconfdel' as follows:

	bldconfdel -n myconfig


5.) Performance Measurement & Research

For each benchmark, we define a Region-Of-Interest (ROI) which includes the
computationally intensive, parallelized phase of the benchmark, but not the
initialization or shutdown phase. Instead of measuring the total program
runtime, you can use and report the execution time of the ROI for any kind of
analysis and comparisons. We provide six inputs for each benchmark program:

	- 'test'	Minimal input to verify that programs are executable.
	- 'simdev'	Very small input which causes code execution comparable
			to a typical input for this program. Intended for
			microarchitectural simulator development.
	- 'simsmall'	Small input for performance measurements with
			microarchitectural simulators
	- 'simmedium'	Medium-sized input for performance measurements with
			microarchitectural simulators
	- 'simlarge'	Large-sized input for performance measurements with
			microarchitectural simulators
	- 'native'	Very large input intended for large-scale experiments
			on real machines

All inputs except 'test' and 'simdev' can be used for performance analysis. As
a rough guideline, on a Pentium 4 processor with 3.0 GHz you can expect
approximately the following execution times:

	- 'test'	almost instantaneous
	- 'simdev'	almost instantaneous
	- 'simsmall'	~1s
	- 'simsmall'	~3-5s
	- 'simlarge'	~12-20s
	- 'native'	~10-30min

The exact runtime depends on the program and its inputs and deviates in some
cases from the described rule of thumb. Different build configurations and
system parameters may also result in a deviation.

5.1.) PARSEC Hooks

All benchmark programs of the benchmark suite support PARSEC hooks, a library
which allows rapid instrumentation of all benchmark. If the benchmarks are
compiled with hooks enabled, at various locations in the code the programs will
call the corresponding hook function. Among other things, the ROI has been
instrumented in such a way, and by default the PARSEC hooks library measures the
time spent in the ROI.

Documentation and code of the PARSEC hooks is available in the 'hooks' package
in group 'libs'. PARSEC hooks are enabled in the build configuration 'gcc-hooks'
and disabled in all other ones.


6.) More Examples

6.1.) Compatible to PARSEC 2.1

Build 'x264' and 'blackscholes' benchmarks with Intel compilers:

	parsecmgmt -a build -p x264 blackscholes -c icc

Build serial version of all kernels:

	parsecmgmt -a build -p kernels -c gcc-serial

Run a self-test with all applications:

	parsecmgmt -a run -p apps -i test

Use version of 'vips' and all kernels with enabled hooks to measure the
performance on a real machine with 32 threads:

	parsecmgmt -a run -p vips kernels -c gcc-hooks -n 32

Use SimpleScalar simulator to run a simulation with the 'gcc-hooks'
build configuration of the 'freqmine' application with small simulator inputs
and 4 threads:

	parsecmgmt -a run -c gcc-hooks -s sim-outorder -p freqmine \
		-i simsmall -n 4

Use an executable 'qsub' to submit runs with all kernels with 16 threads and
large simulator inputs to a batch system (this requires that a program 'qsub'
is installed and in the path which handles the job submission):

	parsecmgmt -a run -s qsub -p kernels -i simlarge -n 16

Clean up after a build or benchmarking run:

	parsecmgmt -a fullclean -p all

Uninstall 'gcc-serial' build of package 'gsl' and build a new version:

	parsecmgmt -a uninstall -p gsl -c gcc-serial
	parsecmgmt -a build -p gsl -c gcc-serial


6.2.) Network benchmarks

Check the status of all components involved in network benchmarks:

        parsecmgmt -a status -p netapps

Build network benchmark 'netstreamcluster':

        parsecmgmt -a build -p raytrace

Build all network benchmark:

        parsecmgmt -a build -p netapps

 Run network benchmark 'netdedup' w/ input 'native' and 2 server threads:

        parsecmgmt -a run -p netdedup -i native -n 2

Run network benchmark 'netferret' w/ input 'simlarge', 4 server threads 
and 2 client connections:

        parsecmgmt -a run -p netferret -i simlarge -n 4 -t 2

For simulation, run 'netdedup' server on a simulator w/ 4 threads and 
run client on a real machine:

        parsecmgmt -a run -p netdedup -i simlarge -n 4 -m server
        parsecmgmt -a run -p netdedup -i simlarge -m client

Do a full cleanup for network benchmarks:

        parsecmgmt -a fullclean -p netapps


6.3.) SPLASH-2 Suite and SPLASH-2x Suite

Check the status of SPLASH-2 suite and SPLASH-2x suite:

        parsecmgmt -a status -p splash2
        parsecmgmt -a status -p splash2x

Build benchmark 'raytrace' from SPLASH-2x suite other than PARSEC:
        parsecmgmt -a build -p splash2x.raytrace
        parsecmgmt -a build -p raytrace   ## defaultly from PARSEC (for comparison)

Build benchmark 'fft' from SPLASH-2 suite with 'gcc-serial' build configuration:

        parsecmgmt -a build -c gcc-serial -p splash2.fft

Build all benchmarks from SPLASH-2 suite and SPLASH-2x suite:

        parsecmgmt -a build -p splash2
        parsecmgmt -a build -p splash2x

Run benchmark 'fft' from SPLASH-2x w/ input 'simsmall' and 4 threads:

        parsecmgmt -a run -p splash2x.fft -i simsmall -n 4

Do a full cleanup for SPLASH-2 suite

        parsecmgmt -a fullclean -p splash2


7.) Structure

The PARSEC suite is composed of the software packages, which are the benchmark
programs and their required libraries and tools, and the framework, which is
everything else. Software packages are located in the 'pkgs/' directory. Each
software package is part of exactly one package group, which has its own
subdirectory. For example, a package named 'foo' which belongs to group 'bar' 
would be located in the directory 'pkgs/bar/foo/'.

PARSEC has the following structure:

	bin/		directory with PARSEC tools
	config/		global configuration
	log/		log files of builds and runs (dynamically created)
	man/		man pages of the PARSEC distribution
	pkgs/		package groups which contain the packages
	version		file with version number of PARSEC distribution

A package has the following directory structure:

	inputs/		directory with input archives (optional)
	inst/		installation directory (dynamically created)
	obj/		build directory (dynamically created)
	outputs/	directory with reference outputs (optional)
	parsec/		PARSEC configuration files
	run/		directory for program execution (dynamically created)
	src/		source code of the package
	version		file with package version

Some of these directories will be auto-generated by 'parsecmgmt' on the fly.

Each package can have multiple builds and installations, and 'parsecmgmt' will
use separate subdirectories for them. PARSEC uses the name of the build
configuration and the platform for which the program is compiled to form a key
that is used to distinguish different builds and installations of a package.

7.1.) Configuration Files

PARSEC distinguishes between global and local configuration. Global
configuration files are located in the 'config/' directory in the root of the
benchmark suite. Local configuration files are only valid for a single software
package and are stored in the 'parsec/' directory of the package they belong to.

The following types of configuration files exist: 'parsec.conf' is a global
file which defines the general structure of the benchmark suite, such as
aliases and which software packages exist. Files named '*.sysconf' define
the basic programs which 'parsecmgmt' is to use on each operating system
platform. To port 'parsecmgmt' to a new operating system, a corresponding file
has to be created. '*.runconf' are configuration files which determine how
benchmark programs are to be executed. Finally, files named '*.bldconf'
describe the build configuration of a package. Run and build configurations
are composed of both global and local configuration files.


8.) Manual Usage

It is possible to build and run PARSEC benchmark programs manually. To do so,
the correct build configuration respectively run configuration should be used
as defined in the PARSEC configuration files.

The following steps are executed by 'parsecmgmt' to build a package:

	- Set variable PARSECDIR to the installation root of PARSEC
	- Set variable PARSECPLAT to the build key used to identify the platform
	- Source system configuration
	- Source global build configuration
	- Source local build configuration
	- Create build directory and change to it
	- If it exists, modify build environment and call configure script
	- Modify build environment and execute 'make'
	- Modify build environment and execute 'make install'

The following steps are executed by 'parsecmgmt' to run a benchmark:

        - Set variable PARSECDIR to the installation root of PARSEC
        - Set variable PARSECPLAT to the build key used to identify the platform
	- Source system configuration
	- Source global run configuration
        - Source local run configuration
	- Create run directory and cd to it
	- Unpack desired input
	- Execute benchmark with parameters from run configuration


9.) Contact

You can reach the PARSEC team as follows:

	http://parsec.cs.princeton.edu/
	parsec@lists.cs.princeton.edu

