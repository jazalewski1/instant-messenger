cmake_minimum_required(VERSION 3.10)

project(messenger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

set(SOURCES
	src/Main.cpp
	src/Listener.cpp
)

add_executable(Server ${SOURCES})

target_include_directories(Server PUBLIC include)

# find_package(Threads)
# target_link_libraries(Server ${CMAKE_THREAD_LIBS_INIT})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)