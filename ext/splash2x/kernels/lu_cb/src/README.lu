GENERAL INFORMATION:

The LU program factors a dense matrix into the product of a lower 
triangular and an upper triangular matrix.  The factorization uses
blocking to exploit temporal locality on individual submatrix elements.
The algorithm used in this implementation is described in 

Woo, S. C., Singh, J. P., and Hennessy, J. L.  The Performance Advantages 
     of Integrating Block Data Transfer in Cache-Coherent Multiprocessors.
     Proceedings of the 6th International Conference on Architectural
     Support for Programming Languages and Operating Systems (ASPLOS-VI),
     October 1994.

Two implementations are provided in the SPLASH-2 distribution:

  (1) Non-contiguous block allocation

      This implementation (contained in the non_contiguous_blocks 
      subdirectory) implements the matrix to be factored with a 
      two-dimensional array.  This data structure prevents blocks from 
      being allocated contiguously, but leads to a conceptually simple
      programming implementation.

  (2) Contiguous block allocation

      This implementation (contained in the contiguous_blocks 
      subdirectory) implements the matrix to be factored as an array
      of blocks.  This data structure allows blocks to be allocated 
      contiguously and entirely in the local memory of processors that
      "own" them, thus enhancing data locality properties.

These programs work under both the Unix FORK and SPROC models.

RUNNING THE PROGRAM:

To see how to run the program, please see the comment at the top of the
file lu.C, or run the application with the "-h" command line option.
Three parameters may be specified on the command line, of which the 
ones that are normally changed are the matrix size and the number of 
processors.  It is suggested that the block size be kept at the value
B=16, since this value works well in practice.  If this parameter is 
changed, the new value should be reported in any results that are 
presented.

BASE PROBLEM SIZE:

The base problem size for an upto-64 processor machine is a 512x512 matrix
with a block size of B=16.

DATA DISTRIBUTION:

Our "POSSIBLE ENHANCEMENT" comments in the source code tell where one
might want to distribute data and how.  Data distribution has a small 
impact on performance on the Stanford DASH multiprocessor.

