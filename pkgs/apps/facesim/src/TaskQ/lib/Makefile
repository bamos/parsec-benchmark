all: taskQ.o

taskQ.o: 
	$(CXX) $(CXXFLAGS) -DTASKQ_DIST_FIXED -c taskQDistCommon.c
	mv taskQDistCommon.o taskQ.o
