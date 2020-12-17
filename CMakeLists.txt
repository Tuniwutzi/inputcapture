CMAKE_MINIMUM_REQUIRED(VERSION 3.15.0)

project(SM64)

SET(target inputcapture)

SET(src ${CMAKE_CURRENT_SOURCE_DIR}/src)

SET(sources ${src}/main.cpp)

#ADD_LIBRARY(${target} STATIC ${sources})
ADD_EXECUTABLE(${target} ${sources})

ADD_SUBDIRECTORY(deps/winctl)

TARGET_LINK_LIBRARIES(${target} PUBLIC winctl)
