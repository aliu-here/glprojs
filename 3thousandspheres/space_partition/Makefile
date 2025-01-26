CC=clang++
FLAGS=-O3 -c

all:
	${CC} frustum_cull.cpp ${FLAGS}
	ar rcs frustum_culling.a frustum_cull.o
debug:
	${CC} frustum_cull.cpp ${FLAGS} -DDEBUG
	ar rcs frustum_culling.a frustum_cull.o
clean:
	rm *.a *.o
