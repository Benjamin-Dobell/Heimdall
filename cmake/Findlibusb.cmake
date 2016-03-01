find_path(LIBUSB_INCLUDE_DIR
    NAMES
        libusb.h
    PATHS
        /usr/local/include
        /opt/local/include
        /usr/include
    PATH_SUFFIXES
        libusb-1.0
)

if (libusb_USE_STATIC_LIBS AND NOT MSVC)
    set (LIBUSB_LIB_PREFIX "lib" CACHE INTERNAL "libusb library name prefix passed to find_library")
    set (LIBUSB_LIB_SUFFIX ".a" CACHE INTERNAL "libusb library name suffix passed to find_library")
else (libusb_USE_STATIC_LIBS AND NOT MSVC)
    set (LIBUSB_LIB_PREFIX "" CACHE INTERNAL "libusb library name prefix passed to find_library")
    set (LIBUSB_LIB_SUFFIX "" CACHE INTERNAL "libusb library name suffix passed to find_library")
endif (libusb_USE_STATIC_LIBS AND NOT MSVC)

find_library(LIBUSB_LIBRARY
    NAMES
        ${LIBUSB_LIB_PREFIX}usb-1.0${LIBUSB_LIB_SUFFIX} ${LIBUSB_LIB_PREFIX}usb${LIBUSB_LIB_SUFFIX}
    PATHS
        /usr/local/lib
        /opt/local/lib
        /usr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libusb REQUIRED_VARS LIBUSB_LIBRARY LIBUSB_INCLUDE_DIR)

if (LIBUSB_FOUND)
    set(LIBUSB_INCLUDE_DIRS ${LIBUSB_INCLUDE_DIR})
    set(LIBUSB_LIBRARIES ${LIBUSB_LIBRARY})
    mark_as_advanced(LIBUSB_INCLUDE_DIR LIBUSB_LIBRARY)
endif (LIBUSB_FOUND)
