#Nicholas Pappas
#nhpappas@ucsc.edu
#CMPS 104a w/ Mackey
#Assignment: Project 1

GPP      = g++ -std=gnu++14 -g -O0 -Wall -Wextra
MKDEP    = g++ -std=gnu++14 -MM
VALGRIND = valgrind --leak-check=full --show-reachable=yes

EXECBIN  = oc
MKFILE   = Makefile
DEPFILE  = Makefile.dep
SOURCES  = auxlib.cpp string_set.cpp main.cpp
HEADERS  = auxlib.h string_set.h
OBJECTS  = ${SOURCES:.cpp=.o}
SRCFILES = ${HEADERS} ${SOURCES} ${MKFILE}
SMLFILES = ${DEPFILE}
CHECKINS = ${SRCFILES} ${SMLFILES}
LISTING  = Listing.ps

all : ${EXECBIN}

${EXECBIN} : ${OBJECTS}
	${GPP} ${OBJECTS} -o ${EXECBIN}

%.o : %.cpp
	${GPP} -c $<

ci :
	${SRCFILES}
	checksource ${CHECKINS}

clean :
	-rm ${OBJECTS}

spotless : clean
	- rm ${EXECBIN} ${DEPFILE}
	- rm *.str *.out *.err

${DEPFILE} :
	${MKDEP} ${SOURCES} >${DEPFILE}

dep :
	- rm ${DEPFILE}
	${MAKE} --no-print-directory ${DEPFILE}

include ${DEPFILE}

test : ${EXECBIN}
	${VALGRIND} ${EXECBIN} * * * >test.out 2>&1

lis : test
	mkpspdf ${LISTING} ${SRCFILES} ${DEPFILE} test.out

again : ${SRCFILES}
	make --no-print-directory spotless dep ci test lis

submit :
	submit cmps104a-wm.s17 asg1 ${SRCFILES} README

