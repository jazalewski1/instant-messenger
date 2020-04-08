set(INCLUDES
includes/Utils.hpp
)

set(SOURCES
)

add_library(UtilitiesLib SHARED
${INCLUDES}
${SOURCES}
)

target_include_directories(UtilitiesLib
PUBLIC includes
)