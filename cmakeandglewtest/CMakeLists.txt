#cmake stuff, i'm just gonna use c++17
cmake_minimum_required(VERSION 3.27)
project(opengltesting)
set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES src/main.cpp)

find_library(GLFW glfw)
find_library(OpenGL GL)
find_library(GLEW GLEW)

if(NOT GLFW)
	message(FATAL_ERROR "GLFW not found")
endif()
if(NOT OpenGL)
	message(FATAL_ERROR "OpenGL not found")
endif()
if(NOT GLEW)
	message(FATAL_ERROR "GLEW not found")
endif()

#copy the shaders and resource files over to the build directory
file(COPY shaders/. DESTINATION shaders)
file(COPY resources/. DESTINATION resources)
ADD_EXECUTABLE(opengltesting ${SOURCE_FILES})

#link the libraries
target_link_libraries(opengltesting glfw)
target_link_libraries(opengltesting GL)
target_link_libraries(opengltesting GLEW)
