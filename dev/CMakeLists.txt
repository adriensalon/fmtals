project(dev)
add_executable(dev "main.cpp")
set_target_properties(dev PROPERTIES CXX_STANDARD 17)
target_link_libraries(dev PRIVATE libfmtals)