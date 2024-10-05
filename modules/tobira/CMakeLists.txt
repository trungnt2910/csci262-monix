project(tobira)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(tobira SHARED
    tobira.cpp
)

set_property(TARGET tobira PROPERTY CXX_STANDARD 23)

install(TARGETS tobira DESTINATION bin)
