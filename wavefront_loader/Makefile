CC = clang++
CCFLAGS = -c -O2
DEBUGFLAGS = -c -g

all: string_utils triangulate loader
	ar rcs libloader.a loader.o string_utils.o triangulate.o
	rm *.o

string_utils:
	${CC} string_utils.cpp ${CCFLAGS}

loader:
	${CC} loader.cpp ${CCFLAGS}

triangulate:
	${CC} triangulate.cpp ${CCFLAGS}

clean:
	rm -f *.o *.a

debug:
	${CC} string_utils.cpp ${DEBUGFLAGS}
	${CC} triangulate.cpp ${DEBUGFLAGS}
	${CC} loader.cpp ${DEBUGFLAGS}
	ar rcs libloaderdebug.a loader.o string_utils.o
	rm *.o

#include libloader.a, like this:
#g++ test.cpp libloader.a -o test -O3
