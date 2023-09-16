flags = -Wall -O3
lflags = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -L/usr/lib/GL 
winlflags = -L include/lib-mingw-w64/glfw3.dll

cc = g++
wincc = x86_64-w64-mingw32-g++

debug:
	mkdir -p build
	${cc}  -Wall main.cpp ${lflags} -o build/main

all:
	mkdir -p build
	${cc} main.cpp ${lflags} ${flags} -o build/main

windows:
	mkdir -p build
	${wincc} main.cpp ${winlflags} ${flags} -o build/main

test:
	./build/main

clean:
	rm build/main 
