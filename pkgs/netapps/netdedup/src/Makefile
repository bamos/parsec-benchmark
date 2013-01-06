PROGRAM = netdedup

INSTDIR = ${PARSECDIR}/pkgs/netapps/netdedup/inst/${PARSECPLAT}/bin

SUBDIRS = server client

.PHONY: subtargets  $(SUBDIRS) install clean 

$(PROGRAM): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

install: 
	mkdir -p ${INSTDIR}
	cp ./server/netdedup ${INSTDIR} 
	cp ./server/netdedup_sim ${INSTDIR} 
	cp ./client/netdedup_client ${INSTDIR} 
	cp ./client/netdedup_client_sim ${INSTDIR} 
	cp ./run.sh ${INSTDIR} 

clean: 
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $$subdir clean; \
	done
	
