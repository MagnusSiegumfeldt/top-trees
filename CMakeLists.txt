cmake_minimum_required(VERSION 3.15)
project(top-trees)

set(CMAKE_CXX_STANDARD_REQUIRED 20)

set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -DNDEBUG")

include_directories(include)

set(TEST_FILES ${TEST_FILES}
test/tree_tests/tree_test.cpp
test/toptree_tests/max_weight_in_component_test.cpp
test/toptree_tests/max_weight_on_path_test.cpp
#test/toptree_tests/add_common_weight_test.cpp
test/toptree_tests/diameter_test.cpp
)

#Main target
add_executable(main src/main.cpp)

add_subdirectory(src/lib/Catch2)
#Removes extra CTest targets
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
#Add Ctest and Catch to CMake
include(CTest)
include(Catch)
enable_testing()

#Create tests target and add it to test list
add_executable(tests ${TEST_FILES})
target_link_libraries(tests PRIVATE Catch2 Catch2WithMain)
catch_discover_tests(tests)
