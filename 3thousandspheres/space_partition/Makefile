CC=clang++
FLAGS=-O3 -c -g -fno-omit-frame-pointer

all:
	${CC} frustum_cull.cpp ${FLAGS}
	ar rcs frustum_culling.a frustum_cull.o
debug:
	${CC} frustum_cull.cpp ${FLAGS} -DDEBUG -g
	ar rcs frustum_culling.a frustum_cull.o
clean:
	rm *.a *.o
