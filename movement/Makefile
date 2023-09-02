flags = -Wall -O3
lflags = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl 
cc = g++

all:
	${cc} ${lflags} ${flags} main.cpp include/glad.c -o build/main

test:
	./build/main

clean:
	rm -r build
	mkdir build
