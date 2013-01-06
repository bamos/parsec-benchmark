# Makefile for streamcluster

PREFIX=${PARSECDIR}/pkgs/netapps/netstreamcluster/inst/${PARSECPLAT}

TARGET=server
TARGET_SIM=server_sim

OBJS=streamcluster.o

ifdef version
  ifeq "$(version)" "pthreads"
    CXXFLAGS :=	$(CXXFLAGS) -DENABLE_THREADS -lpthread
    OBJS += parsec_barrier.o
  endif
  ifeq "$(version)" "tbb"
    CXXFLAGS := $(CXXFLAGS) -DTBB_VERSION
  endif
endif

CXXFLAGS += -DENABLE_PARSEC_UPTCPIP

LIB_UPTCP= -luptcp -lpthread
LIB_UPTCP_SIM= -luptcp.sim -lpthread

all: $(TARGET) $(TARGET_SIM)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) $(LIB_UPTCP) -o $(TARGET)

$(TARGET_SIM): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) $(LIB_UPTCP_SIM) -o $(TARGET_SIM)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o $(TARGET)

