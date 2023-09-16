#cmake stuff, i'm just gonna use c++17
cmake_minimum_required(VERSION 3.27)
project(opengltesting)
set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES src/main.cpp)

#copy the shaders and resource files over to the build directory
file(COPY shaders/. DESTINATION shaders)
file(COPY resources/. DESTINATION resources)
ADD_EXECUTABLE(opengltesting ${SOURCE_FILES})

#link the libraries
target_link_libraries(opengltesting -lglfw)
target_link_libraries(opengltesting -lGL)
target_link_libraries(opengltesting -lGLEW)
