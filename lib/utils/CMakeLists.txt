set(INCLUDES
includes/Utils.hpp
includes/Exceptions.hpp
)

set(SOURCES
)

add_library(UtilitiesLib SHARED
${INCLUDES}
${SOURCES}
)

set_target_properties(UtilitiesLib
PROPERTIES LINKER_LANGUAGE CXX
)

target_include_directories(UtilitiesLib
PUBLIC includes
)