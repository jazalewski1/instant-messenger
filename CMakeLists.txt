cmake_minimum_required(VERSION 3.17)

project(Messenger LANGUAGES CXX)

set(CXX_STANDARD 14)

execute_process(COMMAND bash -c "git clone https://github.com/google/googletest.git ${PROJECT_SOURCE_DIR}/googletest")

add_subdirectory(server)

add_subdirectory(client)

add_subdirectory(lib/utils)

add_subdirectory(tests)