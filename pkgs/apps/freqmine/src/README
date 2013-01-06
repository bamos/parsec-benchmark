Name: freqmine

Description

Frequent Itemsets Mining (FIM) is the basis of Association Rule 
Mining (ARM). Association Rule Mining is the process of analyzing
a set of transactions to extract association rules. ARM is a very 
common used and well-studied data mining problem. The mining is 
applicable to any sequential and time series data via discretization.
Example domains are protein sequences, market data, web logs, text, 
music, stock market, etc. 

To mine ARMs is converted to mine the frequent itemsets Lk, which 
contains the frequent itemsets of length k. Many FIMI (FIM 
Implementation) algorithms have been proposed in the literature, 
including FP-growth and Apriori based approaches. Researches showed
that the FP-growth can get much faster than some old algorithms like 
the Apriori based approaches except in some cases the FP-tree can be
too large to be stored in memory when the database size is so large 
or the database is too sparse.


=======================================
Programming Languages & Libraries 

C++ and OpenMP is used to implement this benchmark.

=======================================
System Requirements:

 1) Intel(R) C++ Compiler: version 9.0 or higher
 2) GNU gcc/g++: version 3.3 or higher
 3) sed: version 4.0.9 or higher recommended.
The benchmark needs at least 530 MBytes memory.

=======================================
Input/Output:

For the input, a date-set file containing the test transactions is provided.
There is another parameter that indicates "minimum-support". When it is a
integer, it means the minimum counts; when it is a floating point number
between 0 and 1, it means the percentage to the total transaction number.

The program output all (different length) frequent itemsets with fixed minimum
support.

=======================================
Characteristics:

(1) Hotspot
The hotspots of the benchmark are three functions in the "fp_tree.cpp" file: FPArray_scan2_DB,
FPArray_conditional_pattern_base and transform_FPTree_into_FPArray.

=======================================
Benchmark Author

Chunrong Lai (chunrong.lai@intel.com)
Contactor
Yurong Chen (yurong.chen@intel.com)

=======================================
References

[1] Xu J., Li M., Kim D. and Xu Y., RAPTOR: Optimal Protein Threading by Linear Programming, Journal of Bioinformatics and Computational Biology, Vol. 1(1):95-117, 2003.

[2] Xu Y., Xu, D., Protein threading using PROSPECT: Design and evaluation. Proteins: Structure, Function, and Genetics, Vol.40(3):343-354..
