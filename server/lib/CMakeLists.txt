set(INCLUDES
includes/Data.hpp
includes/Server.hpp
includes/Listener.hpp
includes/IListener.hpp
)

set(SOURCES
source/Server.cpp
source/Listener.cpp
)

add_library(ServerLib SHARED
${INCLUDES}
${SOURCES}
)

target_include_directories(ServerLib
PUBLIC includes
)