PREFIX		:= ${PARSECDIR}/pkgs/apps/facesim/inst/${PARSECPLAT}
BINARY		:= facesim

ifdef version
	ifeq "$(version)" "pthreads"
		CXXFLAGS+=-DENABLE_PTHREADS	
	endif

	ifeq "$(version)" "serial"
		CXXFLAGS+=-DNEW_SERIAL_IMPLEMENTATIOM	
	endif
endif

all:
	$(MAKE) -C TaskQ/lib all
	$(MAKE) -C Benchmarks/facesim

install:
	mkdir -p $(PREFIX)/bin
	cp -f Benchmarks/facesim/$(BINARY) $(PREFIX)/bin/$(BINARY)

