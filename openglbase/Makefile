flags = -Wall -O3
debugflags = -Wall -O0 -g
lflags = -lglfw -lGL -lGLEW -lX11 -lpthread -lXrandr -lXi -ldl 

windirflags = -L"../includes/win" -L"./wavefront_loader" -I../includes
winlinkedfiles = -l:libloaderwin.a -lglew32 -lglfw3 -lopengl32 -lgdi32 -luser32 -lkernel32
winflags = -static

cc = clang++
wincc = x86_64-w64-mingw32-g++

currdir:
	${cc} ${lflags} ${debugflags} main.cpp wavefront_loader/libloaderdebug.a -o build/main
currdir_opt:
	${cc} ${lflags} ${debugflags} main.cpp wavefront_loader/libloader.a -o build/main
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
all_win:
	(cd ./wavefront_loader && make all_win)
	mkdir -p build
	${wincc} ${windirflags} ${flags} main.cpp ${winflags} ${winlinkedfiles} -o build/main.exe

test:
	./build/main

clean:
	rm build/main 
