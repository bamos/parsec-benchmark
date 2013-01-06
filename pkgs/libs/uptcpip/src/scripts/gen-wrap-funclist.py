import time
import os, fnmatch
import sys


#########################################################                
#
#  1. execute ld to generate log
#  2. collect all undefined reference symbols
#  3. find out the files defining the symbols
#  4. sort all the files
#  5. let user decide whether copy the file or just make fake file
#

if __name__ == '__main__':

	count = 0
	
	argc = len(sys.argv)
	if argc != 2:
		print " Usage: python gen-wrap-funclist.py output\n\n"
		sys.exit(-1)

	output_file = sys.argv[1]
	print "Output result file is [%s]\n" % (output_file)

    # collect all the extern-dep functions
	func_def_list = []

	os.system("ld *.o >& tmpfile.1")
	fp = open("tmpfile.1", "r")

	line = fp.readline().strip()
	while line != '':
		iterms = line.split()
		if len(iterms) != 0:
			if "undefined" in iterms and  "reference" in iterms:
				if not (iterms[-1] in func_def_list):
					func_def_list.append(iterms[-1])
					count = count + 1
            
		line = fp.readline().strip()
        
	fp.close()

    #output the extern function list
	fpw = open(output_file, "w")
	for i in range(count):
		fpw.write("NULL \t")
		fpw.write(func_def_list[i][1:-1])
		fpw.write(" \t 0\t NULL\n")
	fpw.close()

	print("Find %d Wrap-Function Definitaions\n",  count)
	os.remove("tmpfile.1")
