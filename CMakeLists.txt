cmake_minimum_required(VERSION 3.9)
project(basilisk)

set(CMAKE_CXX_STANDARD 17)

# Set compiler flags
set(CLANG_COMPILER_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_COMPILER_FLAGS}")

# Export compile commands for use with LLVM tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set project version
set(basilisk_VERSION_MAJOR "0")
set(basilisk_VERSION_MINOR "1")
set(basilisk_VERSION_PATCH "0")
set(basilisk_VERSION_EXTRA "")
set(basilisk_VERSION "${basilisk_VERSION_MAJOR}.${basilisk_VERSION_MINOR}")
set(basilisk_VERSION_FULL "${basilisk_VERSION_MAJOR}.${basilisk_VERSION_MINOR}.${basilisk_VERSION_PATCH}${basilisk_VERSION_EXTRA}")

# Declare options
option(basilisk_BUILD_DOC "Build documentation" ON)
option(basilisk_BUILD_TEST "Build tests" ON)
set(basilisk_BOOST "/opt/boost" CACHE PATH "Boost directory")
set(basilisk_LLVM "/opt/llvm" CACHE PATH "LLVM directory")

# Set some global locations
set(SRC_DIR "${basilisk_SOURCE_DIR}/src")
set(INCL_DIR "${basilisk_SOURCE_DIR}/include")
set(DEP_DIR "${basilisk_SOURCE_DIR}/deps")

# Boost
set(BOOST_ROOT ${basilisk_BOOST})
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package (Boost 1.69.0 REQUIRED COMPONENTS unit_test_framework)
if(NOT (Boost_FOUND))
    message(ERROR "Boost not found")
endif()

# LLVM
set(LLVM_DIR "${basilisk_LLVM}/build/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

# Doxygen
if (basilisk_BUILD_DOC)
    find_package(Doxygen)
endif()

# Configure header
configure_file(
        "${INCL_DIR}/basilisk/config.h.in"
        "${INCL_DIR}/basilisk/config.h"
)

# Add subdirectories
add_subdirectory(src)

if (basilisk_BUILD_DOC)
    add_subdirectory(doc)
endif()

if (basilisk_BUILD_TEST)
    add_subdirectory(test)
endif()
