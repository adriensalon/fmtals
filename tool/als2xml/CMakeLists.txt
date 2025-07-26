project(libfmtals_als2xml)
add_executable(libfmtals_als2xml "main.cpp")
set_target_properties(libfmtals_als2xml PROPERTIES CXX_STANDARD 17)
target_link_libraries(libfmtals_als2xml PRIVATE libfmtals cereal)