cmake_minimum_required(VERSION 3.10)

project(Client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

set(SOURCES 
	src/Main.cpp
	src/Client.cpp
)

add_executable(Client ${SOURCES})

find_package(Threads)
target_link_libraries(Client ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(Client PUBLIC include)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)