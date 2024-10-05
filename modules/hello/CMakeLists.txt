project(hello)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(hello SHARED
    hello.cpp
)

set_property(TARGET hello PROPERTY CXX_STANDARD 23)

install(TARGETS hello DESTINATION bin)
