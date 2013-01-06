PROGRAM = test

SUBDIRS = test 

.PHONY: subtargets  $(SUBDIRS) install clean 

$(PROGRAM): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

#install: 
#	$(MAKE) -C host.support install;
#	$(MAKE) -C lib install;

clean: 
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $$subdir clean; \
	done
	
