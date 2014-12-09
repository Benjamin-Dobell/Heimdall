macro(test_large_files VARIABLE USE_64_SUFFIX)
    if(NOT DEFINED ${VARIABLE})
        message(STATUS "Checking if large (64-bit) file support is available...")

        if(USE_64_SUFFIX)
            set(SUFFIX_64 "64")
        else(USE_64_SUFFIX)
            set(SUFFIX_64 "")
        endif(USE_64_SUFFIX)

        # First try without any macros defined
        try_compile(LARGE_FILES_SUPPORTED "${CMAKE_BINARY_DIR}"
                "${CMAKE_MODULE_PATH}/LargeFiles${SUFFIX_64}.c"
                OUTPUT_VARIABLE TRY_LARGE_FILES_OUTPUT)

        if(VERBOSE_LARGE_FILES)
            message(STATUS "Large file output (no special flags):\n${TRY_LARGE_FILES_OUTPUT}")
        endif(VERBOSE_LARGE_FILES)

        if(NOT LARGE_FILES_SUPPORTED)
            # Try with C macro _FILE_OFFSET_BITS=64
            try_compile(LARGE_FILES_SUPPORTED "${CMAKE_BINARY_DIR}"
                    "${CMAKE_MODULE_PATH}/LargeFiles${SUFFIX_64}.c"
                    COMPILE_DEFINITIONS "-D_FILE_OFFSET_BITS=64"
                    OUTPUT_VARIABLE TRY_LARGE_FILES_OUTPUT)

            if(VERBOSE_LARGE_FILES)
                message(STATUS "Large file output (_FILE_OFFSET_BITS=64):\n${TRY_LARGE_FILES_OUTPUT}")
            endif(VERBOSE_LARGE_FILES)

            if(LARGE_FILES_SUPPORTED)
                set(_FILE_OFFSET_BITS=64 CACHE INTERNAL "C macro _FILE_OFFSET_BITS=64 is required for 64-bit file support")
            endif(LARGE_FILES_SUPPORTED)
        endif(NOT LARGE_FILES_SUPPORTED)

        if(NOT LARGE_FILES_SUPPORTED)
            # Try with C macro _LARGEFILE_SOURCE

            try_compile(LARGE_FILES_SUPPORTED "${CMAKE_BINARY_DIR}"
                    "${CMAKE_MODULE_PATH}/LargeFiles${SUFFIX_64}.c"
                    COMPILE_DEFINITIONS "-D_LARGEFILE${SUFFIX_64}_SOURCE"
                    OUTPUT_VARIABLE TRY_LARGE_FILES_OUTPUT)

            if(VERBOSE_LARGE_FILES)
                message(STATUS "Large file output (_LARGEFILE${SUFFIX_64}_SOURCE):\n${TRY_LARGE_FILES_OUTPUT}")
            endif(VERBOSE_LARGE_FILES)

            if(LARGE_FILES_SUPPORTED)
                set(_LARGEFILE${SUFFIX_64}_SOURCE=1 CACHE INTERNAL "C macro _LARGEFILE${SUFFIX_64}_SOURCE is required for 64-bit file support")
            endif(LARGE_FILES_SUPPORTED)
        endif(NOT LARGE_FILES_SUPPORTED)

        if(NOT LARGE_FILES_SUPPORTED)
            # Try with both C macro _FILE_OFFSET_BITS=64 and _LARGEFILE_SOURCE
            try_compile(LARGE_FILES_SUPPORTED "${CMAKE_BINARY_DIR}"
                    "${CMAKE_MODULE_PATH}/LargeFiles${SUFFIX_64}.c"
                    COMPILE_DEFINITIONS "-D_FILE_OFFSET_BITS=64" "-D_LARGEFILE${SUFFIX_64}_SOURCE"
                    OUTPUT_VARIABLE TRY_LARGE_FILES_OUTPUT)

            if(VERBOSE_LARGE_FILES)
                message(STATUS "Large file output (_FILE_OFFSET_BITS=64 and _LARGEFILE${SUFFIX_64}_SOURCE):\n${TRY_LARGE_FILES_OUTPUT}")
            endif(VERBOSE_LARGE_FILES)

            if(LARGE_FILES_SUPPORTED)
                set(_FILE_OFFSET_BITS=64 CACHE INTERNAL "C macro _FILE_OFFSET_BITS=64 is required for 64-bit file support")
                set(_LARGEFILE${SUFFIX_64}_SOURCE=1 CACHE INTERNAL "C macro _LARGEFILE${SUFFIX_64}_SOURCE is required for 64-bit file support")
            endif(LARGE_FILES_SUPPORTED)
        endif(NOT LARGE_FILES_SUPPORTED)

        if(NOT LARGE_FILES_SUPPORTED)
            # Maybe we are using the Windows C standard library
            try_compile(LARGE_FILES_SUPPORTED "${CMAKE_BINARY_DIR}"
                    "${CMAKE_MODULE_PATH}/LargeFilesWindows.c")
        endif(NOT LARGE_FILES_SUPPORTED)

        if(LARGE_FILES_SUPPORTED)
            message(STATUS "Checking if large (64-bit) file support is available - yes")
            set(${VARIABLE} 1 CACHE INTERNAL "Is large file support available?")
        else(LARGE_FILES_SUPPORTED)
            message(STATUS "Checking if large (64-bit) file support is available - no")
            set(${VARIABLE} 0 CACHE INTERNAL "Is large file support available?")
        endif(LARGE_FILES_SUPPORTED)
    endif(NOT DEFINED ${VARIABLE})
endmacro(test_large_files VARIABLE USE_64_SUFFIX)

macro(use_large_files TARGET USE_64_SUFFIX)
    test_large_files(USING_LARGE_FILES ${USE_64_SUFFIX})

    if(USING_LARGE_FILES)
        if(DEFINED _FILE_OFFSET_BITS)
            set_property(TARGET ${TARGET}
                APPEND PROPERTY COMPILE_DEFINITIONS "-D_FILE_OFFSET_BITS=${_FILE_OFFSET_BITS}")
        endif(DEFINED _FILE_OFFSET_BITS)

        if(DEFINED _LARGEFILE_SOURCE)
            set_property(TARGET ${TARGET}
                APPEND PROPERTY COMPILE_DEFINITIONS "-D_LARGEFILE_SOURCE")
        endif(DEFINED _LARGEFILE_SOURCE)

        if(DEFINED _LARGEFILE64_SOURCE)
            set_property(TARGET ${TARGET}
                APPEND PROPERTY COMPILE_DEFINITIONS "-D_LARGEFILE64_SOURCE")
        endif(DEFINED _LARGEFILE64_SOURCE)
    else(USING_LARGE_FILES)
        message(FATAL_ERROR "Large file support not available")
    endif(USING_LARGE_FILES)
endmacro(use_large_files TARGET USE_64_SUFFIX)
