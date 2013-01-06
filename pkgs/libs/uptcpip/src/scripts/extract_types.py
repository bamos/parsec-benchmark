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

#######################################################
# Execute makefile and extract the include files
#
def execute_makefile():
	res = os.system("make >& tmpfile.1")
	fp = open("tmpfile.1", "r")
	line = fp.readline().strip()
	while line != '':
        # Example: "if_ether.c:52:24: error: sys/socket.h: No such file or directory"
		str_list = line.split(':')
		if str_list[-1] == " No such file or directory":

			src_file = str_list[0].strip()
			inc_file = str_list[4].strip()
			if inc_file in inc_file_list:
				rev_inc_files[inc_file].append(src_file)
			else:
				inc_file_list.append(inc_file)
				src_file_list = []
				src_file_list.append(src_file)
				revert_inc_file_list[inc_file] = src_file_list

		line = fp.readline().strip()
	


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

			lindex = line.find("__attribute__")
			if lindex != -1:
				rindex = line.rfind(')')
				line = line[0:lindex] + line[rindex+1:]

			if line[-1] != ')':#function pointer define
				items = line.split()
				items[-1] = items[-1].strip('*')
				#print "%s %s" % (file, line)
				print "%s %s %s" % (file, items[0], items[-1]) 			
			
			line = fp2.readline()

		fp2.close()		
		file = fp.readline()

	fp.close()



