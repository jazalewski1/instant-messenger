cmake_minimum_required(VERSION 3.10)

project(Messenger LANGUAGES CXX)

set(SOURCES
src/Main.cpp
src/Listener.cpp
src/Server.cpp
)

set(INCLUDES
includes/Listener.hpp
includes/Server.hpp
)

add_executable(Server ${SOURCES} ${INCLUDES})

set_target_properties(Server PROPERTIES CMAKE_CXX_STANDARD 14)

find_package(Threads)
target_link_libraries(Server ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(Server PUBLIC includes)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)