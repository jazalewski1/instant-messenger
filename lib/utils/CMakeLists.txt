set(INCLUDES
includes/Utils.hpp
includes/Exceptions.hpp
includes/InputObservable.hpp
includes/InputObserver.hpp
)

set(SOURCES
source/InputObservable.cpp
source/InputObserver.cpp
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