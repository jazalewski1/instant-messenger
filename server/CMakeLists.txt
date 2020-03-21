cmake_minimum_required(VERSION 3.10)

project(messenger)

add_executable(Server src/Main.cpp)

target_include_directories(Server PRIVATE include)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)