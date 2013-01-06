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
	if argc != 3:
		print " Usage: python gen-wrap-funclist.py input output\n\n"
		sys.exit(-1)

	input_file = sys.argv[1]
	output_file = sys.argv[2]
	print "Input  file is [%s]\n" % (input_file)
	print "Output result file is [%s]\n" % (output_file)

    # collect all the extern-dep functions
	wrap_filelist = []
	revert_wrap_filelist = {}
	sym = ""

	fp = open(input_file, "r")


	line = fp.readline()
	while line != '':
		if  line == '\n':
			line = fp.readline()
			sym = line.split()[1]
			line = fp.readline()
			if line == '\n':
				continue

		iterms = line.split()
		if len(iterms) != 0:
			file = iterms[0]
			if file[-1] != 'h':
				if not file in wrap_filelist:
					print file		
					wrap_filelist.append(file)
					revert_wrap_filelist[file] = []
					revert_wrap_filelist[file].append(sym)
					count = count + 1
				else:
					if not sym in revert_wrap_filelist[file]:
						revert_wrap_filelist[file].append(sym)

            
		line = fp.readline()
        
	fp.close()

	fpw = open(output_file, "w")
	for x in wrap_filelist:
		funclist = revert_wrap_filelist[x]
		fpw.write(x)
		fpw.write("\n")
		for y in funclist:
			fpw.write("\t\t")
			fpw.write(y)
			fpw.write("\n")
		fpw.write("\n")

	fpw.close()

	print("Find %d Wrap-Function Definitaions\n",  count)
