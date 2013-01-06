PROGRAM = uptcp

ARCH = $(shell uname -m)

SUBDIRS = freebsd.kern freebsd.netinet host.support lib 

.PHONY: subtargets  $(SUBDIRS) install clean real simulation

$(PROGRAM):
	@rm -rf include/machine;
	@cp -r include/$(ARCH) include/machine; 
	@rm -f freebsd.netinet/in_cksum.c; 
	@cp -r freebsd.netinet/in_cksum_$(ARCH).c freebsd.netinet/in_cksum.c; 
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $$subdir; \
	done

install: 
	$(MAKE) -C lib install;

clean:
	rm -rf include/machine; 
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $$subdir clean; \
	done
	
