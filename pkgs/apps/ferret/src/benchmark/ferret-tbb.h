

#ifndef _ferret_tbb_h_
#define _ferret_tbb_h_

#include <sys/types.h>
#include <dirent.h>
#include <stack>
#include <tbb/pipeline.h>

class filter_load : public tbb::filter {
	char m_path[BUFSIZ];
	const char *m_single_file;
	
	std::stack<DIR *> m_dir_stack;
	std::stack<int>   m_path_stack;
	
	private:
		void push_dir(const char * dir);
	
	public:
		filter_load(const char * dir);
		/*override*/void* operator()( void* item );
};


class filter_seg : public tbb::filter {
	public:
		  filter_seg();
		  /*override*/void* operator()(void* item);
};

class filter_extract : public tbb::filter {
	public:
		filter_extract();
		/*override*/void* operator()(void* item);
};

class filter_vec : public tbb::filter {
	public:
		filter_vec();
		/*override*/void* operator()(void* item);
};

class filter_rank : public tbb::filter {
	public:
		filter_rank();
		/*override*/void* operator()(void* item);
};

class filter_out : public tbb::filter {
	public:
		filter_out();
		/*override*/void* operator()(void* item);
};

#endif
