GCC        = g++ -g -O0 -Wall -Wextra -std=gnu++0x
MKDEPS     = g++ -MM -std=gnu++0x
VALGRIND   = valgrind --leak-check=full --show-reachable=yes

MKFILE     = Makefile
# DEPSFILE   = Makefile.deps
SOURCES    = auxlib.cc oc.cc stringset.cc
HEADERS    = auxlib.h stringset.h
OBJECTS    = ${SOURCES:.cc=.o}
EXECBIN    = oc
SRCFILES   = ${HEADERS} ${SOURCES} ${MKFILE}

all : ${EXECBIN}

${EXECBIN} : ${OBJECTS}
	${GCC} -o${EXECBIN} ${OBJECTS}

%.o : %.cc
	${GCC} -c $<

clean :
	- rm ${OBJECTS} *.str