cmake_minimum_required(VERSION 3.12)
project(fmtals)

option(FMTALS_BUILD_TOOL "" ON)
option(FMTALS_BUILD_DEMO "" ON)

# fmtals library
set(JUST_INSTALL_CEREAL ON)
set(SKIP_PERFORMANCE_COMPARISON ON)
add_subdirectory(external/cereal)
add_subdirectory(external/zlib)
file(GLOB_RECURSE fmtals_source "source/*.cpp")
add_library(fmtals STATIC ${fmtals_source})
set_target_properties(fmtals PROPERTIES CXX_STANDARD 17)
target_include_directories(fmtals PUBLIC include)
target_include_directories(fmtals PRIVATE external/zlib)
target_link_libraries(fmtals PRIVATE cereal zlib)

# tools
if(FMTALS_BUILD_TOOL)
    add_executable(fmtals_als2xml "tool/als2xml.cpp")
    set_target_properties(fmtals_als2xml PROPERTIES CXX_STANDARD 17)
    target_link_libraries(fmtals_als2xml PRIVATE fmtals cereal)
    add_executable(fmtals_xml2als "tool/xml2als.cpp")
    set_target_properties(fmtals_xml2als PROPERTIES CXX_STANDARD 17)
    target_link_libraries(fmtals_xml2als PRIVATE fmtals cereal)
endif()

# demo
if(FMTALS_BUILD_DEMO)
    add_executable(fmtals_demo "demo/main.cpp")
    set_target_properties(fmtals_demo PROPERTIES CXX_STANDARD 17)
    target_link_libraries(fmtals_demo PRIVATE fmtals)
endif()