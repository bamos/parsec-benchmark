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
	linux_types = {}

	line = fp.readline()
	while line != "":
		line = line.strip('\n')
		list = line.split()
		if len(list) == 3:
			(file, type, defs) = line.split()

			if type == "funcdef" or type == "typedef" or type == "struct" or type == "define":
				type_defs = type + "." + defs
				if not type_defs in linux:
					linux.append(type_defs)
					linux_types[type_defs] = []
					linux_types[type_defs].append(file)
				else:
					linux.append(type_defs)
					linux_types[type_defs].append(file)
		line = fp.readline()

	fp.close()


	# collect all fixed wrap funclist
	fp = open(input1, "r")

	file_count = 0
	type_count = 0
	line = fp.readline()
	last_file = ""
	bsd = []
	bsd_defs = []
	bsd_only = []
	while line != "":
		line = line.strip('\n')
		list = line.split()
		if len(list) == 3:
			(file, type, defs) = line.split()
	
			if type == "funcdef" or type == "typedef" or type == "struct" or type == "define":
				type_defs = type + "." + defs

				if file != last_file:
					if last_file != "":
						print "\t------------------"
						for xx in bsd_only:
							print "\t%s" % xx
					bsd_only = []
					last_file = file
					print "\n%s" % file
					file_count = file_count + 1

				if not defs in bsd_defs:
					bsd_defs.append(defs)
					if type_defs in linux:
						type_count = type_count + 1
						print "\t%s" % type_defs 
						for x in linux_types[type_defs]:
							print "\t\t --> %s" % (x)
					else:
						bsd_only.append(type_defs)


		line = fp.readline()

	print "\t------------------"
	for xx in bsd_only:
		print "\t%s" % xx
	
	fp.close()
	print "Conflict Files = %d, Types Count = %d" % (file_count, type_count)



