CMAKE_MINIMUM_REQUIRED(VERSION 3.15.0)
add_compile_options("/std:c++latest")

project(SM64)

SET(target inputcapture)

SET(src ${CMAKE_CURRENT_SOURCE_DIR}/src)

SET(sources ${src}/main.cpp)

ADD_EXECUTABLE(${target} ${sources})

ADD_SUBDIRECTORY(deps/winctl)
ADD_SUBDIRECTORY(deps/json)

TARGET_LINK_LIBRARIES(${target} PUBLIC winctl nlohmann_json::nlohmann_json)

#[[
SET_TARGET_PROPERTIES(${target}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
)
TARGET_COMPILE_FEATURES(
    ${target}
    PUBLIC
        cxx_std_17
)
]]
