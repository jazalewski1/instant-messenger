add_executable(Client app/Main.cpp)

target_include_directories(Client
PRIVATE app
)

add_subdirectory(lib)

find_package(Threads)

target_link_libraries(Client
${CMAKE_THREAD_LIBS_INIT}
ClientLib
UtilitiesLib
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)