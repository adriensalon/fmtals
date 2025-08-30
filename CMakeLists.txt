cmake_minimum_required(VERSION 3.12)
project(fmtals)

option(FMTALS_BUILD_TOOL "Build tool executables" ON)

# fmtals library
add_subdirectory(external/zlib)
file(GLOB_RECURSE fmtals_source "source/*.cpp")
add_library(fmtals STATIC ${fmtals_source})
set_target_properties(fmtals PROPERTIES CXX_STANDARD 17)
target_include_directories(fmtals PUBLIC include)
target_include_directories(fmtals PRIVATE ${CEREAL_INCLUDE_DIR})
target_include_directories(fmtals PRIVATE external/zlib)
target_link_libraries(fmtals PRIVATE zlib)

# tools
if(FMTALS_BUILD_TOOL)
    add_executable(als2xml "tool/als2xml.cpp")
    set_target_properties(als2xml PROPERTIES CXX_STANDARD 17)
    target_link_libraries(als2xml PRIVATE fmtals)
    add_executable(xml2als "tool/xml2als.cpp")
    set_target_properties(xml2als PROPERTIES CXX_STANDARD 17)
    target_link_libraries(xml2als PRIVATE fmtals)
endif()
