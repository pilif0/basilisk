# Loosely based on: http://neyasystems.com/an-engineers-guide-to-unit-testing-cmake-and-boost-unit-tests/

# Grab all cpp files as tests
file(GLOB TEST_FILES "${TEST_DIR}/*.cpp")

foreach(test_file ${TEST_FILES})
    # Extract the filename
    get_filename_component(test_name ${test_file} NAME_WE)

    # Include Boost, Basilisk and LLVm headers and link LLVM
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
    include_directories(${INCL_DIR})
    link_libraries( ${llvm_libs})

    # Add test target
    add_executable(${test_name} ${test_file})

    # Link to Boost and Basilisk
    target_link_libraries(${test_name} ${Boost_LIBRARIES} basilisk)

    # Move the compiled tests into the test directory
    set_target_properties(${test_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/test)

    # Add the test to the test list
    list(APPEND TESTS ${test_name})
endforeach(test_file)

# Copy test lists to parent for adding
set(basilisk_TESTS ${TESTS} PARENT_SCOPE)
