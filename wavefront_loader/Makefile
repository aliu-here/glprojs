CC = clang++
INCLUDES = 
CXX_STD = c++23
CCFLAGS = -c -O3 -Wall -Werror
DEBUGFLAGS = -g -DDEBUG -fno-omit-frame-pointer

wincc = x86_64-w64-mingw32-g++
winincludes = -Llibmingw32 -I../includes
winflags = -static

all: clean string_utils triangulate loader save_load_utils
	ar rcs libloader.a loader.o string_utils.o triangulate.o

coverage: CCFLAGS := ${CCFLAGS} ${DEBUGFLAGS} --coverage
coverage: clean string_utils triangulate loader save_load_utils
	ar rcs libloader.a loader.o string_utils.o triangulate.o

debug: CCFLAGS := ${CCFLAGS} ${DEBUGFLAGS}
debug: clean string_utils triangulate loader save_load_utils
	ar rcs libloader.a loader.o string_utils.o triangulate.o


all_win: CC = ${wincc}
all_win: INCLUDES = ${winincludes}
all_win: CCFLAGS += ${winflags}
all_win: clean string_utils triangulate loader save_load_utils
	x86_64-w64-mingw32-ar rcs libloaderwin.a loader.o string_utils.o triangulate.o

string_utils:
	${CC} ${INCLUDES} string_utils.cpp ${CCFLAGS} -std=${CXX_STD}

loader:
	${CC} ${INCLUDES} loader.cpp ${CCFLAGS} -std=${CXX_STD}

triangulate:
	${CC} ${INCLUDES} triangulate.cpp ${CCFLAGS} -std=${CXX_STD}

save_load_utils:
	${CC} ${INCLUDES} save_load_utils.cpp ${CCFLAGS} -std=${CXX_STD}

clean:
	rm -f *.o *.a
#include libloader.a, like this:
#g++ test.cpp libloader.a -o test -O3
