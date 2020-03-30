cmake_minimum_required(VERSION 3.10)

find_package(Threads)

add_executable(Server src/Main.cpp)

add_library(ServerLib
includes/Data.hpp
includes/Server.hpp
includes/Listener.hpp
includes/IListener.hpp
src/Server.cpp
src/Listener.cpp
)

target_link_libraries(ServerLib
${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(Server
ServerLib
)

target_include_directories(Server PRIVATE includes)
target_include_directories(ServerLib PUBLIC includes)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)