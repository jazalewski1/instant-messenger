add_executable(Server app/Main.cpp)

target_include_directories(Server
PRIVATE app
)

add_subdirectory(lib)

find_package(Threads)

target_link_libraries(Server
${CMAKE_THREAD_LIBS_INIT}
ServerLib
UtilitiesLib
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)