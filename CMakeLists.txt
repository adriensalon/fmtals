cmake_minimum_required(VERSION 3.12)
project(libfmtals)

set(JUST_INSTALL_CEREAL ON)
set(SKIP_PERFORMANCE_COMPARISON ON)
add_subdirectory(external/cereal)
add_subdirectory(external/zlib)

file(GLOB_RECURSE libfmtals_source "source/*.cpp")
add_library(libfmtals STATIC ${libfmtals_source})
set_target_properties(libfmtals PROPERTIES CXX_STANDARD 17)
target_include_directories(libfmtals PUBLIC include)
target_include_directories(libfmtals PRIVATE external/zlib)
target_link_libraries(libfmtals PRIVATE cereal zlib)

add_subdirectory(tool/als2xml)
add_subdirectory(tool/xml2als)
add_subdirectory(test)