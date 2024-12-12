cc = clang++
ccflags = -c -O3
debugflags = -c -g -O0

all:
	${cc} update_physobjs.cpp ${ccflags}
	ar rcs engine.a update_physobjs.o
	rm *.o
debug:
	${cc} update_physobjs.cpp ${debugflags}
	ar rcs engine.a update_physobjs.o
	rm *.o
clean:
	rm *.o *.a
