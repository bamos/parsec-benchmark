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
	if argc != 4:
		print " Usage: python copy-include.py src_inc_dir dst_inc_dir input_file\n\n"
		sys.exit(-1)

	src_inc_dir = sys.argv[1].rstrip("/")
	dst_inc_dir = sys.argv[2].rstrip("/")
	input_file = sys.argv[3]
	print "Top source include directory is [%s]" % (src_inc_dir)
	print "Top target include directory is [%s]\n" % (dst_inc_dir)

	logfp = open("copy-include.log", "a")
	logfp.write("\n=============================================\n")
	today = time.localtime()
	today_str = "%d-%d-%d %d:%d:%d" %(today[0:6])
	logfp.write(today_str)
	logfp.write("\n=============================================\n")

	execute_makefile()
	fd = open(input_file, "r")
	line = fd.readline();
	while line != "":
		line = line.strip(".")
		line = line.strip("\n")
		line = line.strip("/")

		(path, file) = line.rsplit('/', 1)
		print "%s  %s" % (path, file)
		src_path_dir = src_inc_dir + "/" + path
		src_path     = src_inc_dir + "/" + path + "/" + file
		dst_path_dir = dst_inc_dir + "/" +  path
		file = "bsd_" + file
		dst_path = dst_inc_dir + "/" +  path + "/" + file

		# check if dir in include_dir
		logfp.write("cp " + src_path + " \t" + dst_path + "\n")
		if not path in os.listdir(dst_inc_dir):
			os.makedirs(dst_inc_dir + "/" + path)
		shutil.copyfile(src_path, dst_path)
		
		line = fd.readline()

	fd.close()

	logfp.close()
	os.remove("tmpfile.1")
