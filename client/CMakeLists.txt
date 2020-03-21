cmake_minimum_required(VERSION 3.10)

project(Client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

add_executable(Client src/Main.cpp)

target_include_directories(Client PRIVATE include)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)