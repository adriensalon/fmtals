project(libfmtals_test)
add_executable(libfmtals_test "main.cpp")
set_target_properties(libfmtals_test PROPERTIES CXX_STANDARD 17)
target_link_libraries(libfmtals_test PRIVATE libfmtals)