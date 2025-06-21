flags = -Wall -O3 -g
debugflags = -Wall -O3 -g -fno-omit-frame-pointer
lflags = -lglfw -lGL -lGLEW -lX11 -lpthread -lXrandr -lXi -ldl 
winlflags = -L include/lib-mingw-w64/glfw3.dll
staticlinked = wavefront_loader/libloader.a space_partition/frustum_culling.a

cc = clang++
wincc = x86_64-w64-mingw32-g++

currdir:
	${cc} ${lflags} ${debugflags} main.cpp ${staticlinked} -o build/main
currdir_opt:
	${cc} ${lflags} ${flags} main.cpp ${staticlinked} -o build/main
debug:
	(cd ./wavefront_loader && make debug)
	mkdir -p build
	rm -rf build/*
	${cc} ${lflags} ${debugflags} main.cpp ${staticlinked} -o build/main

all:
	(cd ./wavefront_loader && make)
	mkdir -p build
	rm -rf build/*
	${cc} ${lflags} ${flags} main.cpp ${staticlinked} -o build/main

test:
	./build/main

clean:
	rm build/main 
