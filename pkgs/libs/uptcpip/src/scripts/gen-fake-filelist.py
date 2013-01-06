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
		print " Usage: python copy-wrap-func.py all-extern-files extern-files-for-copy\n\n"
		sys.exit(-1)

	all_files = sys.argv[1]
	copy_files = sys.argv[2]


    # collect all wrap-funclist
	fp = open(copy_files, "r")
	copy_file_list = []
	new_file = 0

	line = fp.readline()
	while line != "":
		line = line.strip("\n")
		line = line.strip("\t")
		if line != "":
			if line[-2:] == ".c":
				new_file = 1
				copy_file_list.append(line)
		else:
			new_file = 0

		line = fp.readline()

	fp.close()


	# collect all fixed wrap funclist
	fp = open(all_files, "r")
	new_file = 0
	is_fake = 0

	line = fp.readline()
	while line != "":
		line = line.strip("\n")
		line = line.strip("\t")
		if line != "":
			if line[-2:] == ".c":
				new_file = 1
				if not line in copy_file_list:
					is_fake = 1
					print ""
					print line
			else:
				if is_fake:
					print line
		else:
			new_file = 0
			is_fake = 0

		line = fp.readline()

	fp.close()



