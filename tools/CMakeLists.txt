# Include Boost and Basilisk headers
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories(${INCL_DIR})

# Link libraries
link_libraries(${Boost_LIBRARIES} basilisk)

# Add lexer executable
add_executable(lexer_executable lexer_executable.cpp)

# Add parser executable
add_executable(parser_executable parser_executable.cpp)

# Add codegen executable
add_executable(codegen_executable codegen_executable.cpp)
