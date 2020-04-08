set(INCLUDES
includes/Client.hpp
includes/Host.hpp
includes/IHost.hpp
)

set(SOURCES
source/Client.cpp
source/Host.cpp
)

add_library(ClientLib
${INCLUDES}
${SOURCES}
)

target_include_directories(ClientLib
PUBLIC includes
)