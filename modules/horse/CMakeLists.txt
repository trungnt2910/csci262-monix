project(horse)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(horse SHARED
    horse.cpp
)

set_property(TARGET horse PROPERTY CXX_STANDARD 23)

install(TARGETS horse DESTINATION bin)
