PREFIX=${PARSECDIR}/pkgs/apps/blackscholes/inst/${PARSECPLAT}

CSRC    = $(SRC)
TARGET  = blackscholes
M4_BASE = .
MACROS  = c.m4.pthreads

ifdef source
	ifeq "$(source)" "simd" 
		SRC = blackscholes.simd.c
		CXXFLAGS += -msse3
	endif
else
	SRC	= blackscholes.c
endif

ifdef version
  ifeq "$(version)" "pthreads"
    M4_SRC    = blackscholes.m4.cpp
    CSRC      = $(M4_SRC)
    MT        = -DENABLE_THREADS
    CXXFLAGS += -pthread
  endif
  ifeq "$(version)" "openmp"
    MT        = -DENABLE_OPENMP
  endif
  ifeq "$(version)" "tbb"
    MT        = -DENABLE_TBB
  endif
else
  MT        = 
endif



# Default build single precision version
NCO     = -DNCO=4

ifdef chk_err
ERR     = -DERR_CHK
endif

ifdef single
NCO = -DNCO=4
endif

ifdef size
SZ = -DN=$(size)
else
SZ = -DN=960
endif

ifdef double
NCO = -DNCO=2
endif

CXXFLAGS += $(MT) $(NCO) $(FUNC) $(ERR) $(CSRC)

all: $(TARGET)

$(TARGET): clean $(CSRC)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -o $(TARGET)

$(M4_SRC): $(SRC)
	$(M4) $(M4_BASE)/$(MACROS) $< > $@

clean:
	rm -f $(TARGET) $(M4_SRC)

install:
	mkdir -p $(PREFIX)/bin
	cp -f $(TARGET) $(PREFIX)/bin/$(TARGET)

