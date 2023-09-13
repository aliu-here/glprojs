flags = -Wall -O0
lflags = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl 

cc = g++

debug:
	mkdir -p build
	${cc} ${lflags} -Wall main.cpp include/glad.c -o build/main

all:
	mkdir -p build
	${cc} ${lflags} ${flags} main.cpp include/glad.c -o build/main

test:
	./build/main

clean:
	rm build/main 
