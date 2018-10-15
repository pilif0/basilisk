# Set location of files to use
list(APPEND DOC_SRC ${basilisk_HEADERS})
list(APPEND DOC_SRC ${basilisk_SOURCES})

# Set Doxygen options
set(DOXYGEN_PROJECT_NAME "Basilisk")
set(DOXYGEN_PROJECT_NUMBER ${basilisk_VERSION_FULL})

doxygen_add_docs(
        doc
        ${DOC_SRC}
        COMMENT "Generate doc"
)
