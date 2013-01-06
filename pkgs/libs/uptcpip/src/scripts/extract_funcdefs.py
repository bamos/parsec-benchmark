########################################################
#
#
#######################################################
import time
import os, fnmatch
import sys
import shutil 
import datetime

inc_file_list = []
revert_inc_file_list = {} 

funcfilters = {}

#########################################################                
#
#  1. system("make")
#  1. While find new include files:
#        1.1 mkdir
#        1.2 copy the file to new dir
#        1.3 system("make")
# 
if __name__ == '__main__':
    
	argc = len(sys.argv)
	if argc != 2:
		print " Usage: python extract_type.py filelist\n\n"
		sys.exit(-1)

	filelist = sys.argv[1]

	funcfilters["__attribute_"] = "("
	funcfilters["__MALLOC_"] = " "
	funcfilters["_MPFR_"] = " "

	fp = open(filelist, "r")

	file = fp.readline()
	while file != "":
		file = file.strip('\n')
		cmd = "~/working/UpTCP/uptcp/tools/c-parser/extract-def <" + file + " >tmpfile" 
		os.system(cmd)

		fp2 = open("tmpfile", "r")
		line = fp2.readline()
		while line != "":
			line = line.strip('\n')
			line = line.strip(" {;=#")

			for key in funcfilters.keys():
				lindex = line.find(key)
				while lindex != -1:
					rindex = line.find(funcfilters[key], lindex);
					if rindex != -1:
						line = line[0:lindex] + line[rindex+1:];
					else:
						break
					lindex = line.find(key, lindex+1)


			bracket = 0
			lindex = line.rfind(")")
			if lindex != -1:
				bracket = bracket + 1

			while bracket != 0 and lindex != -1:
				lindex = lindex - 1
				if line[lindex] == ")":
					bracket = bracket + 1
	
				if line[lindex] == "(":
					bracket = bracket - 1
			
			line = line[0:lindex]
			line = line.strip()
	
			func = line.split()[-1].strip('*')
			if not func in funcfilters:
				#print "%s %s" % (file, line)
				print "%s funcdef %s" % (file, func) 			
			
			line = fp2.readline()

		fp2.close()		
		file = fp.readline()

	fp.close()
	os.system("rm -f tmpfile ")



