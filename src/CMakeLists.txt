# Link and include libraries
link_libraries()
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories(${INCL_DIR})

# Set up source and header lists
set(basilisk_HEADERS
        "${INCL_DIR}/basilisk/config.h"
        "${INCL_DIR}/basilisk/Lexer.h"
        "${INCL_DIR}/basilisk/Tokens.h"
        "${INCL_DIR}/basilisk/AST.h"
        "${INCL_DIR}/basilisk/AST_util.h"
        "${INCL_DIR}/basilisk/Parser.h")
set(basilisk_SOURCES
        "${SRC_DIR}/Lexer.cpp"
        "${SRC_DIR}/Parser.cpp"
        "${SRC_DIR}/AST.cpp"
        "${SRC_DIR}/AST_util.cpp")

# Copy source and header lists to parent for use in documentation
set(basilisk_HEADERS ${basilisk_HEADERS} PARENT_SCOPE)
set(basilisk_SOURCES ${basilisk_SOURCES} PARENT_SCOPE)

# Add the library
add_library(basilisk ${basilisk_SOURCES} ${basilisk_HEADERS})

# Link required Boost libraries
target_link_libraries(basilisk ${Boost_LIBRARIES})
