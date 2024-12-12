flags = -Wall -O3
debugflags = -Wall -Og -g
lflags = -lglfw -lGL -lGLEW -lX11 -lpthread -lXrandr -lXi -ldl 
winlflags = -L include/lib-mingw-w64/glfw3.dll

cc = clang++
wincc = x86_64-w64-mingw32-g++

currdir_opt:
	${cc} ${lflags} ${debugflags} main.cpp wavefront_loader/libloader.a engine/engine.a -o build/verlet
currdir:
	${cc} ${lflags} ${debugflags} main.cpp wavefront_loader/libloaderdebug.a engine/engine.a -o build/main
debug:
	(cd ./wavefront_loader && make debug)
	(cd ./engine && make debug)
	mkdir -p build
	rm -rf build/*
	${cc} ${lflags} ${debugflags} main.cpp wavefront_loader/libloaderdebug.a engine/engine.a -o build/main

all:
	(cd ./wavefront_loader && make)
	(cd ./engine && make)
	mkdir -p build
	rm -rf build/*
	${cc} ${lflags} ${flags} main.cpp wavefront_loader/libloader.a engine/engine.a -o build/main

test:
	./build/main

clean:
	rm build/main 
