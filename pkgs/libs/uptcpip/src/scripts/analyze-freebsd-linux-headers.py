import time
import os, fnmatch
import sys

#########################################################                
#  1. read all wrap-func 
#  2. read all fixed-wrap-func
#  3. fake-wrap-func = wrap-func - fixed-wrap-func
#
if __name__ == '__main__':
	argc = len(sys.argv)
	if argc != 3:
		print " Usage: python copy-wrap-func.py input1 input2\n\n"
		sys.exit(-1)

	input1 = sys.argv[1]
	input2 = sys.argv[2]


    # collect all wrap-funclist
	fp = open(input2, "r")
	linux = []
	linux_files = {}

	line = fp.readline()
	while line != "":
		line = line.strip('\n')
		header = line.rsplit('/', 1)[1]
		if not header in linux:
			linux.append(header)
			linux_files[header] = []
			linux_files[header].append(line)
		else:
			linux.append(header)
			linux_files[header].append(line)
		line = fp.readline()

	fp.close()


	# collect all fixed wrap funclist
	fp = open(input1, "r")

	line = fp.readline()
	while line != "":
		line = line.strip('\n')
		header = line.rsplit('/', 1)[1]
		if header in linux:
			print line 
			for x in linux_files[header]:
				print "\t\t --> \t %s" % (x)

		line = fp.readline()

	fp.close()



