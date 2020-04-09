add_executable(ClientTest source/ClientTest.cpp)

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


add_executable(ServerTest source/ServerTest.cpp)

target_link_libraries(ServerTest
PRIVATE ServerLib
PRIVATE UtilitiesLib
PRIVATE gtest
PRIVATE gtest_main
PRIVATE gmock
PRIVATE gmock_main
)

target_include_directories(ServerTest
PRIVATE mocks
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)