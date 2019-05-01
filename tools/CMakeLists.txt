# Include Boost and Basilisk headers
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
include_directories(${INCL_DIR})

# Add basilisk executable
add_executable(basilisk_c basilisk_c.cpp)
target_link_libraries(basilisk_c ${Boost_LIBRARIES} basilisk ${llvm_libs})
set_target_properties(basilisk_c PROPERTIES OUTPUT_NAME "basilisk")
