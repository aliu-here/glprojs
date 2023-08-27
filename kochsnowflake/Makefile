flags = -Wall -Werror -O0
lflags = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl 
cc = g++

debug:
	${cc} ${lflags} -Wall main.cpp include/glad.c -o build/main

all:
	${cc} ${lflags} ${flags} main.cpp include/glad.c -o build/main

test:
	./build/main

clean:
	rm -r build
	mkdir build
