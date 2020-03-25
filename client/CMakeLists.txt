cmake_minimum_required(VERSION 3.10)

project(Messenger LANGUAGES CXX)


set(SOURCES 
src/Main.cpp
src/Client.cpp
src/Host.cpp
)

set(INCLUDES
includes/Client.hpp
includes/Host.hpp
)

add_executable(Client ${SOURCES} ${INCLUDES})

set_target_properties(Client PROPERTIES CMAKE_CXX_STANDARD 14)

find_package(Threads)
target_link_libraries(Client ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(Client PUBLIC includes)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)