cmake_minimum_required(VERSION 3.10)

find_package(Threads)

add_executable(Client src/Main.cpp)

add_library(ClientLib
includes/Client.hpp
includes/Host.hpp
includes/IHost.hpp
src/Client.cpp
src/Host.cpp
)

target_link_libraries(ClientLib
${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(Client
ClientLib
)

target_include_directories(Client PRIVATE includes)
target_include_directories(ClientLib PUBLIC includes)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)