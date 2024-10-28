flags = -Wall -O3
debugflags = -Wall -O0 -g
lflags = -lglfw -lGL -lGLEW -lX11 -lpthread -lXrandr -lXi -ldl 
winlflags = -L include/lib-mingw-w64/glfw3.dll

cc = clang++
wincc = x86_64-w64-mingw32-g++

debug:
	(cd ./wavefront_loader && make debug)
	mkdir -p build
	rm -rf build/*
	${cc} ${lflags} ${debugflags} main.cpp wavefront_loader/libloaderdebug.a -o build/main

all:
	(cd ./wavefront_loader && make)
	mkdir -p build
	rm -rf build/*
	${cc} ${lflags} ${flags} main.cpp wavefront_loader/libloader.a -o build/main

test:
	./build/main

clean:
	rm build/main 
