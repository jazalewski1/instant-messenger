cmake_minimum_required(VERSION 3.10)

add_executable(ClientTest src/ClientTest.cpp)

target_link_libraries(ClientTest
PRIVATE ClientLib
PRIVATE gtest
PRIVATE gtest_main
PRIVATE gmock
PRIVATE gmock_main
)

target_include_directories(ClientTest
PRIVATE mocks
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)