flags = -Wall -O3
debugflags = -Wall -O3 -g -DDEBUG -fsanitize=address,undefined -fno-omit-frame-pointer
lflags = -lglfw -lGL -lGLEW -lX11 -lpthread -lXrandr -lXi -ldl 
winlflags = -L include/lib-mingw-w64/glfw3.dll

cc = clang++
wincc = x86_64-w64-mingw32-g++

currdir:
	${cc} ${lflags} ${debugflags} main.cpp wavefront_loader/libloaderdebug.a -o build/main
currdir_opt:
	${cc} ${lflags} ${flags} main.cpp wavefront_loader/libloader.a space_partition/frustum_culling.a -o build/main
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
