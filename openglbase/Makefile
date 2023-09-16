flags = -Wall -O3
lflags = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl 
winlflags = -L include/lib-mingw-w64/glfw3.dll

cc = g++
wincc = x86_64-w64-mingw32-g++

debug:
	mkdir -p build
	${cc} ${lflags} -Wall main.cpp include/glad.c -o build/main

all:
	mkdir -p build
	${cc} ${lflags} ${flags} main.cpp include/glad.c -o build/main

windows:
	mkdir -p build
	${wincc} ${winlflags} ${flags} main.cpp include/glad.c -o build/main

test:
	./build/main

clean:
	rm build/main 
