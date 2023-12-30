CC = g++
CCFLAGS = -c -O3

all: loader
	ar rcs libloader.a loader.o string_utils.o
	rm *.o

string_utils:
	${CC} string_utils.cpp ${CCFLAGS}

loader: string_utils
	${CC} loader.cpp ${CCFLAGS}

clean:
	rm -f *.o *.a

#include libloader.a, like this:
#g++ test.cpp libloader.a -o test -O3
