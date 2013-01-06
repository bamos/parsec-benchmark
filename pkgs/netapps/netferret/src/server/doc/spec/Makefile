# Makefile to compile the LaTeX source

TEXSRC = cass.tex
DVI = ${TEXSRC:.tex=.dvi}
PDF = ${TEXSRC:.tex=.pdf} 
PS  = ${TEXSRC:.tex=.ps}  

DVIPSFLAGS = -q -t letter
#LTXFLAGS = -interaction=batchmode
TEXPATH = ".:"

.PHONY: all depend clean nuke
.SUFFIXES: .tex .bib .dvi .pdf .ps

#	.tex.dvi:
#		export TEXINPUTS=${TEXPATH};\
#		latex ${LTXFLAGS} $<;\
#		latex ${LTXFLAGS} $<;\
#		if [ -f $*.bib ] ; then\
#			bibtex $*; \
#			latex $<; \
#		fi;\
#		latex ${LTXFLAGS} $<\
#		latex ${LTXFLAGS} $<\
#		latex ${LTXFLAGS} $<

.tex.pdf:
	export TEXINPUTS=${TEXPATH};\
	pdflatex ${LTXFLAGS} $<;\
	if [ -f $*.bib ] ; then\
		bibtex $*; \
		pdflatex $<; \
	fi;\
	pdflatex ${LTXFLAGS} $<\
	pdflatex ${LTXFLAGS} $<\
	pdflatex ${LTXFLAGS} $<

#	.dvi.ps:
#		dvips ${DVIPSFLAGS} -o $@ $<

.pdf.ps:
	pdf2ps $<

all: ${PDF}

dvi: clean ${DVI}

ps: clean ${PS}

pdf: clean ${PDF}

install: 
	acroread -toPostScript ${PDF}
	cp -f ${PDF} /n/fs/memex/memex_public_web/cass
	cp -f ${PS} /n/fs/memex/memex_public_web/cass

depend:
	./texdep > depend.mk

clean:
	rm -f *~ *.log *.aux
	rm -f *.bbl *.blg *.mpx

nuke: clean
	rm -f ${DVI} ${PDF} ${PS}

# Dependencies
${TEXSRC}: ${BIBSRC} ${MACROS}

include depend.mk
