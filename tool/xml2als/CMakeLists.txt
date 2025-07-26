project(libfmtals_xml2als)
add_executable(libfmtals_xml2als "main.cpp")
set_target_properties(libfmtals_xml2als PROPERTIES CXX_STANDARD 17)
target_link_libraries(libfmtals_xml2als PRIVATE libfmtals)